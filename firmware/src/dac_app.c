/*
Modulo DAC.
Configura la salida analogica AOUT en P0.26 y permite reflejar una
tension proporcional en mV a la medicion de iluminacion en Lux.
 */

#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "dac_app.h"

void configDAC(void) {
    DAC_Init();
    DAC_UpdateValue(0);
    DAC_CONVERTER_CFG_T conf_dac = {
        .doubleBuffer = DISABLE,
        .dmaCounter = DISABLE,
        .dmaRequest = DISABLE
    };
    DAC_ConfigDAConverterControl(&conf_dac);
}

void DAC_SetValue(uint16_t value) {
    if (value > 1023) {
        value = 1023;
    }
    DAC_UpdateValue(value);
}