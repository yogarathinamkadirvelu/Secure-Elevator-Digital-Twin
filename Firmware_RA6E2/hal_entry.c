#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "arm_math.h"

#define TIME_BETWEEN_FLOORS  3000
#define TIME_CAR_SETTLING    3000  
#define TIME_DOOR_OPERATION  2000
#define MAX_WEIGHT_LIMIT     800.0f

typedef enum {
    STATE_IDLE, STATE_DOORS_CLOSING, STATE_MOVING, STATE_ARRIVED_WAIT,
    STATE_DOORS_OPENING, STATE_DOORS_OPEN, STATE_OVERLOAD
} elevator_state_t;

void send_json_event(const char* event_type, const char* details);
void arm_keypad_interrupt(void);
char scan_keypad(void);
void process_key(char key);
void process_elevator_logic(void);
void process_background_dsp(void);
uint32_t custom_rand(void);

volatile uint32_t g_ms_ticks = 0;
volatile bool keypad_interrupt = false;
volatile bool uart_tx_complete = false;

elevator_state_t current_state = STATE_IDLE;
int current_floor = 0;
int current_direction = 0;
uint32_t state_timer = 0;
bool floor_queue[10] = {false};

float simulated_base_weight = 0.0f;
float filtered_clean_weight = 0.0f;
bool is_overloaded = false;

#define NUM_TAPS 5
#define BLOCK_SIZE 10
float32_t fir_state[NUM_TAPS + BLOCK_SIZE - 1];
float32_t fir_coeffs[NUM_TAPS] = {0.2f, 0.2f, 0.2f, 0.2f, 0.2f};
arm_fir_instance_f32 fir_inst;

bsp_io_port_pin_t ROW_PINS[4] = {BSP_IO_PORT_00_PIN_08, BSP_IO_PORT_01_PIN_13, BSP_IO_PORT_04_PIN_09, BSP_IO_PORT_05_PIN_00};
bsp_io_port_pin_t COL_PINS[4] = {BSP_IO_PORT_04_PIN_08, BSP_IO_PORT_01_PIN_05, BSP_IO_PORT_01_PIN_00, BSP_IO_PORT_01_PIN_01};
char key_map[4][4] = {
    {'1','2','3','A'}, {'4','5','6','B'},
    {'7','8','9','C'}, {'*','0','#','D'}
};

uint32_t custom_rand(void) {
    static uint32_t seed = 123456789;
    seed = (1103515245 * seed + 12345) % 2147483648;
    return seed;
}

void uart_callback(uart_callback_args_t *p_args) {
    if(p_args && p_args->event == UART_EVENT_TX_COMPLETE) uart_tx_complete = true;
}

void send_json_event(const char* event_type, const char* details) {
    char msg[256];
    sprintf(msg, "{\"event\":\"%s\", %s}\r\n", event_type, details);
    uart_tx_complete = false;
    R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *)msg, (uint32_t)strlen(msg));
    uint32_t timeout = 100000;
    while(!uart_tx_complete && timeout > 0) timeout--;
    R_BSP_SoftwareDelay(15, BSP_DELAY_UNITS_MILLISECONDS);
}

void timer_1ms_callback(timer_callback_args_t *p_args) {
    FSP_PARAMETER_NOT_USED(p_args); g_ms_ticks++;
}

void keypad_irq_callback(external_irq_callback_args_t *p_args) {
    FSP_PARAMETER_NOT_USED(p_args);
    static uint32_t last = 0;
    if((g_ms_ticks - last) > 50) { keypad_interrupt = true; last = g_ms_ticks; }
}

void arm_keypad_interrupt(void) {
    for(int i = 0; i < 4; i++) R_IOPORT_PinWrite(&g_ioport_ctrl, ROW_PINS[i], BSP_IO_LEVEL_LOW);
}

char scan_keypad(void) {
    bsp_io_level_t col_state;
    bool a_pressed = false, nine_pressed = false;
    char single_key = 0; int keys_pressed = 0;

    for(int r = 0; r < 4; r++) {
        R_IOPORT_PinWrite(&g_ioport_ctrl, ROW_PINS[r], BSP_IO_LEVEL_LOW);
        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MILLISECONDS);
        for(int c = 0; c < 4; c++) {
            R_IOPORT_PinRead(&g_ioport_ctrl, COL_PINS[c], &col_state);
            if(col_state == BSP_IO_LEVEL_LOW) {
                if (r == 0 && c == 3) a_pressed = true; // Key A
                if (r == 2 && c == 2) nine_pressed = true; // Key 9
                single_key = key_map[r][c];
                keys_pressed++;
            }
        }
        R_IOPORT_PinWrite(&g_ioport_ctrl, ROW_PINS[r], BSP_IO_LEVEL_HIGH);
    }
    arm_keypad_interrupt();
    if (a_pressed && nine_pressed) return 'S';
    if (keys_pressed >= 1) return single_key;
    return 0;
}

void process_background_dsp(void) {
    static uint32_t last_dsp_run = 0;
    static uint32_t last_telemetry = 0;
    char details[128];

    if ((g_ms_ticks - last_dsp_run) >= 100) {
        last_dsp_run = g_ms_ticks;
        float32_t noisy_buffer[BLOCK_SIZE], clean_buffer[BLOCK_SIZE];

        for(int i = 0; i < BLOCK_SIZE; i++) {
            int val = (int)simulated_base_weight + ((int)(custom_rand() % 10) - 5);
            noisy_buffer[i] = (float)(val < 0 ? 0 : val);
        }

        arm_fir_f32(&fir_inst, noisy_buffer, clean_buffer, BLOCK_SIZE);
        filtered_clean_weight = clean_buffer[BLOCK_SIZE - 1];
        if (filtered_clean_weight > MAX_WEIGHT_LIMIT) {
            if (!is_overloaded) {
                is_overloaded = true;
                sprintf(details, "\"fault_code\":\"ERR_OVERLOAD\", \"measured_kg\":%d", (int)filtered_clean_weight);
                send_json_event("CRITICAL_ALARM", details);
                send_json_event("SYSTEM_STATE", "\"state\":\"OVERLOAD_SAFETY_LOCK\"");
            }
        }
        else if (is_overloaded) {
            is_overloaded = false;
            send_json_event("SYSTEM_STATE", "\"state\":\"IDLE_READY\"");
        }

        if ((g_ms_ticks - last_telemetry) >= 1000) {
            last_telemetry = g_ms_ticks;
            sprintf(details, "\"sensor\":\"PHYSICAL_LOAD_CELL\", \"dsp_smooth_kg\":%d", (int)filtered_clean_weight);
            send_json_event("TELEMETRY_STREAM", details);
        }
    }
}


void process_key(char key) {
    char details[128];

    
    if (key == 'D') {
        is_overloaded = true;
        send_json_event("CRITICAL_ALARM", "\"fault_code\":\"MANUAL_EMERGENCY_STOP\", \"measured_kg\":999");
        send_json_event("SYSTEM_STATE", "\"state\":\"OVERLOAD_SAFETY_LOCK\"");
        return;
    }

    
    if (key == 'B') { 
        if (current_state == STATE_IDLE || current_state == STATE_DOORS_OPEN) {
            current_state = STATE_DOORS_OPENING; state_timer = g_ms_ticks;
            send_json_event("SYSTEM_STATE", "\"state\":\"DOORS_OPENING\", \"floor\":-1");
        }
        return;
    }

    if (key == 'C') { 
        if (current_state == STATE_DOORS_OPEN) {
            current_state = STATE_DOORS_CLOSING; state_timer = g_ms_ticks;
            send_json_event("SYSTEM_STATE", "\"state\":\"DOORS_CLOSING\"");
        }
        return;
    }
    if (key == 'A') {
        send_json_event("PROMPT", "\"msg\":\"Awaiting secondary key...\"");
        return;
    }
    if (key == 'S') { 
        if (is_overloaded) return;
        floor_queue[9] = true;
        send_json_event("AUTHENTICATION", "\"status\":\"Success\", \"authorized_floor\":9");
        return;
    }
    if (key == '*' || key == '#') {
        // ALLOW weight changes IF doors are open OR if we are in an overload!
        if (current_state != STATE_DOORS_OPEN && !is_overloaded) {
            send_json_event("WARNING", "\"msg\":\"Action Blocked: Doors are closed!\"");
            return;
        }

        if (key == '*') {
            simulated_base_weight += 75.0f;
            send_json_event("EVENT", "\"action\":\"Passenger Boarded (+75kg)\"");
        } else {
            simulated_base_weight -= 75.0f;
            if (simulated_base_weight < 0) simulated_base_weight = 0.0f;
            send_json_event("EVENT", "\"action\":\"Passenger Disembarked (-75kg)\"");
        }
        return;
    }
    if (is_overloaded) return;

    if(key >= '0' && key <= '8') {
        int req_floor = key - '0';

        // If we are already on this floor, open the doors instantly!
        if (req_floor == current_floor && current_state == STATE_IDLE) {
            current_state = STATE_DOORS_OPENING; state_timer = g_ms_ticks;
            sprintf(details, "\"state\":\"DOORS_OPENING\", \"floor\":%d", current_floor);
            send_json_event("SYSTEM_STATE", details);
        }
        else if (req_floor != current_floor) {
            floor_queue[req_floor] = true;
            sprintf(details, "\"action\":\"Floor %d Request Received\"", req_floor);
            send_json_event("EVENT", details);
        }
    }
}
void process_elevator_logic(void) {
    char details[64];
    if (is_overloaded) { current_state = STATE_OVERLOAD; return; }
    else if (current_state == STATE_OVERLOAD) { current_state = STATE_IDLE; }

    switch(current_state) {
        case STATE_IDLE:
            if(floor_queue[current_floor]) {
                floor_queue[current_floor] = false;
                current_state = STATE_DOORS_OPENING; state_timer = g_ms_ticks;
                sprintf(details, "\"state\":\"DOORS_OPENING\", \"floor\":%d", current_floor);
                send_json_event("SYSTEM_STATE", details);
                break;
            }
            bool req_up = false, req_down = false;
            for(int i = current_floor + 1; i < 10; i++) if(floor_queue[i]) req_up = true;
            for(int i = current_floor - 1; i >= 0; i--) if(floor_queue[i]) req_down = true;

            if(current_direction == 1 && req_up) current_direction = 1;
            else if(current_direction == -1 && req_down) current_direction = -1;
            else if(req_up) current_direction = 1;
            else if(req_down) current_direction = -1;
            else current_direction = 0;

            if(current_direction != 0) {
                current_state = STATE_DOORS_CLOSING; state_timer = g_ms_ticks;
                send_json_event("SYSTEM_STATE", "\"state\":\"DOORS_CLOSING\"");
            }
            break;

        case STATE_DOORS_CLOSING:
            if((g_ms_ticks - state_timer) > TIME_DOOR_OPERATION) {
                // BUG FIX: Recalculate direction BEFORE moving! Prevents crashing into Floor 10.
                bool check_up = false, check_down = false;
                for(int i = current_floor + 1; i < 10; i++) if(floor_queue[i]) check_up = true;
                for(int i = current_floor - 1; i >= 0; i--) if(floor_queue[i]) check_down = true;

                if(current_direction == 1 && check_up) current_direction = 1;
                else if(current_direction == -1 && check_down) current_direction = -1;
                else if(check_up) current_direction = 1;
                else if(check_down) current_direction = -1;
                else current_direction = 0;

                if (current_direction != 0) {
                    current_state = STATE_MOVING; state_timer = g_ms_ticks;
                    sprintf(details, "\"state\":\"IN_TRANSIT\", \"vector\":\"%s\"", (current_direction == 1) ? "ASCENDING" : "DESCENDING");
                    send_json_event("SYSTEM_STATE", details);
                } else {
                    current_state = STATE_IDLE;
                    send_json_event("SYSTEM_STATE", "\"state\":\"IDLE_READY\"");
                }
            }
            break;

        case STATE_MOVING:
            if((g_ms_ticks - state_timer) > TIME_BETWEEN_FLOORS) {
                current_floor += current_direction; state_timer = g_ms_ticks;
                sprintf(details, "\"current_floor\":%d", current_floor);
                send_json_event("LOCATION_TRACKER", details);

                if(floor_queue[current_floor]) {
                    floor_queue[current_floor] = false;
                    current_state = STATE_ARRIVED_WAIT;
                    sprintf(details, "\"state\":\"ARRIVED\", \"floor\":%d", current_floor);
                    send_json_event("SYSTEM_STATE", details);
                } else {
                    bool keep_going = false, reverse_req = false;
                    if(current_direction == 1) {
                        for(int i = current_floor + 1; i < 10; i++) if(floor_queue[i]) keep_going = true;
                        for(int i = current_floor - 1; i >= 0; i--) if(floor_queue[i]) reverse_req = true;
                    } else {
                        for(int i = current_floor - 1; i >= 0; i--) if(floor_queue[i]) keep_going = true;
                        for(int i = current_floor + 1; i < 10; i++) if(floor_queue[i]) reverse_req = true;
                    }

                    if(!keep_going) {
                        if(reverse_req) {
                            current_direction = -current_direction;
                            sprintf(details, "\"state\":\"IN_TRANSIT\", \"vector\":\"%s\"", (current_direction == 1) ? "ASCENDING" : "DESCENDING");
                            send_json_event("SYSTEM_STATE", details);
                        } else {
                            current_state = STATE_IDLE; current_direction = 0;
                            send_json_event("SYSTEM_STATE", "\"state\":\"IDLE_READY\"");
                        }
                    }
                }
            }
            break;

        case STATE_ARRIVED_WAIT:
            if((g_ms_ticks - state_timer) > TIME_CAR_SETTLING) {
                current_state = STATE_DOORS_OPENING; state_timer = g_ms_ticks;
                sprintf(details, "\"state\":\"DOORS_OPENING\", \"floor\":%d", current_floor);
                send_json_event("SYSTEM_STATE", details);
            }
            break;

        case STATE_DOORS_OPENING:
            if((g_ms_ticks - state_timer) > TIME_DOOR_OPERATION) {
                current_state = STATE_DOORS_OPEN; state_timer = g_ms_ticks;
            }
            break;

        case STATE_DOORS_OPEN:
            break;
    }
}

void hal_entry(void) {
    R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    R_GPT_Open(&g_timer_1ms_ctrl, &g_timer_1ms_cfg);
    R_GPT_Start(&g_timer_1ms_ctrl);

    arm_fir_init_f32(&fir_inst, NUM_TAPS, fir_coeffs, fir_state, BLOCK_SIZE);

    R_ICU_ExternalIrqOpen(&g_external_irq0_ctrl, &g_external_irq0_cfg); R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
    R_ICU_ExternalIrqOpen(&g_external_irq1_ctrl, &g_external_irq1_cfg); R_ICU_ExternalIrqEnable(&g_external_irq1_ctrl);
    R_ICU_ExternalIrqOpen(&g_external_irq2_ctrl, &g_external_irq2_cfg); R_ICU_ExternalIrqEnable(&g_external_irq2_ctrl);
    R_ICU_ExternalIrqOpen(&g_external_irq3_ctrl, &g_external_irq3_cfg); R_ICU_ExternalIrqEnable(&g_external_irq3_ctrl);

    arm_keypad_interrupt();
    send_json_event("BOOT_SEQUENCE", "\"status\":\"SYSTEM_ONLINE\", \"firmware\":\"SECURE_SCADA_FINAL\"");

    while(1) {
        if(keypad_interrupt) {
            keypad_interrupt = false;
            char key = scan_keypad();
            if(key) process_key(key);
        }
        process_background_dsp();
        process_elevator_logic();
    }
}
