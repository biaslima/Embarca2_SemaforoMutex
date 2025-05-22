#ifndef PERIFERICOS_H
#define PERIFERICOS_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

void buzzer_init(int pin);
void tocar_frequencia(int frequencia, int duracao_ms);
void buzzer_desliga(int pin);
void atualizaLedRGB(uint LED_VERMELHO, uint LED_VERDE, uint LED_AZUL, uint16_t usuariosAtivos, uint maxUsuarios);

#endif