#ifndef MOTOR_H
#define MOTOR_H
#include "stm32f4xx.h"

// Variable speed 
extern int speed;

// Prototypes
void Config_TIM3(void);
//  Fonction qui contient fonctionnement  dess moteur
void Motor_Process_Command(char *command); 

#endif