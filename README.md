# 🏢 Hardware-Accelerated Digital Twin for SCADA Elevators

![ARM Cortex-M33](https://img.shields.io/badge/Architecture-ARM%20Cortex--M33-blue)
![Renesas RA6E2](https://img.shields.io/badge/Silicon-Renesas%20RA6E2-red)
![TrustZone](https://img.shields.io/badge/Security-ARM%20TrustZone-green)
![Status](https://img.shields.io/badge/Status-Active%20Development-success)

## 📌 The Cyber-Physical Divide
Software lives in a perfect vacuum. Embedded systems live in the brutal, chaotic physical world. This repository houses the complete architecture for a zero-latency, hardware-in-the-loop Digital Twin designed for a mission-critical industrial elevator system. 

It bridges the gap between raw, noisy analog load cells and a clean, event-driven web UI using strict hardware determinism and mathematical filtering.

## 📂 Architecture Structure

* **`Firmware_RA6E2/`** * Cortex-M33 C code utilizing the Flexible Software Package (FSP).
  * Hardware-accelerated CMSIS-DSP FIR filtering to crush kinetic and EMI noise.
  * ARM TrustZone® implementation separating VIP logic (Secure) from telemetry (Non-Secure).
* **`Middleware_Python/`** * Asynchronous `pyserial` translator.
  * Full-duplex WebSocket bridge breaking the local USB browser sandbox.
* **`Dashboard_Web/`** * Zero-latency, event-driven HTML/Vanilla JS user interface.
  * Direct DOM manipulation for sub-millisecond hardware synchronization.

---
*Note: The source code in these directories is being pushed progressively in tandem with my 20-Week Silicon-to-Screen Engineering Series.*
