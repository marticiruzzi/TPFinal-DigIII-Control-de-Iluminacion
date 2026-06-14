/*
 Modulo PWM.
 Genera la señal de control hacia el gate del MOSFET mediante Timer1.
 */

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "pwm_app.h"
#include "app_config.h"
extern volatile uint8_t sistema_activo;

volatile uint8_t duty_pwm = 0;
volatile uint8_t duty_pwm_pendiente = 0;

void confTim1(void) {						
    TIM_TIMERCFG_T conf = {
        .prescaleOpt = TIM_US,
        .prescaleValue = 1
    };

    TIM_MATCHCFG_T match0 = {				
        .channel = TIM_MATCH_0,
        .intEn = ENABLE,
        .stopEn = DISABLE,
        .resetEn = ENABLE,
        .extOpt = TIM_NOTHING,
        .matchValue = TIM1_PERIODO			
    };

    TIM_MATCHCFG_T match1 = {				
        .channel = TIM_MATCH_1,
        .intEn = ENABLE,
        .stopEn = DISABLE,
        .resetEn = DISABLE,
        .extOpt = TIM_NOTHING,
        .matchValue = 1						
    };

    TIM_InitTimer(LPC_TIM1, &conf);
    TIM_ConfigMatch(LPC_TIM1, &match0);
    TIM_ConfigMatch(LPC_TIM1, &match1);
    NVIC_SetPriority(TIMER1_IRQn, 3);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

void PWM_SetDuty(uint8_t duty) {
    if (duty > 100) duty = 100;
    duty_pwm_pendiente = duty;					
}

void PWM_AplicarDuty(uint8_t duty) {			
    uint32_t ticks;
    if (duty == 0) {											
        TIM_UpdateMatchValue(LPC_TIM1, TIM_MATCH_1, 1);
    } else if (duty >= 100) {									
        TIM_UpdateMatchValue(LPC_TIM1, TIM_MATCH_1, TIM1_PERIODO + 1);          
    } else {
        ticks = ((uint32_t)(TIM1_PERIODO + 1) * duty) / 100;
        if (ticks == 0)            ticks = 1;					
        if (ticks > TIM1_PERIODO)  ticks = TIM1_PERIODO;		
        TIM_UpdateMatchValue(LPC_TIM1, TIM_MATCH_1, ticks);
    }
}

void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT) == SET) {
        if (duty_pwm != duty_pwm_pendiente) {					
            duty_pwm = duty_pwm_pendiente;                      
            PWM_AplicarDuty(duty_pwm);                          
        }

        if (sistema_activo && duty_pwm > 0)
            GPIO_SetPins(PORT_0, 1 << 0);						
        else
            GPIO_ClearPins(PORT_0, 1 << 0);						
        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }

    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR1_INT) == SET) {
        if (duty_pwm < 100)
            GPIO_ClearPins(PORT_0, 1 << 0);						
        TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
    }
}