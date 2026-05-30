#include "exti.h"
#include "motor.h" //  TIM3 et speed
#include "adc.h"   //  TIM2 (dans config TIM2)


void config_EXTI(void) {
    RCC->APB2ENR |= (1 << 14);
// activation de péripherique SYSCFG bit 14
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
// Configure EXTI0 sur PA0
    EXTI->IMR |= EXTI_IMR_IM0;
// IMR = Interrupt Mask Register

// IM0 = autoriser EXTI0
    EXTI->RTSR |= EXTI_RTSR_TR0;
    NVIC_SetPriority(EXTI0_IRQn, 1);
// 1 = priorité élevée
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR0) {

        if (TIM3->CR1 & 1) { // emmergency STOP
            TIM3->CR1 &= ~1;
            TIM2->CR1 &= ~1;
            TIM3->CCR1 = 0; TIM3->CCR2 = 0;
            TIM3->CCR3 = 0; TIM3->CCR4 = 0;
            speed = 300;
        } else { // START
            TIM3->CR1 |= 1;
// Démarre TIM3 ? moteurs ON
            TIM2->CR1 |= 1;
//Démarre TIM2 ? ADC ON
        }
        EXTI->PR |= EXTI_PR_PR0;
// Efface le flag d’interruption EXTI0  si non interruption infinie
    }
}