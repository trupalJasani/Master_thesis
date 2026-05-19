# AgriNode: Master Thesis

An ultra-low-power ESP32-C3 edge node designed for crop disease forecasting over LoRa P2P.


## 📌 Project Overview
This repository contains the firmware for an ultra-low-power agricultural edge computing node, developed as part of a Master's thesis. The primary objective is to collect data and compute the **Smith Period** (a predictive index for crop disease) directly at the edge, reducing bandwidth by transmitting only the calculated forecasting index over a LoRa P2P network.

## ⚙️ Hardware Stack
* **Microcontroller:** ESP32-C3 (RISC-V, Wi-Fi/Bluetooth disabled for power savings)
* **Telemetry Radio:** Seeed Wio-E5 LoRa Module (UART AT Commands)
* **Sensors:**
    * **Sensirion SHT31:** Ambient Temperature & Humidity (I2C)
    * **Davis 6420 Leaf Wetness:** Analog moisture grid (ADC)
    * **Capacitive Soil Moisture:** Analog soil permittivity (ADC)
* **Power Management:** Deep sleep architecture with an N-Channel MOSFET on GPIO 10 to completely sever power to external peripherals during sleep cycles.

## 🏛️ Software Architecture
The codebase is designed using professional embedded software engineering principles, prioritizing portability, hardware abstraction, and memory safety.

### 1. Inversion of Control (IoC) & Object-Oriented C
All sensor drivers (`sht31`, `davis_6420`, `soil_moisture`) are written in strictly platform-agnostic, pure C. They contain zero ESP-IDF specific headers. Instead, they rely on a function-pointer architecture (Adapter Pattern). Hardware capabilities are injected into the sensor objects during initialization.

### 2. Board Support Package (BSP)
All hardware-specific ESP32-C3 code (I2C initialization, ADC configuration, GPIO switching) is isolated within the `bsp` component. The BSP serves as the translation layer between the abstract sensor logic and the physical silicon.

### 3. Memory Safety
To guarantee long-term stability in the field, this firmware relies entirely on **Static Allocation**. There is zero use of dynamic memory allocation (`malloc()`), eliminating the risk of heap fragmentation or memory leaks over multi-month deployments.

## 📂 Directory Structure

```text
├── components/
│   ├── bsp/                # Hardware Abstraction Layer (Pins, I2C, ADC wrappers)
│   ├── davis_6420/         # Pure C Davis Leaf Wetness driver
│   ├── sht31/              # Pure C Sensirion SHT31 driver
│   └── soil_moisture/      # Pure C Capacitive Soil Moisture driver
├── main/
│   ├── CMakeLists.txt      # ESP-IDF build configuration
│   ├── logic_smith_period.c # Agronomic math and threshold calculations
│   └── main.c              # Finite State Machine (Wake -> Read -> Process -> Transmit -> Sleep)
├── .gitignore              # Ignores build/ and SDK config files
├── CMakeLists.txt          # Project-level CMake
└── README.md               # Project documentation