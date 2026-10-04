# STM32 IoT Mobile Robot 🤖

A **Bluetooth-controlled mobile robot** built on the **STM32F407** microcontroller, with **real-time IoT supervision** of its analog sensors and temperature. The firmware is written entirely **bare-metal** (direct register access, **no HAL / no CubeMX**), and streams telemetry to the cloud (**ThingSpeak**) and to a local **Node-RED + Mosquitto MQTT** dashboard through an **ESP32** WiFi bridge.

> The robot is **manually driven** (not autonomous): movement commands are sent live from an Android Serial Bluetooth terminal over an **HC-06** module.

![Language: C](https://img.shields.io/badge/Language-C-00599C?logo=c&logoColor=white)
![Platform: STM32F407](https://img.shields.io/badge/Platform-STM32F407-03234B?logo=stmicroelectronics&logoColor=white)
![Firmware: Bare--metal](https://img.shields.io/badge/Firmware-Bare--metal%20(no%20HAL)-orange)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)

---

## ✨ Features

- 🎮 **Live Bluetooth driving** — directional + speed commands over HC-06 (USART2 @ 9600 baud), echoed back to an Android serial terminal.
- ⚙️ **4-motor differential drive** — TIM3 hardware PWM (4 channels) driving an L298N H-bridge.
- 🌡️ **Digital temperature sensing** — DS1621 over I²C1.
- 📈 **3 analog sensors** — sampled by ADC1 with DMA, triggered periodically by TIM2 (~every 3 s).
- ☁️ **Cloud telemetry** — HTTP GET to ThingSpeak (4 fields) via ESP32 AT commands.
- 🏠 **Local IoT dashboard** — HTTP POST (JSON) to Node-RED, re-published over Mosquitto MQTT to a real-time dashboard.
- 🛑 **Hardware emergency stop / start** — push-button on PA0 via EXTI interrupt.
- 🧱 **100 % bare-metal C** — every peripheral configured directly through CMSIS register definitions.

---

## 🔩 Hardware Architecture

| Component | Role | Interface | MCU connection |
|-----------|------|-----------|----------------|
| **STM32F407** | Main controller (bare-metal) | — | Runs from 16 MHz HSI |
| **ESP32** | WiFi bridge (AT firmware) | USART3 @ 115200 | PB10 (TX) / PB11 (RX) |
| **HC-06** | Bluetooth (robot control) | USART2 @ 9600 | PA2 (TX) / PA3 (RX) |
| **DS1621** | Temperature sensor (addr `0x48`) | I²C1 | PB6 (SCL) / PB7 (SDA) |
| **3× analog sensors** | Environment sensing | ADC1 + DMA2 | PC0 / PC1 / PC2 |
| **4× DC motors** | Differential drive (via **L298N**) | TIM3 PWM | PC6 / PC7 / PC8 / PC9 |
| **Push-button** | Emergency stop / start | EXTI0 | PA0 |
| **Secondary BT/UART** | Optional control input | USART1 @ 9600 | PA9 (TX) / PA10 (RX) |

---

## 🧠 Software Architecture

Each peripheral lives in its own module (`*.c` / `*.h`), configured directly through registers:

| Module | Peripheral | What it does |
|--------|-----------|--------------|
| `gpio.c` | **GPIO** | Pin modes / alternate functions for every peripheral below |
| `motor.c` | **TIM3 PWM** | 4-channel PWM (1 kHz), parses motor commands, sets duty (speed 0–99) |
| `adc.c` | **ADC1 + DMA2 + TIM2** | TIM2 triggers ADC1, 3 channels scanned, DMA2 Stream0 (circular) writes `adc_val[3]`, sets `adc_ready` |
| `i2c.c` | **I²C1** | DS1621 init + temperature read (`I2C1_Read_Temp`) |
| `usart.c` | **USART1/2/3** | IRQ-driven RX with IDLE-line framing; USART2 = HC-06, USART3 = ESP32, shared command dispatch |
| `exti.c` | **EXTI0** | PA0 button → emergency stop / start toggle (gates TIM3 & TIM2) |
| `esp32.c` | **ESP32 driver** | AT command sequencing: WiFi join, TCP connect, ThingSpeak GET + Node-RED POST |
| `delay.c` | **Delay** | Simple blocking delay helper |
| `main.c` | **Application** | Boot/init sequence + main loop (read sensors → build payloads → `sendAllData`) |

**Control flow (main loop):** every time `adc_ready` is set (≈ every 3 s by TIM2), `main` reads the DS1621 temperature, formats a ThingSpeak HTTP GET and a JSON payload, and calls `sendAllData()` which pushes both over the ESP32.

**Command flow (interrupt-driven):** a Bluetooth byte stream arrives on USART2; the IDLE-line interrupt marks a complete frame, which is dispatched to `Motor_Process_Command()` and also forwarded to the ESP32 and echoed back to the Android terminal.

---

## 🌐 IoT Pipeline

```
┌────────────────┐   ADC×3 + DS1621 temp  (every ~3 s)
│   Sensors      │ ─────────────────────────────────────┐
│ PC0/PC1/PC2    │                                       │
│ DS1621 (I2C)   │                                       ▼
└────────────────┘                              ┌──────────────────┐
                                                │    STM32F407     │
┌────────────────┐  F/B/L/R/S + speed (BT)      │   (bare-metal)   │
│  HC-06  +       │ ────────────────────────────▶│  USART2 @ 9600   │
│  Android app    │                              └────────┬─────────┘
└────────────────┘                                       │ USART3 @ 115200
                                                          │ (AT commands)
                                                          ▼
                                                 ┌──────────────────┐
                                                 │      ESP32       │
                                                 │   (WiFi STA)     │
                                                 └───┬──────────┬───┘
                                      HTTP GET       │          │   HTTP POST (JSON)
                                                     ▼          ▼
                                         ┌────────────────┐  ┌────────────────────┐
                                         │  ThingSpeak    │  │     Node-RED       │
                                         │  cloud charts  │  │ :1880  /stm32-data │
                                         │  field1..4     │  └─────────┬──────────┘
                                         └────────────────┘            │ MQTT publish
                                                                       ▼
                                                            ┌────────────────────┐
                                                            │  Mosquitto Broker  │
                                                            │       (MQTT)       │
                                                            └─────────┬──────────┘
                                                                      │ subscribe
                                                                      ▼
                                                            ┌────────────────────┐
                                                            │  Node-RED Dashboard│
                                                            │   (real-time UI)   │
                                                            └────────────────────┘
```

**Two parallel sinks** are fed on every cycle:
1. **ThingSpeak** — `GET /update?api_key=...&field1..4` to `api.thingspeak.com:80` (cloud charts / history).
2. **Node-RED** — `POST /stm32-data` with `{"adc1","adc2","adc3","temp"}` to your Node-RED instance on `:1880`, which re-publishes over **Mosquitto MQTT** to a live dashboard.

---

## 📌 Pin Mapping

| Pin | Peripheral | Function | AF / Mode |
|-----|-----------|----------|-----------|
| PA0  | EXTI0  | Button (emergency stop / start) | Input, pull-down |
| PA2  | USART2 | TX → HC-06 RX | AF7 |
| PA3  | USART2 | RX ← HC-06 TX | AF7 |
| PA9  | USART1 | TX (secondary UART) | AF7 |
| PA10 | USART1 | RX (secondary UART) | AF7 |
| PB6  | I²C1   | SCL → DS1621 | AF4, open-drain |
| PB7  | I²C1   | SDA ↔ DS1621 | AF4, open-drain |
| PB10 | USART3 | TX → ESP32 RX | AF7 |
| PB11 | USART3 | RX ← ESP32 TX | AF7 |
| PC0  | ADC1_IN10 | Analog sensor 1 | Analog |
| PC1  | ADC1_IN11 | Analog sensor 2 | Analog |
| PC2  | ADC1_IN12 | Analog sensor 3 | Analog |
| PC6  | TIM3_CH1 | Motor PWM (left fwd)  | AF2 |
| PC7  | TIM3_CH2 | Motor PWM (left rev)  | AF2 |
| PC8  | TIM3_CH3 | Motor PWM (right rev) | AF2 |
| PC9  | TIM3_CH4 | Motor PWM (right fwd) | AF2 |

---

## 🎮 Bluetooth Commands

Send single-letter commands (case-insensitive) over the HC-06 link (9600 baud). Send a **number** (`0`–`99`) to set the PWM speed used by the next move.

| Command | Action |
|:-------:|--------|
| `F` | Move **forward** |
| `B` | Move **backward** |
| `L` | Turn **left** |
| `R` | Turn **right** |
| `G` | Soft **right curve** (forward, gentle turn — not a spin) |
| `S` | **Stop** all motors |
| `0`–`99` | Set **speed** (PWM duty, capped at 99) |

> Example session: `50` → `F` drives forward at 50 % duty; `S` stops.
> The **PA0 button** acts as a hardware emergency stop / start toggle independently of Bluetooth.

---

## 🚀 Getting Started

### 1. Prerequisites
- **Keil MDK-ARM (µVision V5)** with the **STM32F4xx_DFP** device pack installed
  *(the `RTE/` folder — startup & system files — is regenerated by Keil from this pack, which is why it is git-ignored).*
- **ST-Link** programmer/debugger.
- **ESP32** flashed with **AT firmware**, an **HC-06** module, a **DS1621**, and an **L298N** motor driver.

### 2. Build & flash
```bash
git clone https://github.com/chamsyakoubi6-cloud/stm32-iot-robot.git
```
1. Open `projet_esp.uvprojx` in Keil µVision.
2. Build the project (**F7**).
3. Connect the ST-Link and flash (**Load / F8**).

### 3. Configure your credentials
Replace the placeholders before building:

| Placeholder | File | Set it to |
|-------------|------|-----------|
| `YOUR_WIFI_SSID` | `esp32.c` | Your WiFi network name |
| `YOUR_WIFI_PASSWORD` | `esp32.c` | Your WiFi password |
| `YOUR_NODERED_IP` | `esp32.c` (×2) | Your Node-RED host IP |
| `YOUR_THINGSPEAK_API_KEY` | `main.c` | Your ThingSpeak **Write API key** |

### 4. ThingSpeak
1. Create a channel with **4 fields** (sensor 1–3 + temperature).
2. Copy the **Write API Key** into `main.c`.

### 5. Node-RED + Mosquitto
1. Install & run the **Mosquitto** MQTT broker (default port `1883`).
2. In **Node-RED** (port `1880`), build a flow:
   `HTTP In (POST /stm32-data)` → `JSON parse` → `MQTT out` (to Mosquitto) → `HTTP response`.
3. Add `MQTT in` + **dashboard** nodes (gauges / charts) to visualize the data live.
4. Put the machine's IP into `YOUR_NODERED_IP` in `esp32.c`.

### 6. Drive the robot
1. Pair the **HC-06** with your phone.
2. Open a **Serial Bluetooth Terminal** app at **9600 baud**.
3. Send commands from the [Bluetooth Commands](#-bluetooth-commands) table.

---

## 📁 Project Structure

```
.
├── main.c              # Init sequence + main telemetry loop
├── gpio.c / .h         # GPIO & alternate-function setup
├── motor.c / .h        # TIM3 PWM + Bluetooth command parser
├── adc.c / .h          # ADC1 + DMA2 + TIM2 periodic sampling
├── i2c.c / .h          # I2C1 driver for the DS1621
├── usart.c / .h        # USART1/2/3 (IRQ + IDLE-line framing)
├── exti.c / .h         # PA0 button (emergency stop / start)
├── esp32.c / .h        # ESP32 AT-command driver (WiFi + HTTP)
├── delay.c / .h        # Blocking delay helper
├── projet_esp.uvprojx  # Keil µVision project
├── projet_esp.uvoptx   # Keil µVision options
├── README.md           # Documentation (this file)
├── LICENSE             # MIT license
└── .gitignore          # Ignored build artifacts & editor files
```

---

## 📜 License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for details.

© 2026 **Chamsy Yakoubi**
