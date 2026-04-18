#include "stm32f4xx.h"
#include "pwm.h"

/*
 * TIM2 CH1 on PA5 (onboard LED LD2), AF1
 * PSC=15, ARR=99 -> 16MHz / (16 * 100) = 10 kHz PWM
 * CCR1 = 0..100 maps to duty cycle percentage
 */

void pwm_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PA5 -> AF1 */
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (2U << 10);
    GPIOA->AFR[0] &= ~(0xFU << 20);
    GPIOA->AFR[0] |=  (1U   << 20);

    TIM2->PSC  = 15;
    TIM2->ARR  = 99;
    TIM2->CCR1 = 0;   /* start off */

    /* PWM mode 1, preload enable */
    TIM2->CCMR1 |= (6U << 4) | TIM_CCMR1_OC1PE;
    TIM2->CCER  |= TIM_CCER_CC1E;
    TIM2->CR1   |= TIM_CR1_ARPE | TIM_CR1_CEN;
}
