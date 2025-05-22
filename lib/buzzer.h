#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

void buzzer_init(int pin);
void tocar_frequencia(int frequencia, int duracao_ms);
void buzzer_desliga(int pin);

#endif