#ifndef EXTI_H
#define EXTI_H

#include "stm32f4xx.h"

extern uint8_t button_pressed;

void config_EXTI(void);
void EXTI0_IRQHandler(void);

#endif