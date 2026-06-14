/*
Sistema Automatico de Control de Iluminacion de un Ambiente

Archivo principal del firmware.
Inicializa los modulos del sistema y ejecuta el lazo principal,
donde se procesan comandos UART y se reporta el estado.
 */

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_uart.h"
#include "dac_app.h"
#include "uart_app.h"
#include "pwm_app.h"
#include "eint_app.h"
#include "app_config.h"
#include "adc_dma_app.h"
#include "app.h"
#include <string.h>                
#include <stdint.h>              


int main(void) {
    SystemCoreClockUpdate();						
    SysTick_Config(SystemCoreClock / 1000);			

    confPin();
    configDAC();
    configADC();
    configDMA();
    configTIM0();
    confTim1();
    configUART1();

    mv_deseado = porcentaje_a_mv(porcentaje_deseado); 								

    lux_deseado = (uint16_t)(((uint32_t)mv_deseado * LUX_MAXIMO) / MV_MAXIMO);		

    UART_SendString("=== Sistema de Control de Iluminacion ===\r\n");
    UART_SendString("Presione EINT0 para arrancar el sistema\r\n");
    UART_SendString("Ingrese porcentaje deseado (0-100) via UART:\r\n");

    while (1) {														
        if (comando_listo) {
            UART_ProcesarComando();
            continue;                                          
        }

        if (aviso_arranque == 1) {
            aviso_arranque = 0;
            UART_SendString("\r\n[SISTEMA ARRANCADO]\r\n");
        } else if (aviso_arranque == 2) {
            aviso_arranque = 0;
            UART_SendString("\r\n[SISTEMA DETENIDO]\r\n");
        }

        if (enviar_estado) {
            enviar_estado = 0;
            UART_EnviarEstado();
        }
    }
    return 0;
}

