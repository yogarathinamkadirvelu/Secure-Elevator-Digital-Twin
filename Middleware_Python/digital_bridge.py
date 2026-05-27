import asyncio
import websockets
import serial
import sys

# --- CONFIGURATION ---
COM_PORT = 'COM8'  # <--- Change to match your board!
BAUD_RATE = 115200

# Connect to the Renesas Board
try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.01)
    print(f"[SUCCESS] Connected to Renesas on {COM_PORT}")
except Exception as e:
    print(f"[ERROR] Could not connect to {COM_PORT}. Is Tera Term open?")
    sys.exit()

# Keep track of connected web browsers
connected_clients = set()

async def client_handler(websocket):
    # When you open the HTML file, this registers the connection
    connected_clients.add(websocket)
    print("\n[🔗 CONNECTED] Web Dashboard Linked Successfully!")
    try:
        await websocket.wait_closed()
    finally:
        connected_clients.remove(websocket)
        print("\n[❌ DISCONNECTED] Web Dashboard Closed.")

async def serial_reader():
    while True:
        try:
            # Changed 'if' to 'while' -> Read EVERYTHING instantly until the buffer is empty
            while ser.in_waiting > 0: 
                raw_data = ser.readline().decode('utf-8').strip()
                if raw_data:
                    # Print to Python terminal so you know it arrived
                    print(f"⚡ FAST-FORWARD: {raw_data}")
                    
                    # Instantly fire it into the HTML WebSocket
                    for client in list(connected_clients):
                        await client.send(raw_data)
        except Exception:
            pass
        
        # Reduced sleep time for maximum polling speed
        await asyncio.sleep(0.001)
async def main():
    # Start the invisible tunnel on port 8765
    async with websockets.serve(client_handler, "localhost", 8765):
        print("==================================================")
        print(" 🌐 WEBSOCKET BRIDGE ONLINE (ws://localhost:8765) ")
        print("==================================================")
        await serial_reader()

if __name__ == "__main__":
    # Windows fix for newer Python versions
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(main())