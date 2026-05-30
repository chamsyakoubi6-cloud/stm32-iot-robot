#include "gpio.h"

void Config_GPIO(void) {
    RCC->AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    // --- ADC Inputs: PC0, PC1, PC2 (Analog)
    GPIOC->MODER |= (0b11 << 0) | (0b11 << 2) | (0b11 << 4);

    // --- PWM Pins: PC6, PC7, PC8, PC9 (AF2)
    GPIOC->MODER |= (0b10 << 12) | (0b10 << 14) | (0b10 << 16) | (0b10 << 18);
    GPIOC->AFR[0] |= (2 << 24) | (2 << 28);
    GPIOC->AFR[1] |= (2 << 0)  | (2 << 4);

    // --- USART2 Pins: PA2 (TX), PA3 (RX) (AF7)
    GPIOA->MODER |= (0b10 << 4) | (0b10 << 6);
    GPIOA->AFR[0] |= (7 << 8) | (7 << 12);

    // --- USART1 Pins: PA9 (TX), PA10 (RX) (AF7)
    GPIOA->MODER  |= (0b10 << 18) | (0b10 << 20);
    GPIOA->AFR[1] |= (7 << 4) | (7 << 8);

    // --- I2C Pins: PB6, PB7 (AF4)
    GPIOB->MODER  |= (0b10 << 12) | (0b10 << 14);
    GPIOB->OTYPER |= (0b1 << 6) | (0b1 << 7);
    GPIOB->PUPDR  |= (0b01 << 12) | (0b01 << 14);
    GPIOB->AFR[0] |= (4 << 24) | (4 << 28);

    // --- Button PA0 (Input + Pull Down pour eviter les faux contacts)
    GPIOA->MODER &= ~0b11;
    GPIOA->PUPDR &= ~0b11;
    GPIOA->PUPDR |= (0b10 << 0);
}