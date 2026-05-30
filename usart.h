#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"
#include <stdint.h>

// USART2 - Debug terminal
void Config_USART2(void);
void SendChar_USART2(char c);
void SendString_USART2(char *pt);
void USART2_IRQHandler(void);

// USART3 - ESP32
void USART3_Config(uint32_t baudrate);
void SendChar_USART3(char c);
void SendString_USART3(char *pt);
void USART3_IRQHandler(void);
void Flush_ESP_To_USART2(void);

// USART1 - Bluetooth
void Config_USART1(uint32_t baudrate);
void SendString_USART1(char *pt);
void USART1_IRQHandler(void);

extern char          RX_BUFFER[50];
extern char          esp_rx_buffer[256];
extern volatile int  esp_rx_index;

#endif