#ifndef DELAY_H
#define DELAY_H

#include "stm32f4xx.h"

extern uint32_t tick_count;

uint32_t GetTick(void);
void Delay_Init(void);
void Delay_ms(uint32_t ms);

#endif