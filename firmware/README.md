# stm32-uart-shell

Bare-metal UART command shell for the STM32F446RE Nucleo board. No HAL - all register-level.

I built this after my first STM32 project (an IMU data logger) because I wanted something interactive. The IMU project was all one-way data streaming, so I never had to deal with receiving data on the MCU side. This project forced me to figure out interrupt-driven RX, ring buffers, and DMA transmit - all things I'd only read about before.

## What it does

You plug in the Nucleo via USB, open a serial terminal, and get a command prompt. You can control the onboard LED, read an ADC pin, check uptime, etc. It's basically a tiny debug shell.

```
=== STM32F446RE Command Shell ===
Type 'help' for commands
> help
Commands:
  help          - show this message
  led [on|off]  - toggle onboard LED
  adc           - read voltage on PA0
  pwm [0-100]   - set LED brightness
  uptime        - time since boot
  status        - dump peripheral state
> adc
ADC PA0: raw=2047  voltage=1648mV
> pwm 50
PWM duty set to 50%
```

## Hardware

- STM32F446RE Nucleo-64 (that's it - no extra components needed)
- USB cable for power + serial

| Pin | Function |
|-----|----------|
| PA2 | USART2 TX (goes through ST-LINK to your PC) |
| PA3 | USART2 RX |
| PA0 | ADC input - wire something here or it'll read noise |
| PA5 | PWM output (this is the green LED on the Nucleo) |

## Build & flash

You need `arm-none-eabi-gcc` and `st-flash`.

```bash
cd firmware
make          # build
make flash    # write to board
make serial   # open terminal (115200 baud)
```

To quit the serial terminal: `Ctrl-A`, then `K`, then `Y`.

## How it works

```
main.c       - SysTick init, main loop (accumulates typed characters into a line)
uart.c       - USART2 setup, polling TX for echo, RX interrupt, DMA TX for responses
ringbuf.c    - lock-free ring buffer between ISR and main loop
commands.c   - function pointer dispatch table + all the command handlers
adc.c        - ADC1 single conversion on PA0
pwm.c        - TIM2 CH1 PWM on PA5, 10 kHz
```

The key design decision was separating RX and TX approaches: incoming bytes hit an interrupt and get stuffed into a ring buffer (keeps the ISR fast), while outgoing command responses get sent via DMA so the CPU isn't stuck waiting. Single-character echo still uses polling TX since DMA setup overhead isn't worth it for one byte.

## Known issues / limitations

- No backspace support - if you mistype, the whole line gets parsed as-is
- Ring buffer is 128 bytes. If you somehow flood the input faster than the main loop can drain it, bytes get dropped. Never actually hit this at 115200 baud though
- The `atoi` call in the PWM command doesn't validate input - `pwm abc` just sets duty to 0
- Lines longer than 63 chars get silently truncated

## Clock config

Running on the default 16 MHz HSI (no PLL). Baud rate divider, SysTick reload, and timer prescaler are all calculated for 16 MHz. If you enable the PLL, all three need to be recalculated.

## References

- [RM0390](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) - sec 27 USART, sec 10 DMA, sec 18 ADC, sec 17 TIM2
