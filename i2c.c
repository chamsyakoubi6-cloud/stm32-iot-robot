#include "i2c.h"

#define DS1621_ADDR  0x48

void Config_I2C1(void) {
    RCC->APB1ENR |= (1 << 21);

    I2C1->CR1  = (1 << 15);
    I2C1->CR1  = 0;

    I2C1->CR2   = 16;
    I2C1->CCR   = 80;
    I2C1->TRISE = 17;

    I2C1->CR1 |= (1 << 0);
}

static void i2c_start(void) {
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));
}

static void i2c_stop(void) {
    I2C1->CR1 |= (1 << 9);
}

static void i2c_addr_w(uint8_t addr) {
    I2C1->DR = (addr << 1);
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;
}

static void i2c_write(uint8_t data) {
    while (!(I2C1->SR1 & (1 << 7)));
    I2C1->DR = data;
    while (!(I2C1->SR1 & (1 << 2)));
}

void I2C1_Init_DS1621(void) {
    while (I2C1->SR2 & (1 << 1));

    i2c_start();
    i2c_addr_w(DS1621_ADDR);
    i2c_write(0xAC);
    i2c_write(0x02);
    i2c_stop();

    for (volatile uint32_t d = 0; d < 200000; d++);

    i2c_start();
    i2c_addr_w(DS1621_ADDR);
    i2c_write(0xEE);
    i2c_stop();
}

int8_t I2C1_Read_Temp(void) {
    int8_t temp;

    while (I2C1->SR2 & (1 << 1));

    i2c_start();
    i2c_addr_w(DS1621_ADDR);
    i2c_write(0xAA);

    i2c_start();

    I2C1->DR = (DS1621_ADDR << 1) | 1;
    while (!(I2C1->SR1 & (1 << 1)));
    I2C1->CR1 &= ~(1 << 10);
    (void)I2C1->SR2;
    I2C1->CR1 |= (1 << 9);

    while (!(I2C1->SR1 & (1 << 6)));
    temp = (int8_t)I2C1->DR;

    I2C1->CR1 |= (1 << 10);

    return temp;
}