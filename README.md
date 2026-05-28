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
  * **Current Focus:** Hardware FPU acceleration and 200MHz clock routing for zero-latency DSP.
  * Hardware-accelerated CMSIS-DSP FIR filtering (IEEE-754 `float32_t`) to natively crush kinetic and EMI noise without CPU blocking.
  * Software-in-the-Loop (SIL) kinetic noise simulation to safely test state-machine reaction times.
  * ARM TrustZone® implementation separating VIP logic (Secure) from telemetry (Non-Secure).
* **`Middleware_Python/`** *Asynchronous `pyserial` translator.*
  * Full-duplex WebSocket bridge breaking the local USB browser sandbox.
* **`Dashboard_Web/`** *Zero-latency, event-driven HTML/Vanilla JS UI.*
  * Direct DOM manipulation for sub-millisecond hardware synchronization.

---
*Note: The source code in these directories is being pushed progressively in tandem with my 20-Week Silicon-to-Screen Engineering Series. Currently at Week 3: The Silicon Selection.*
