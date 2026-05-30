#include "usart.h"
#include "motor.h"
#include <string.h>

char RX_BUFFER[50];
int  i = 0;

char            esp_rx_buffer[256];
volatile int    esp_rx_index = 0;

/* =====================================================================
   Helper partage par USART1 et USART2 IRQ handlers.
   - Dispatch la commande au moteur (Motor_Process_Command)
   - Forwarde le buffer brut vers l'ESP (USART3) : pipeline IoT intact
   - Echo "[BT] xxx" vers USART2 (visible cote HC-06 / app Android)
   Le HC-06 etant cable sur USART2 (PA2/PA3 d'apres gpio.c), c'est
   USART2_IRQHandler qui va reellement faire tourner les moteurs.
   USART1 garde le meme dispatch par symetrie (cout nul s'il n'est pas cable).
   ===================================================================== */
static void dispatch_command(char *buf, int *idx) {
    if (*idx <= 0) return;
    if (*idx > 49) *idx = 49;
    buf[*idx] = '\0';

    Motor_Process_Command(buf);

    SendString_USART3(buf);
    SendString_USART3("\r\n");

    SendString_USART2("[BT] ");
    SendString_USART2(buf);
    SendString_USART2("\r\n");

    memset(buf, 0, 50);
    *idx = 0;
}

/* =====================================================================
   USART2 - HC-06 Bluetooth @ 9600 baud (PA2=TX, PA3=RX)
   Entree moteur principale. Conserve le forward vers USART3 (ESP).
   ===================================================================== */
void Config_USART2(void) {
    RCC->APB1ENR |= (1 << 17);
    USART2->BRR  = 1667;
    USART2->CR1  = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE
                 | USART_CR1_RXNEIE | USART_CR1_IDLEIE;
    NVIC_SetPriority(USART2_IRQn, 2);
    NVIC_EnableIRQ(USART2_IRQn);
}

void SendChar_USART2(char c) {
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = c;
}

void SendString_USART2(char *pt) {
    while (*pt) SendChar_USART2(*pt++);
}

void USART2_IRQHandler(void) {
    uint32_t sr = USART2->SR;

    if (sr & (1 << 5)) {                /* RXNE */
        char c = (char)USART2->DR;
        if (i < 49) RX_BUFFER[i++] = c;
    }
    if (sr & (1 << 4)) {                /* IDLE: trame BT complete */
        (void)USART2->SR;
        (void)USART2->DR;
        /* HC-06 -> moteur + forward ESP + echo (cf. dispatch_command) */
        dispatch_command(RX_BUFFER, &i);
    }
    if (sr & USART_SR_ORE) {
        (void)USART2->DR;
    }
}

/* =====================================================================
   USART1 - Bluetooth (PA9=TX, PA10=RX)
   ===================================================================== */
static char BT_BUFFER[50];
static int  bt_i = 0;

void Config_USART1(uint32_t baudrate) {
    RCC->APB2ENR |= (1 << 4);   // USART1EN
    USART1->BRR  = 16000000U / baudrate;
    USART1->CR1  = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE
                 | USART_CR1_RXNEIE | USART_CR1_IDLEIE;
    NVIC_SetPriority(USART1_IRQn, 2);
    NVIC_EnableIRQ(USART1_IRQn);
}

void SendString_USART1(char *pt) {
    while (*pt) {
        while (!(USART1->SR & (1 << 7)));
        USART1->DR = *pt++;
    }
}

void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;

    if (sr & (1 << 5)) {                /* RXNE */
        char c = (char)USART1->DR;
        if (bt_i < 49) BT_BUFFER[bt_i++] = c;
    }
    if (sr & (1 << 4)) {                /* IDLE */
        (void)USART1->SR;
        (void)USART1->DR;
        /* Meme dispatch que USART2 (helper partage) */
        dispatch_command(BT_BUFFER, &bt_i);
    }
    if (sr & USART_SR_ORE) {
        (void)USART1->DR;
    }
}


void USART3_Config(uint32_t baudrate) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER  &= ~((3U << 20) | (3U << 22));
    GPIOB->MODER  |=  ((2U << 20) | (2U << 22));
    GPIOB->OSPEEDR|=  ((3U << 20) | (3U << 22));
    GPIOB->PUPDR  &= ~((3U << 20) | (3U << 22));
    GPIOB->AFR[1] &= ~((0xFU << 8) | (0xFU << 12));
    GPIOB->AFR[1] |=  ((7U << 8)   | (7U << 12));

    RCC->APB1ENR  |=  RCC_APB1ENR_USART3EN;
    RCC->APB1RSTR |=  RCC_APB1RSTR_USART3RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

    USART3->BRR = 16000000U / baudrate;
    USART3->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE
                | USART_CR1_RXNEIE;
    NVIC_SetPriority(USART3_IRQn, 2);
    NVIC_EnableIRQ(USART3_IRQn);
}

void SendChar_USART3(char c) {
    while (!(USART3->SR & (1 << 7)));
    USART3->DR = c;
}

void SendString_USART3(char *pt) {
    while (*pt) SendChar_USART3(*pt++);
}

void USART3_IRQHandler(void) {
    uint32_t sr = USART3->SR;
    if (sr & USART_SR_RXNE) {
        char c = (char)USART3->DR;
        if (esp_rx_index < 255) {
            esp_rx_buffer[esp_rx_index++] = c;
            esp_rx_buffer[esp_rx_index]   = '\0';
        }
    }
    if (sr & USART_SR_ORE) {
        (void)USART3->DR;
    }
}

void Flush_ESP_To_USART2(void) {
    if (esp_rx_index > 0) {
        NVIC_DisableIRQ(USART3_IRQn);
        SendString_USART2("\r\n[ESP] ");
        SendString_USART2(esp_rx_buffer);
        esp_rx_index     = 0;
        esp_rx_buffer[0] = '\0';
        NVIC_EnableIRQ(USART3_IRQn);
    }
}