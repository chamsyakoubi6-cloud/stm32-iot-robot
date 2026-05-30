#ifndef ADC_H
#define ADC_H

#include "stm32f4xx.h"

extern uint16_t adc_val[3];
extern uint8_t adc_ready; // Ajout de volatile

void Config_TIM2(void);
void Config_DMA2_ADC1(void);
void config_adc1(void);
void TIM2_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);

#endif