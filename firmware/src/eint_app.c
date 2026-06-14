
/*
Modulo de interrupcion externa EINT0.
Gestiona el pulsador conectado a P2.10 para alternar entre arranque
y detencion del sistema.
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "eint_app.h"

#include <stdint.h>

extern void sistema_start(void);
extern void sistema_stop(void);

extern volatile uint8_t sistema_activo;

void EINT0_IRQHandler(void) {				
    if (sistema_activo == 0)
        sistema_start();
    else
        sistema_stop();
    EXTI_ClearFlag(EXTI_EINT0);
}

void confPin(void) {
    PINSEL_CFG_T confpin = {
        .port      = PORT_0,
        .pin       = PIN_0,
        .func      = 0,
        .mode      = 0,
        .openDrain = DISABLE
    };
    PINSEL_ConfigPin(&confpin);					
    GPIO_SetDir(PORT_0, 1 << 0, GPIO_OUTPUT);
    GPIO_ClearPins(PORT_0, 1 << 0);

    EXTI_Init();
    EXTI_PinConfig(EXTI_EINT0, EXTI_PULLUP);	
    EXTI_CFG_T conf_exti = {
        .line     = EXTI_EINT0,
        .mode     = EXTI_EDGE_SENSITIVE,
        .polarity = EXTI_FALLING_EDGE
    };
    EXTI_ConfigEnable(&conf_exti);
    NVIC_SetPriority(EINT0_IRQn, 2);
    NVIC_EnableIRQ(EINT0_IRQn);
}