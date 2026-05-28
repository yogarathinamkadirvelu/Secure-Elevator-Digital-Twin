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
  * **Current Focus (Week 3 Update):** 200MHz hardware clock routing and IEEE-754 Hardware FPU acceleration enabled via `configuration.xml`.
  * Software-in-the-Loop (SIL) kinetic noise simulation and non-blocking safety state machines.
  * Hardware-accelerated CMSIS-DSP FIR filtering to natively crush EMI noise.
  * ARM TrustZone® implementation separating VIP logic (Secure) from telemetry (Non-Secure).
* **`Middleware_Python/`** *Asynchronous `pyserial` translator.*
  * Full-duplex WebSocket bridge breaking the local USB browser sandbox.
* **`Dashboard_Web/`** *Zero-latency, event-driven HTML/Vanilla JS UI.*
  * Direct DOM manipulation for sub-millisecond hardware synchronization.

---
## 🗓️ 20-Week Silicon-to-Screen Engineering Series
*The source code and architectural documentation are being pushed progressively. Active deployment status:*

* [x] **Week 1:** The Cyber-Physical Architecture & Repository Blueprint
* [x] **Week 2:** The Digital Torture Chamber (SIL Noise Simulation logic uploaded)
* [x] **Week 3:** The Silicon Selection (200MHz FSP Clock Tree & Hardware FPU configurations uploaded)
* [ ] **Week 4:** *Next up: The Matrix Keypad Math (Non-Blocking Bitwise Polling)*
