# 🏢 Secure-Elevator-Digital-Twin: Hardware-Accelerated SCADA

![Architecture](https://img.shields.io/badge/Architecture-ARM%20Cortex--M33-blue)
![Silicon](https://img.shields.io/badge/Silicon-Renesas%20RA6E2-red)
![Security](https://img.shields.io/badge/Security-ARM%20TrustZone-green)
![Status](https://img.shields.io/badge/Phase-Active%20Development-success)

## 📌 The Cyber-Physical Divide
Software lives in a perfect vacuum. Embedded systems live in the brutal, chaotic physical world. This repository houses the complete architecture for a zero-latency, hardware-in-the-loop Digital Twin designed for a mission-critical industrial elevator system. 

It bridges the gap between raw, noisy analog load cells and a clean, event-driven web UI using strict hardware determinism, DSP math, and TrustZone isolation.

## 📂 Architecture Structure

* **`Firmware_RA6E2/`** *Cortex-M33 C code utilizing the Flexible Software Package (FSP).*
* **`Middleware_Python/`** *Asynchronous `pyserial` translator & WebSocket bridge.*
* **`Dashboard_Web/`** *Zero-latency, event-driven HTML/Vanilla JS UI.*

---

## 🗓️ The Silicon-to-Screen Engineering Log
*This architecture is being developed and documented progressively. Below is the active log of engineering milestones.*

### **Week 1: The Cyber-Physical Architecture**
* **Focus:** System Design & Data Flow
* **Execution:** Defined the blueprint bridging the physical edge (sensors) to the cloud UI. Established the necessity for strict industrial determinism over standard cloud polling, outlining the transition from raw analog voltage to hardware-accelerated DSP, and finally to DOM manipulation via asynchronous Python WebSockets.

### **Week 2: Software-in-the-Loop (SIL) & The Digital Torture Chamber**
* **Focus:** Kinetic Noise Simulation & Safety State Machines
* **Execution:** Before testing critical safety logic on unverified physical hardware, I engineered a deterministic SIL mathematical model in C. This module injects synthetic kinetic noise (+/- 5kg bounce) and simulates severe motor EMI into the base weight values. This created a perfectly hostile environment to guarantee the non-blocking state machine handles `ERR_OVERLOAD` conditions without freezing.

### **Week 3: The Silicon Selection & Hardware Math**
* **Focus:** Clock Routing & FPU Acceleration 
* **Execution:** Configured the Renesas RA6E2 silicon to handle continuous 32-bit decimal (`float32_t`) arrays without choking the CPU. Routed the `HOCO 20MHz` through the PLL to achieve a `200MHz` System Clock. Explicitly enabled the Cortex-M33 IEEE-754 single-precision Hardware Floating-Point Unit (FPU) to execute the FIR DSP filter natively in the silicon, achieving zero-latency data processing.

---
*(Week 4 in development: Non-Blocking Matrix Keypad Math & Bitwise Hardware Polling)*
