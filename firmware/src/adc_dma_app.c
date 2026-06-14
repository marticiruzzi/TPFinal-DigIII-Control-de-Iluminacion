/*
Modulo ADC + DMA + Timer0.
Configura el muestreo periodico del sensor de luz y procesa bloques de muestras.
 */

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "adc_dma_app.h"
#include "app_config.h"
#include "dac_app.h"
#include "pwm_app.h"

#include <stdint.h>

extern volatile uint8_t sistema_activo;

extern volatile uint8_t porcentaje_deseado;
extern volatile uint8_t porcentaje_ambiente;
extern volatile int16_t error_luz;

extern volatile uint16_t lux_ambiente;
extern volatile uint16_t mv_ambiente;

extern volatile uint8_t enviar_estado;

extern uint8_t mv_a_porcentaje(uint16_t mv);

volatile uint16_t adc_promedio = 0;


void configADC(void) {
    ADC_Init(200000);								
    ADC_PinConfig(ADC_CHANNEL_0);					
    ADC_BurstDisable();
    ADC_ChannelEnable(ADC_CHANNEL_0);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_StartCmd(ADC_START_ON_MAT01);
}



void configDMA(void) {
    static GPDMA_LLI_T lli_adc= {                       
        .srcAddr = (uint32_t)&LPC_ADC->ADDR0,
        .dstAddr = BUFFER_ADC_ADDR,
        .nextLLI = (uint32_t)&lli_adc,
        .control = (BUFFER_TAM & 0xFFF) | (2 << 18) | (2 << 21) | (0 << 26) | (1 << 27) | (1 << 31)
    };

    GPDMA_Channel_CFG_T dma_adc = {
        .channelNum = GPDMA_CH_0,
        .transferSize = BUFFER_TAM,
        .type = GPDMA_P2M,
        .srcMemAddr = (uint32_t)&LPC_ADC->ADDR0,
        .dstMemAddr = BUFFER_ADC_ADDR,
        .srcConn = GPDMA_ADC,
        .dstConn  = 0,
        .src = {
            .width = GPDMA_WORD,
            .burst = GPDMA_BSIZE_1,
            .increment = DISABLE
        },
        .dst = {
            .width = GPDMA_WORD,
            .burst = GPDMA_BSIZE_1,
            .increment = ENABLE
        },
        .intTC = ENABLE,
        .intErr = DISABLE,
        .linkedList = (uint32_t)&lli_adc
    };

    GPDMA_Init();
    GPDMA_SetupChannel(&dma_adc);
    NVIC_SetPriority(DMA_IRQn, 1);
    NVIC_EnableIRQ(DMA_IRQn);
}



void configTIM0(void) {						
    TIM_TIMERCFG_T tim0 = {
        .prescaleOpt   = TIM_US,
        .prescaleValue = 1					
    };
    TIM_MATCHCFG_T mat01 = {
        .channel = TIM_MATCH_1,
        .intEn = DISABLE,
        .stopEn = DISABLE,
        .resetEn = ENABLE,
        .extOpt = TIM_TOGGLE,
        .matchValue = 500 - 1				
    };
    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_PinConfig(TIM_MAT0_1_P1_29);
}



void DMA_IRQHandler(void) {
    if (LPC_GPDMA->DMACIntTCStat & (1 << 0)) {					
        LPC_GPDMA->DMACIntTCClear = (1 << 0);					

        if (sistema_activo) {									
            procesarBloqueDMA();
        }
    }
}

void procesarBloqueDMA(void) {                                  
    volatile uint32_t *buf = (volatile uint32_t *)BUFFER_ADC_ADDR;		
    uint32_t suma = 0;
    uint32_t i;
    uint32_t dac_valor;
    uint8_t nuevo_duty;                                                   

    for (i=0; i< BUFFER_TAM; i++) {
        suma += (buf[i] >> 4) & 0xFFF;									
    }
    adc_promedio = (uint16_t)(suma / BUFFER_TAM);						

    mv_ambiente = (uint16_t)(((uint32_t)adc_promedio * 3300) / 4095);      

    lux_ambiente = (uint16_t)(((uint32_t)mv_ambiente * LUX_MAXIMO) / MV_MAXIMO);

    porcentaje_ambiente = mv_a_porcentaje(mv_ambiente);

    error_luz = (int16_t)porcentaje_deseado - (int16_t)porcentaje_ambiente;

    if (error_luz <= 0) {											
        nuevo_duty = 0;
    } else if ((porcentaje_deseado > ERROR_ZONA_MUERTA) &&			
               (error_luz <= ERROR_ZONA_MUERTA)) {
        nuevo_duty = 0;
    } else if (error_luz >= 100) {									
        nuevo_duty = 100;											
    } else {														
        nuevo_duty = (uint8_t)error_luz;
    }

    if (nuevo_duty != duty_pwm_pendiente) {							
        PWM_SetDuty(nuevo_duty);
    }

    // DAC: refleja directamente los lux medidos
    dac_valor = ((uint32_t)lux_ambiente * 1023) / 3300;
    if (dac_valor > 1023) dac_valor = 1023;
    DAC_SetValue((uint16_t)dac_valor);
}


