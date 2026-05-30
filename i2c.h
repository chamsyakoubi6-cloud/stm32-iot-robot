#ifndef I2C_H
#define I2C_H

#include "stm32f4xx.h"
#include <stdint.h>  // (Pour int8_t)

void Config_I2C1(void);
void I2C1_Init_DS1621(void);
int8_t I2C1_Read_Temp(void);

#endif