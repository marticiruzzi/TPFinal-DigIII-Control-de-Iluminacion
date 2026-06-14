#ifndef UART_APP_H
#define UART_APP_H

#include <stdint.h>
#include "app_config.h"

extern volatile char rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t rx_index;
extern volatile uint8_t comando_listo;

void configUART1(void);

void UART_SendString(const char *str);
void UART_SendUInt(uint32_t num);
void UART_SendInt(int32_t num);

void UART_ProcesarComando(void);
void UART_EnviarEstado(void);

uint8_t convertirTextoAPorcentaje(volatile char *str);

#endif