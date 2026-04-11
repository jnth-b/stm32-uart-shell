#include "stm32f4xx.h"
#include "uart.h"
#include "ringbuf.h"
#include "commands.h"
#include "adc.h"
#include "pwm.h"

RingBuf_t rx_buf;
volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void)
{
    ms_ticks++;
}

int main(void)
{
    rb_init(&rx_buf);

    /* SysTick: 1 ms tick at 16 MHz HSI */
    SysTick->LOAD = 16000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;

    uart_init();
    dma_uart_tx_init();
    adc_init();
    pwm_init();

    uart_send_string("\r\n=== STM32F446RE Command Shell ===\r\n");
    uart_send_string("Type 'help' for commands\r\n> ");

    char     line[64];
    uint32_t line_idx = 0;

    while (1) {
        char c;
        while (rb_read(&rx_buf, &c)) {
            uart_send_char(c);               /* echo */

            if (c == '\r' || c == '\n') {
                uart_send_string("\r\n");
                line[line_idx] = '\0';
                if (line_idx > 0)
                    parse_and_dispatch(line);
                line_idx = 0;
                uart_send_string("> ");
            } else if (line_idx < 63) {
                line[line_idx++] = c;
            }
        }
    }
}
