#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/bootrom.h"
#include "stdio.h"
#include "lib/perifericos.h" //Biblioteca para os periféricos usados

//Definições de display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
ssd1306_t ssd;

//Definições de botões
#define BOTAO_A 5
#define BOTAO_B 6
#define BOTAO_JOYSTICK 22

//Definições de LEDs e Buzzer
#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 11
#define BUZZER 21

//Variáveis globais
#define maxUsuarios 10
uint16_t usuariosAtivos = 0;

//Buffer para conversão de string
char bufferUsuarios[32];
char bufferVagas[32];

//Semaforos
SemaphoreHandle_t xSemaphoreUsuarios;
SemaphoreHandle_t xDisplayMutex;
SemaphoreHandle_t xSemaphoreReset;

//Função que cria uma tela HOME
void exibirTelaPadrao() {
    
    sprintf(bufferUsuarios, "Usuarios: %d", usuariosAtivos);
    sprintf(bufferVagas, "Vagas: %d", (maxUsuarios - usuariosAtivos));

    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
        ssd1306_fill(&ssd, 0);
        ssd1306_draw_string(&ssd, "Laboratorio", 20, 15);
        ssd1306_draw_string(&ssd, "EmbarcaTech", 20, 25);
        ssd1306_draw_string(&ssd, bufferUsuarios, 5, 44);
        ssd1306_draw_string(&ssd, bufferVagas, 5, 52);
        ssd1306_send_data(&ssd);
        xSemaphoreGive(xDisplayMutex);
    }
}

// Tarefa de ocupação do laboratório
void vEntradaTask(void *params) {
    while (true) {
        if (gpio_get(BOTAO_A) == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (gpio_get(BOTAO_A) == 0) {
                if (xSemaphoreTake(xSemaphoreUsuarios, 0) == pdTRUE) {
                    usuariosAtivos++;
                    atualizaLedRGB(LED_VERMELHO, LED_VERDE, LED_AZUL, usuariosAtivos, maxUsuarios);

                    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                        ssd1306_fill(&ssd, 0);
                        ssd1306_draw_string(&ssd, "Entrada", 36, 25);
                        ssd1306_draw_string(&ssd, "OK!", 50, 34);
                        ssd1306_send_data(&ssd);
                        vTaskDelay(pdMS_TO_TICKS(500));
                        xSemaphoreGive(xDisplayMutex);
                    }

                    exibirTelaPadrao();
                } else {
                    tocar_frequencia(800, 300);
                    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                        ssd1306_fill(&ssd, 0);
                        ssd1306_draw_string(&ssd, "Lab. Cheio!", 20, 25);
                        ssd1306_send_data(&ssd);
                        xSemaphoreGive(xDisplayMutex);

                        vTaskDelay(pdMS_TO_TICKS(1500));
                        exibirTelaPadrao();
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa de desocupação do laboratorio
void vSaidaTask(void *params) {
    while (true) {
        if (gpio_get(BOTAO_B) == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (gpio_get(BOTAO_B) == 0) {
                if (usuariosAtivos > 0) {
                    usuariosAtivos--;
                    xSemaphoreGive(xSemaphoreUsuarios);
                    atualizaLedRGB(LED_VERMELHO, LED_VERDE, LED_AZUL, usuariosAtivos, maxUsuarios);

                    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                        ssd1306_fill(&ssd, 0);
                        ssd1306_draw_string(&ssd, "Saida", 44, 25 );
                        ssd1306_draw_string(&ssd, "OK!", 50, 34);
                        ssd1306_send_data(&ssd);
                        vTaskDelay(pdMS_TO_TICKS(500));
                        xSemaphoreGive(xDisplayMutex);
                    }

                    exibirTelaPadrao();
                } else {
                    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                        ssd1306_fill(&ssd, 0);
                        ssd1306_draw_string(&ssd, "Lab. Vazio!", 20, 25);
                        ssd1306_send_data(&ssd);
                        xSemaphoreGive(xDisplayMutex);
                        vTaskDelay(pdMS_TO_TICKS(1500));
                        exibirTelaPadrao();
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa de reset
void vResetTask(void *params) {
    while (true) {
        if (xSemaphoreTake(xSemaphoreReset, portMAX_DELAY) == pdTRUE) {
            while (xSemaphoreTake(xSemaphoreReset, pdMS_TO_TICKS(50)) == pdTRUE) {
            }
            
            while (usuariosAtivos > 0) {
                xSemaphoreGive(xSemaphoreUsuarios);
                usuariosAtivos--;
            }

            atualizaLedRGB(LED_VERMELHO, LED_VERDE, LED_AZUL, usuariosAtivos, maxUsuarios);

            tocar_frequencia(1200, 300);
            vTaskDelay(pdMS_TO_TICKS(150));
            tocar_frequencia(1200, 300);

            if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                ssd1306_fill(&ssd, 0);
                ssd1306_draw_string(&ssd, "Sistema", 36, 20);
                ssd1306_draw_string(&ssd, "resetado", 32, 30);
                ssd1306_send_data(&ssd);
                xSemaphoreGive(xDisplayMutex);
            }
            

            vTaskDelay(pdMS_TO_TICKS(1500));
            exibirTelaPadrao();
            
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

// Interrupção do botão joystick
void gpio_irq_handler(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio == BOTAO_JOYSTICK) {
        xSemaphoreGiveFromISR(xSemaphoreReset, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// Setup
int main() {
    //Inicialização de componentes
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    buzzer_init(BUZZER);

    gpio_init(BOTAO_A); gpio_set_dir(BOTAO_A, GPIO_IN); gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B); gpio_set_dir(BOTAO_B, GPIO_IN); gpio_pull_up(BOTAO_B);
    gpio_init(BOTAO_JOYSTICK); gpio_set_dir(BOTAO_JOYSTICK, GPIO_IN); gpio_pull_up(BOTAO_JOYSTICK);

    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE); gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AZUL); gpio_set_dir(LED_AZUL, GPIO_OUT);

    //Definição de interrupção do botão Z
    gpio_set_irq_enabled_with_callback(BOTAO_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Criação dos semáforos e mutex
    xSemaphoreUsuarios = xSemaphoreCreateCounting(maxUsuarios, maxUsuarios);
    xDisplayMutex = xSemaphoreCreateMutex();
    xSemaphoreReset = xSemaphoreCreateBinary();

    xTaskCreate(vEntradaTask, "EntradaTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vSaidaTask, "SaidaTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vResetTask, "ResetTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    //Exibição inicial do sistema
    exibirTelaPadrao();
    atualizaLedRGB(LED_VERMELHO, LED_VERDE, LED_AZUL, usuariosAtivos, maxUsuarios);

    vTaskStartScheduler();
    panic_unsupported();
}

