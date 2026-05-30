#ifndef __ESP32_H
#define __ESP32_H

#include <stdint.h>

void SendATCommand(char* cmd);
void ESP_Config(void);
void sendAllData(char* http_get, char* json_payload);
void esp_delay(uint32_t ms);

#endif
