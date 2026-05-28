# 🏢 Secure-Elevator-Digital-Twin: Hardware-Accelerated SCADA

![Architecture](https://img.shields.io/badge/Architecture-ARM%20Cortex--M33-blue)
![Silicon](https://img.shields.io/badge/Silicon-Renesas%20RA6E2-red)
![Security](https://img.shields.io/badge/Security-Logical%20Enclave-green)
![Status](https://img.shields.io/badge/Phase-Active%20Development-success)

## 📌 The Cyber-Physical Divide
Software lives in a perfect vacuum. Embedded systems live in the brutal, chaotic physical world. This repository houses the complete architecture for a zero-latency, hardware-in-the-loop Digital Twin designed for a mission-critical industrial elevator system. 

It bridges the gap between raw, noisy analog load cells and a clean, event-driven web UI using strict hardware determinism, DSP math, and multi-scan logical security.

## 📂 Architecture Structure

* **`Firmware_RA6E2/`** *Cortex-M33 C code utilizing the Flexible Software Package (FSP).*
  * **Current Focus:** Logical security enclaves via Multi-Scan Bitwise Authentication for VIP access.
  * 200MHz hardware clock routing and IEEE-754 Hardware FPU acceleration.
  * Non-blocking bitwise matrix keypad scanning for zero-latency CPU execution.
  * Software-in-the-Loop (SIL) kinetic noise simulation and non-blocking safety state machines.
  * Hardware-accelerated CMSIS-DSP FIR filtering to natively crush EMI noise.
* **`Middleware_Python/`** *Asynchronous `pyserial` translator.*
  * Full-duplex WebSocket bridge breaking the local USB browser sandbox.
* **`Dashboard_Web/`** *Zero-latency, event-driven HTML/Vanilla JS UI.*
  * Direct DOM manipulation for sub-millisecond hardware synchronization.

---
## 🚀 Development Milestones & Upload Status
*The source code and architectural documentation are being pushed progressively. Current deployment status:*

* [x] **Architecture Blueprint:** System design, repository structure, and data flow established.
* [x] **Safety State Machines & SIL Simulation:** C-code logic for synthetic kinetic noise injection and non-blocking overload handling uploaded.
* [x] **Silicon Configuration:** 200MHz FSP clock tree and Hardware FPU enablement files locked in.
* [x] **Hardware Polling:** Non-blocking matrix keypad math and bitwise scanning logic uploaded.
* [x] **Telemetry Bridge:** Asynchronous full-duplex WebSocket integration via Python uploaded.
* [x] **Logical Security:** Multi-Scan Bitwise Authentication algorithm simulating secure enclaves uploaded.
* [ ] **The Digital Twin UI:** Zero-latency Vanilla JS DOM manipulation and WebSocket front-end integration *(Next up)*.
