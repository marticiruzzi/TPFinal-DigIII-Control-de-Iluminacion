/*
Modulo principal de aplicacion.

Contiene la logica general del sistema de control de iluminacion:
arranque y detencion, conversiones entre porcentaje y magnitudes
medidas, actualizacion de estados globales y temporizacion periodica
mediante SysTick.
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"
#include "adc_dma_app.h"
#include "lpc17xx_gpdma.h"

#include "app.h"
#include "app_config.h"
#include "dac_app.h"
#include "pwm_app.h"
#include "uart_app.h"

#include <stdint.h>

volatile uint8_t  sistema_activo = 0;			

volatile uint8_t  porcentaje_deseado = 50;
volatile uint8_t  porcentaje_ambiente = 0;
volatile int16_t  error_luz = 0;	

volatile uint16_t lux_ambiente = 0;
volatile uint16_t mv_ambiente = 0;
volatile uint16_t lux_deseado = 0;
volatile uint16_t mv_deseado = 0;

volatile uint8_t  enviar_estado = 0;
volatile uint8_t  aviso_arranque = 0;			
volatile uint32_t contador_2_segundos = 0;


uint8_t mv_a_porcentaje(uint16_t mv) {
    if (mv <= 200) return 0;

    if (mv <= 425) return 1;

    if (mv <= 1087) return (uint8_t)(1 + (((mv - 425) * 19) / 662));

    if (mv <= 1436) return (uint8_t)(20 + (((mv - 1087) * 10) / 349));

    if (mv <= 2063) return (uint8_t)(30 + (((mv - 1436) * 20) / 627));

    if (mv <= 2718) return (uint8_t)(50 + (((mv - 2063) * 25) / 655));

    if (mv <= 2884) return (uint8_t)(75 + (((mv - 2718) * 10) / 166));

    if (mv <= 3133) return (uint8_t)(85 + (((mv - 2884) * 15) / 249));

    return 100; 
}

uint16_t porcentaje_a_mv(uint8_t pct) {
    if (pct == 0) return 0;
    if (pct <= 1) return (uint16_t)((pct * 425) / 1);
    if (pct <= 20) return (uint16_t)(425 + (((pct - 1) * 662) / 19));
    if (pct <= 30) return (uint16_t)(1087 + (((pct - 20) * 349) / 10));
    if (pct <= 50) return (uint16_t)(1436 + (((pct - 30) * 627) / 20));
    if (pct <= 75) return (uint16_t)(2063 + (((pct - 50) * 655) / 25));
    if (pct <= 85) return (uint16_t)(2718 + (((pct - 75) * 166) / 10));
    if (pct <= 100) return (uint16_t)(2884 + (((pct - 85) * 249) / 15));
    return 3133;
}


void SysTick_Handler(void) {
    contador_2_segundos++;
    if (contador_2_segundos >= 2000) {
        contador_2_segundos = 0;
        enviar_estado = 1;
    }
}



void sistema_start(void) {					
    sistema_activo = 1;
    aviso_arranque = 1;

    adc_promedio = 0;
    porcentaje_ambiente = 0;
    error_luz = 0;
    duty_pwm = 0;
    duty_pwm_pendiente = 0;
    lux_ambiente = 0;
    mv_ambiente = 0;

    DAC_SetValue(0);
    PWM_AplicarDuty(0);
    GPIO_ClearPins(PORT_0, 1 << 0);
    configDMA();
    GPDMA_ChannelStart(GPDMA_CH_0);
    TIM_Enable(LPC_TIM1);
    TIM_Enable(LPC_TIM0);
}

void sistema_stop(void) {						
    sistema_activo = 0;
    aviso_arranque = 2;

    TIM_Disable(LPC_TIM0);
    TIM_Disable(LPC_TIM1);
    GPDMA_ChannelStop(GPDMA_CH_0);

    GPIO_ClearPins(PORT_0, 1 << 0);
    DAC_SetValue(0);
    duty_pwm = 0;
    duty_pwm_pendiente = 0;
    PWM_AplicarDuty(0);
    lux_ambiente = 0;
    mv_ambiente = 0;
}




