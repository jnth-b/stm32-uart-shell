# Bare-Metal UART Command Shell (STM32F446RE)

[![Target](https://img.shields.io/badge/MCU-STM32F446RE-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f446re.html)
[![Core](https://img.shields.io/badge/Core-ARM%20Cortex--M4-green.svg)](https://developer.arm.com/Processors/Cortex-M4)
[![HAL](https://img.shields.io/badge/HAL-None%20(Bare--Metal)-red.svg)]()
[![Toolchain](https://img.shields.io/badge/Toolchain-ARM%20GCC%20%2B%20Make-orange.svg)]()
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)

A zero-HAL, register-level interactive UART command shell built for the **STM32F446RE Nucleo-64** development board. Designed for low-latency hardware diagnostics, peripheral validation, and deterministic telemetry without vendor library bloat.

---

## ⚡ Architectural Highlights
- **Direct Register Manipulation**: Peripheral initialization and control executed via direct memory-mapped register bit manipulation (`RCC`, `GPIOA`, `USART2`, `DMA1`, `TIM2`, `ADC1`). No STM32 HAL or LL dependencies.
- **Interrupt-Driven RX with Circular Ring Buffer**: A lock-free 128-byte ring buffer decouples the UART receiver ISR from command parsing, ensuring zero character drops at 115200 baud during command evaluation.
- **Non-Blocking DMA TX**: Diagnostic telemetry and command responses streamed over **DMA1 Stream6 Channel 4**, freeing CPU cycles for main-loop execution.
- **Function-Pointer Dispatch Table**: Command evaluation uses a bounded token parser linked to a static command dispatch lookup table.
- **Hardware Integration**: On-chip 12-bit ADC single conversion, TIM2 hardware PWM brightness control, and SysTick 1 ms system uptime counter.

---

## 📐 System Architecture

```
UART RX Pin (PA3) ──> USART2_IRQHandler ──> [ 128-Byte Ring Buffer ]
                                                    │
                                           Main Loop Parsing
                                                    ▼
                                         [ Command Tokenizer ]
                                                    │
                 ┌──────────────┬───────────────────┴──────────────────┬─────────────┐
                 ▼              ▼                                      ▼             ▼
            `help` cmd     `adc` cmd                              `pwm` cmd     `uptime` cmd
           (Menu List)    (PA0 12-Bit ADC)                       (TIM2 CH1)    (SysTick 1ms)
                 │              │                                      │             │
                 └──────────────┴───────────────────┬──────────────────┴─────────────┘
                                                    ▼
                                        [ DMA1 Stream6 TX Buffer ] ──> [ UART TX: PA2 ]
```

---

## 💻 Console Command Reference

Connect your serial terminal at **115200 Baud, 8 Data Bits, No Parity, 1 Stop Bit (8N1)**.

| Command | Arguments | Peripheral | Description |
|:---|:---|:---|:---|
| `help` | None | Dispatcher | Displays all available commands and syntax instructions |
| `led` | `<on\|off>` | GPIOA PIN 5 | Toggles green onboard user LED (LD2) state |
| `adc` | None | ADC1 Channel 0 | Initiates single conversion on pin PA0; returns raw value & millivolts |
| `pwm` | `<0-100>` | TIM2 Channel 1 | Modulates TIM2 CCR1 register for 0–100% duty cycle LED dimming |
| `uptime`| None | SysTick Timer | Reports system uptime since hardware reset in milliseconds |
| `status`| None | RCC / USART2 | Dumps core clock frequency, peripheral bus speed, and buffer watermarks |

### Example Serial Session
```text
stm32> help
=== Available Commands ===
  help           - Show command menu
  led <on|off>   - Toggle user LED2
  adc            - Read analog input PA0 (mV)
  pwm <0-100>    - Set LED PWM brightness
  uptime         - Print uptime in ms
  status         - System diagnostics
==========================

stm32> uptime
Uptime: 45210 ms

stm32> adc
ADC1_IN0: 1650 mV (Raw: 2048)

stm32> pwm 75
TIM2_CH1 PWM Duty Cycle set to 75%

stm32> led on
LED2 state: ON
```

---

## 🛠️ Hardware Setup & Pin Mapping

| Peripheral | Nucleo-64 Pin | Signal Name | Register Function |
|:---|:---|:---|:---|
| **USART2 TX** | PA2 | CN10 Pin 35 | Alternate Function AF7 (Connected to ST-Link VCP) |
| **USART2 RX** | PA3 | CN10 Pin 37 | Alternate Function AF7 (Connected to ST-Link VCP) |
| **User LED2** | PA5 | CN10 Pin 11 | GPIO Output / TIM2 Channel 1 PWM (AF1) |
| **ADC Input** | PA0 | CN8 Pin 1 (A0) | Analog Mode (`MODER = 11`) |
| **GND** | GND | CN6 Pin 6 | System Ground Reference |

---

## 🚀 Building and Flashing

### Prerequisites
- **Compiler**: `arm-none-eabi-gcc`
- **Flashing Tool**: `st-flash` (from `stlink` toolset) or `openocd`
- **Serial Terminal**: `screen`, `minicom`, or `PuTTY`

### Commands
```bash
# Clone the repository
git clone https://github.com/jnth-b/stm32-uart-shell.git
cd stm32-uart-shell/firmware

# Compile bare-metal firmware
make

# Flash binary to Nucleo board via ST-Link
make flash

# Launch serial terminal (macOS / Linux)
make serial
# Or manually: screen /dev/tty.usbmodem* 115200
```

---

## 📂 Project Structure
```text
stm32-uart-shell/
├── firmware/
│   ├── Inc/
│   │   ├── stm32f446xx.h      # Device register definitions
│   │   ├── uart.h             # USART2 & ring buffer interface
│   │   ├── dma.h              # DMA1 Stream6 configuration
│   │   ├── shell.h            # Command parser & dispatch table
│   │   └── adc.h              # ADC1 single conversion driver
│   ├── Src/
│   │   ├── main.c             # System init & main polling loop
│   │   ├── uart.c             # Register-level UART driver & ISR
│   │   ├── dma.c              # DMA transmission routines
│   │   ├── shell.c            # Command tokenizer & handlers
│   │   └── adc.c              # Direct register ADC sampling
│   ├── Makefile               # Optimized GCC build and flash rules
│   └── startup_stm32f446xx.s  # Vector table & reset handler
├── .gitignore
└── README.md
```

---

## 📄 License
This project is open-source under the [MIT License](LICENSE).
