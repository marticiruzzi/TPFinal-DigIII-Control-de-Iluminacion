#ifndef APP_H
#define APP_H

#include <stdint.h>

extern volatile uint8_t sistema_activo;

extern volatile uint8_t porcentaje_deseado;
extern volatile uint8_t porcentaje_ambiente;
extern volatile int16_t error_luz;

extern volatile uint16_t lux_ambiente;
extern volatile uint16_t mv_ambiente;
extern volatile uint16_t lux_deseado;
extern volatile uint16_t mv_deseado;

extern volatile uint8_t enviar_estado;
extern volatile uint8_t aviso_arranque;
extern volatile uint32_t contador_2_segundos;

void confPin(void);

void sistema_start(void);
void sistema_stop(void);

uint8_t mv_a_porcentaje(uint16_t mv);
uint16_t porcentaje_a_mv(uint8_t pct);

void SysTick_Handler(void);

#endif