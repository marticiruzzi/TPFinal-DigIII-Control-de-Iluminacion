#ifndef PWM_APP_H
#define PWM_APP_H

#include <stdint.h>

extern volatile uint8_t duty_pwm;
extern volatile uint8_t duty_pwm_pendiente;

void confTim1(void);
void PWM_SetDuty(uint8_t duty);
void PWM_AplicarDuty(uint8_t duty);

#endif