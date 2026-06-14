/*
Modulo UART.
Recibe el porcentaje deseado y transmite el estado del sistema.
 */

#include "LPC17xx.h"
#include "lpc17xx_uart.h"
#include "uart_app.h"
#include "app_config.h"

#include <string.h>
#include <stdint.h>


extern volatile uint8_t sistema_activo;

extern volatile uint8_t enviar_estado;
extern volatile uint32_t contador_2_segundos;

extern volatile uint8_t porcentaje_deseado;
extern volatile uint8_t porcentaje_ambiente;
extern volatile int16_t error_luz;
extern volatile uint8_t duty_pwm;

extern volatile uint16_t adc_promedio;
extern volatile uint16_t lux_ambiente;
extern volatile uint16_t mv_ambiente;
extern volatile uint16_t lux_deseado;
extern volatile uint16_t mv_deseado;

extern uint16_t porcentaje_a_mv(uint8_t pct);

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t comando_listo = 0;

void configUART1(void) {
    UART_CFG_T uartCfg = {
        .baudRate = 115200,								
        .dataBits = UART_DBITS_8,
        .stopBits = UART_STOPBIT_1,
        .parity = UART_PARITY_NONE					
    };													
    UART_FIFO_CFG_T fifoCfg = {
        .level = UART_FIFO_TRGLEV0,				   
        .resetRxBuf = ENABLE,                       
        .resetTxBuf = ENABLE,                       
        .dmaMode = DISABLE                          
    };

    UART_PinConfig(UART_TX1_P0_15);
    UART_PinConfig(UART_RX1_P0_16);
    UART_Init(UART_PC, &uartCfg);                   
    UART_FIFOConfig(UART_PC, &fifoCfg);
    UART_TxEnable(UART_PC);
    UART_IntConfig(UART_PC, UART_INT_RBR, ENABLE);      
    UART_IntConfig(UART_PC, UART_INT_RLS, ENABLE);      
    NVIC_SetPriority(UART1_IRQn, 0);
    NVIC_EnableIRQ(UART1_IRQn);
}


void UART1_IRQHandler(void) {                           
    uint32_t intId = UART_GetIntId(UART_PC);			
    uint32_t tipo_int;
    uint8_t estado;
    uint8_t dato;
    uint8_t leidos = 0;


    if (intId & UART_IIR_INTSTAT_PEND) return;			

    tipo_int = intId & UART_IIR_INTID_MASK;            
    estado = UART_GetLineStatus(UART_PC);               

    if (tipo_int == UART_IIR_INTID_RLS) {				
        if (!comando_listo) {							
            rx_index = 0;
            rx_buffer[0] = '\0';
        }

        while ((leidos < RX_BUFFER_SIZE) && (UART_Receive(UART_PC, &dato, 1, NONE_BLOCKING) == 1)) {	
            leidos++;																		
        }
        return;
    }

    if (tipo_int == UART_IIR_INTID_RDA || tipo_int == UART_IIR_INTID_CTI) {		
        if ((estado & (UART_LINESTAT_OE | UART_LINESTAT_PE | UART_LINESTAT_FE | UART_LINESTAT_BI | UART_LINESTAT_RXFE)) && !comando_listo) { 
            rx_index = 0;
            rx_buffer[0] = '\0';
        }


        while ((leidos < RX_BUFFER_SIZE) && (UART_Receive(UART_PC, &dato, 1, NONE_BLOCKING) == 1)) {		
            leidos++;

            if (comando_listo) {
                continue;                               
            }

            if (dato == '\r' || dato == '\n') {		
                if (rx_index > 0) {                 
                    rx_buffer[rx_index] = '\0';
                    rx_index = 0;
                    comando_listo = 1;
                }
            } else if (dato == 8 || dato == 127) {	
                if (rx_index > 0) {					
                    rx_index--;
                    rx_buffer[rx_index] = '\0';
                }
            } else {
                if (dato < '0' || dato > '9') {		
                    continue;
                }

                if (rx_index < (RX_BUFFER_SIZE - 1)) {		
                    rx_buffer[rx_index++] = dato;
                    rx_buffer[rx_index] = '\0';
                } else {
                    rx_index     = 0;
                    rx_buffer[0] = '\0';            
                }
            }
        }
    }
}


void UART_SendString(const char *str) {				
    UART_Send(UART_PC, (uint8_t *)str, strlen(str), BLOCKING);      
}

void UART_SendUInt(uint32_t num) {					
    char buf[11];
    int8_t i = 0, j;
    if (num == 0) {
        UART_Send(UART_PC, (uint8_t *)"0", 1, BLOCKING);        
        return;
    }
    while (num > 0 && i < 10) {
        buf[i++] = (char)((num % 10) + '0');                
        num /= 10;
    }
    for (j=i-1; j>=0; j--)								
        UART_Send(UART_PC, (uint8_t *)&buf[j], 1, BLOCKING);
}

void UART_SendInt(int32_t num) {						
    if (num < 0) {
        UART_Send(UART_PC, (uint8_t *)"-", 1, BLOCKING);
        UART_SendUInt((uint32_t)(-num));
    } else {
        UART_SendUInt((uint32_t)num);
    }
}



void UART_ProcesarComando(void) {                  
    char comando_local[RX_BUFFER_SIZE];        
    uint8_t nuevo_valor;
    uint8_t i;

    NVIC_DisableIRQ(UART1_IRQn);				
    for (i=0; i< RX_BUFFER_SIZE; i++) {
        comando_local[i] = rx_buffer[i];
        if (rx_buffer[i] == '\0') break;        
    }
    comando_local[RX_BUFFER_SIZE - 1] = '\0';	
    rx_buffer[0] = '\0';                        
    rx_index = 0;
    comando_listo = 0;
    NVIC_EnableIRQ(UART1_IRQn);                

    enviar_estado = 0;							
    contador_2_segundos = 0;					

    nuevo_valor = convertirTextoAPorcentaje(comando_local);		

    if (nuevo_valor <= 100) {						
        porcentaje_deseado = nuevo_valor;
        mv_deseado = porcentaje_a_mv(porcentaje_deseado); 		
        lux_deseado = (uint16_t)(((uint32_t)mv_deseado * LUX_MAXIMO) / MV_MAXIMO);

        UART_SendString("\r\nOK: ");
        UART_SendUInt(porcentaje_deseado);
        UART_SendString(" %\r\n");
    } else {
        UART_SendString("\r\nERROR: Ingrese un valor entre 0 y 100\r\n");
    }
}

uint8_t convertirTextoAPorcentaje(volatile char *str) {		
    uint16_t valor = 0;
    uint8_t i = 0;
    if (str[0] == '\0') return 255;
    while (str[i] != '\0' && i < RX_BUFFER_SIZE) {
        if (str[i] < '0' || str[i] > '9') return 255;
        valor = valor * 10 +(uint16_t)(str[i] - '0');
        if (valor > 100) return 255;
        i++;
    }
    if (i >= RX_BUFFER_SIZE) return 255;
    return (uint8_t)valor;
}


void UART_EnviarEstado(void) {
    UART_SendString("\r\n======== ESTADO DEL SISTEMA ========\r\n");

    UART_SendString("Estado: ");
    UART_SendString(sistema_activo ? "ACTIVO\r\n": "DETENIDO\r\n");

    UART_SendString("Valor deseado en porcentaje: ");
    UART_SendUInt(porcentaje_deseado);
    UART_SendString("\r\n");

    UART_SendString("Valor deseado en Lux: ");
    UART_SendUInt(lux_deseado);
    UART_SendString("\r\n");

    UART_SendString("Luz ambiente medida  en porcentaje: ");
    UART_SendUInt(porcentaje_ambiente);
    UART_SendString("\r\n");

    UART_SendString("Luz ambiente medida en Lux: ");
    UART_SendUInt(lux_ambiente);
    UART_SendString("\r\n");

    UART_SendString("Error (des - amb): ");
    UART_SendInt(error_luz);
    UART_SendString("\r\n");

    UART_SendString("Valor de lux en mV: ");
    UART_SendUInt(mv_ambiente);
    UART_SendString("\r\n");
}