#ifndef USART_H
#define USART_H

#include "main.h"

void MX_USART2_UART_Init(void);
void USART2_Write(const uint8_t *data, uint16_t length);
void USART2_WriteString(const char *text);

#endif
