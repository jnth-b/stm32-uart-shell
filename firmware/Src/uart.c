#include "stm32f4xx.h"
#include "uart.h"
#include "ringbuf.h"
#include <string.h>

extern RingBuf_t rx_buf;

/*
 * USART2 on PA2 (TX) and PA3 (RX)
 * Both pins hardwired to ST-LINK virtual COM port on the Nucleo.
 * AF7 = USART2. PCLK1 = 16 MHz (HSI, no PLL).
 * BRR for 115200 baud: 16000000 / 115200 ~ 138.89 -> 0x008B
 */

void uart_init(void)
{
    /* enable clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2 -> AF7 (TX) */
    GPIOA->MODER  &= ~(3U << 4);
    GPIOA->MODER  |=  (2U << 4);
    GPIOA->AFR[0] &= ~(0xFU << 8);
    GPIOA->AFR[0] |=  (7U   << 8);

    /* PA3 -> AF7 (RX) */
    GPIOA->MODER  &= ~(3U << 6);
    GPIOA->MODER  |=  (2U << 6);
    GPIOA->AFR[0] &= ~(0xFU << 12);
    GPIOA->AFR[0] |=  (7U   << 12);

    /* 115200 baud, 8N1 */
    USART2->BRR = 0x008B;
    USART2->CR1 = USART_CR1_TE
                | USART_CR1_RE
                | USART_CR1_RXNEIE
                | USART_CR1_UE;

    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
}

void uart_send_char(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
        ;
    USART2->DR = c;
}

void uart_send_string(const char *s)
{
    while (*s)
        uart_send_char(*s++);
}

/* USART2 RX interrupt -- stuff byte into ring buffer */
void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE) {
        char c = (char)(USART2->DR & 0xFF);
        rb_write(&rx_buf, c);
    }

    /* clear overrun error to prevent ISR lockup (RM0390 sec 27.6.1) */
    if (USART2->SR & USART_SR_ORE) {
        (void)USART2->SR;
        (void)USART2->DR;
    }
}

/* ---- DMA1 Stream6 Channel4 -- USART2 TX (RM0390 Table 28) ---- */

#define TX_BUF_SIZE 256
static char           tx_buf[TX_BUF_SIZE];
volatile uint8_t      dma_tx_busy = 0;

void dma_uart_tx_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    USART2->CR3 |= USART_CR3_DMAT;

    /* disable stream before configuring */
    DMA1_Stream6->CR  = 0;
    while (DMA1_Stream6->CR & DMA_SxCR_EN)
        ;

    DMA1->HIFCR = 0x003F0000;   /* clear Stream6 flags */

    /* ch4, mem-to-periph, memory increment, transfer complete IRQ */
    DMA1_Stream6->CR  = (4U << DMA_SxCR_CHSEL_Pos)
                      | DMA_SxCR_MINC
                      | (1U << DMA_SxCR_DIR_Pos)
                      | DMA_SxCR_TCIE;

    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;

    NVIC_SetPriority(DMA1_Stream6_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void dma_uart_send(const char *s, uint32_t len)
{
    if (len == 0 || len > TX_BUF_SIZE) return;

    while (dma_tx_busy)
        ;

    memcpy(tx_buf, s, len);
    DMA1_Stream6->M0AR = (uint32_t)tx_buf;
    DMA1_Stream6->NDTR = len;
    DMA1->HIFCR        = 0x003F0000;
    dma_tx_busy         = 1;
    DMA1_Stream6->CR   |= DMA_SxCR_EN;
}

void DMA1_Stream6_IRQHandler(void)
{
    DMA1->HIFCR       = 0x003F0000;
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    dma_tx_busy        = 0;
}
