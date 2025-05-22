#include "perifericos.h"

static int buzzer_pin;
static uint slice_num;
static uint channel;

//Incia o buzzer com PWM
void buzzer_init(int pin) {
    buzzer_pin = pin;

    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    channel = pwm_gpio_to_channel(buzzer_pin);
    
    pwm_set_clkdiv(slice_num, 125.0);  
    pwm_set_wrap(slice_num, 1000);     
    pwm_set_chan_level(slice_num, channel, 500); 
    
    printf("Buzzer inicializado no pino %d (PWM slice %d, canal %d)\n", 
           pin, slice_num, channel);
}

void tocar_frequencia(int frequencia, int duracao_ms) {
    // Fórmula: wrap = 1_000_000 / frequência
    uint32_t wrap = 1000000 / frequencia;
    
    printf("Tocando frequência %d Hz (wrap=%d)\n", frequencia, wrap);
    
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, channel, wrap / 2);
    pwm_set_enabled(slice_num, true);
    
    sleep_ms(duracao_ms);
    
    pwm_set_enabled(slice_num, false);
}

//Desliga o buzzer
void buzzer_desliga(int pin){
    pwm_set_enabled(slice_num, false);
    gpio_put(pin, 0);
}

void atualizaLedRGB(uint LED_VERMELHO, uint LED_VERDE, uint LED_AZUL, uint16_t usuariosAtivos, uint maxUsuarios) {
    gpio_put(LED_VERMELHO, 0);
    gpio_put(LED_VERDE, 0);
    gpio_put(LED_AZUL, 0);

    if (usuariosAtivos == 0) {
        gpio_put(LED_AZUL, 1); 
    } else if (usuariosAtivos < maxUsuarios - 1) {
        gpio_put(LED_VERDE, 1); 
    } else if (usuariosAtivos == maxUsuarios - 1) {
        gpio_put(LED_VERMELHO, 1);
        gpio_put(LED_VERDE, 1); 
    } else if (usuariosAtivos == maxUsuarios) {
        gpio_put(LED_VERMELHO, 1); 
    }
}
