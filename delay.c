#include "stm32f4xx.h"
#include "delay.h"

uint32_t tick_count = 0;

void SysTick_Handler(void) {
    tick_count++;
}

uint32_t GetTick(void) {
    return tick_count;
}

void Delay_Init(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
}

void Delay_ms(uint32_t ms) {
    uint32_t start = tick_count;
    while ((tick_count - start) < ms);
}