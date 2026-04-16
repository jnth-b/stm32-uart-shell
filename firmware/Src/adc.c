#include "stm32f4xx.h"
#include "adc.h"

/* ADC1 Channel 0 on PA0, 12-bit single conversion, software-triggered */

void adc_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    GPIOA->MODER |= (3U << 0);   /* PA0 analog mode */

    ADC1->CR2  = 0;
    ADC1->SQR3 = 0;              /* channel 0 */
    ADC1->SQR1 = 0;              /* 1 conversion */
    ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t adc_read(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC))
        ;
    return (uint16_t)(ADC1->DR);
}
