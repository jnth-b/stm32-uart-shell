#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char *s);

/* DMA transmit */
void dma_uart_tx_init(void);
void dma_uart_send(const char *s, uint32_t len);

#endif
