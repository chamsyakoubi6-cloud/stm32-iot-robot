#include "motor.h"
#include <string.h>

int speed = 50;

void Config_TIM3(void) {
    RCC->APB1ENR |= (1 << 1);

    TIM3->PSC = 159;
    TIM3->ARR = 99;

    TIM3->CCMR1 = (6 << 4)  | (1 << 3)
                | (6 << 12) | (1 << 11);
    TIM3->CCMR2 = (6 << 4)  | (1 << 3)
                | (6 << 12) | (1 << 11);

    TIM3->CCER = (1 << 0) | (1 << 4) | (1 << 8) | (1 << 12);

    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    TIM3->CCR3 = 0;
    TIM3->CCR4 = 0;

    TIM3->CR1 |= (1 << 7);
    TIM3->EGR  =  1;
}

static void motor_stop(void) {
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    TIM3->CCR3 = 0;
    TIM3->CCR4 = 0;
}

void Motor_Process_Command(char *command) {
    if (command == 0 || command[0] == '\0') return;

    char c = command[0];

    switch (c) {
        case 'F': case 'f':
            TIM3->CCR1 = speed; TIM3->CCR2 = 0;
            TIM3->CCR3 = 0;     TIM3->CCR4 = speed;
            break;
        case 'B': case 'b':
            TIM3->CCR1 = 0;     TIM3->CCR2 = speed;
            TIM3->CCR3 = speed; TIM3->CCR4 = 0;
            break;
        case 'L': case 'l':
            TIM3->CCR1 = 0;     TIM3->CCR2 = speed;
            TIM3->CCR3 = 0;     TIM3->CCR4 = speed;
            break;
        case 'R': case 'r':
            TIM3->CCR1 = speed; TIM3->CCR2 = 0;
            TIM3->CCR3 = speed; TIM3->CCR4 = 0;
            break;
        /* 'G' / 'g' : virage doux droite (pas un spin).
           Roue gauche (CH1/CH2 = PC6/PC7) en avant, roue droite (CH3/CH4
           = PC8/PC9) arretee : le robot avance en courbant vers la droite. */
        case 'G': case 'g':
            TIM3->CCR1 = speed; TIM3->CCR2 = 0;
            TIM3->CCR3 = 0;     TIM3->CCR4 = 0;
            break;
        case 'S': case 's':
            motor_stop();
            break;
        default:
            if (c >= '0' && c <= '9') {
                int v = 0;
                for (int j = 0; command[j] >= '0' && command[j] <= '9'; j++) {
                    v = v * 10 + (command[j] - '0');
                }
                if (v > 99) v = 99;
                speed = v;
            }
            break;
    }
}