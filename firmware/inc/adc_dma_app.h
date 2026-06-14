#ifndef ADC_DMA_APP_H
#define ADC_DMA_APP_H

#include <stdint.h>

extern volatile uint16_t adc_promedio;

void configADC(void);
void configDMA(void);
void configTIM0(void);
void procesarBloqueDMA(void);

void DMA_IRQHandler(void);

#endif