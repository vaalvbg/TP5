#ifndef USART_CONFIG_H
#define USART_CONFIG_H

#include "stm32f1xx.h"

void USART1_Init(uint32_t baudrate);
bool USART1_read(char *caracter);
void USART1_write(uint8_t c);
bool USART1_Receive_String(char* buffer, uint32_t bufferSize);
void USART1_Write_String(const char* string);
#endif