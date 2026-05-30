#include "stm32f4xx.h"
#include "esp32.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void esp_delay(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 8000; i++) __NOP();
}

void SendATCommand(char* cmd) {
    SendString_USART3(cmd);
    SendString_USART3("\r\n");
}

void ESP_Config(void) {
    SendATCommand("AT");
    esp_delay(500);
    SendATCommand("AT+CWMODE=1");
    esp_delay(500);
    SendATCommand("AT+CWQAP");
    esp_delay(500);
    SendString_USART3("AT+CWJAP=\"YOUR_WIFI_SSID\",\"YOUR_WIFI_PASSWORD\"\r\n");
    esp_delay(7000);
    SendATCommand("AT+CIFSR");
    esp_delay(500);
}

void sendAllData(char* http_get, char* json_payload) {
    char at_cmd[60];

    SendATCommand("AT+CIPCLOSE");
    esp_delay(500);

    SendATCommand("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");
    esp_delay(3000);

    int len1 = strlen(http_get);
    sprintf(at_cmd, "AT+CIPSEND=%d", len1);
    SendString_USART3(at_cmd);
    SendString_USART3("\r\n");
    esp_delay(1500);
    SendString_USART3(http_get);
    esp_delay(3000);

    SendATCommand("AT+CIPCLOSE");
    esp_delay(500);

    SendATCommand("AT+CIPSTART=\"TCP\",\"YOUR_NODERED_IP\",1880");
    esp_delay(3000);

    char post_req[400];
    int json_len = strlen(json_payload);
    sprintf(post_req,
        "POST /stm32-data HTTP/1.1\r\n"
        "Host: YOUR_NODERED_IP:1880\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n"
        "%s", json_len, json_payload);

    int len2 = strlen(post_req);
    sprintf(at_cmd, "AT+CIPSEND=%d", len2);
    SendString_USART3(at_cmd);
    SendString_USART3("\r\n");
    esp_delay(1500);
    SendString_USART3(post_req);
    esp_delay(3000);

    SendATCommand("AT+CIPCLOSE");
    esp_delay(500);
}
