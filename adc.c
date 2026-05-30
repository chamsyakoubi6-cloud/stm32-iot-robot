#include "adc.h"

uint16_t adc_val[3];
uint8_t  adc_ready = 0;

void Config_TIM2(void) {
    RCC->APB1ENR |= (1 << 0);
    TIM2->PSC     = 15999;
    TIM2->ARR     = 2999;
    TIM2->DIER   |= (1 << 0);
    NVIC_SetPriority(TIM2_IRQn, 3);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void Config_DMA2_ADC1(void) {
    RCC->AHB1ENR |= (1 << 22);

    DMA2_Stream0->CR &= ~1;
    while (DMA2_Stream0->CR & 1);

    DMA2->LIFCR |= (1 << 5);

    DMA2_Stream0->PAR  = (unsigned)&ADC1->DR;
    DMA2_Stream0->M0AR = (unsigned)adc_val;
    DMA2_Stream0->NDTR = 3;

    DMA2_Stream0->CR = (0 << 25) | (1 << 13) | (1 << 11) | (1 << 10) | (1 << 8) | (1 << 4);

    NVIC_SetPriority(DMA2_Stream0_IRQn, 3);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    DMA2_Stream0->CR |= 1;
}

void config_adc1(void) {
    RCC->APB2ENR |= (1 << 8);
    ADC1->SMPR1   = (4 << 0) | (4 << 3) | (4 << 6);
    ADC1->CR1     = (1 << 8);
    ADC1->CR2     = (1 << 9) | (1 << 8);
    ADC1->SQR1    = (2 << 20);
    ADC1->SQR3    = (10 << 0) | (11 << 5) | (12 << 10);
    ADC1->CR2    |= 1;
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & 1) {
        TIM2->SR  &= ~1;
        ADC1->CR2 |= (1 << 30);
    }
}

void DMA2_Stream0_IRQHandler(void) {
    if (DMA2->LISR & (1 << 5)) {
        DMA2->LIFCR = (1 << 5);
        adc_ready = 1;
        __DSB();  /* Synchronise memoire sans volatile */
    }
}