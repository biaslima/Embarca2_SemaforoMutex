#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/bootrom.h"
#include "stdio.h"
#include "lib/buzzer.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
ssd1306_t ssd;

#define BOTAO_A 5 // Entrada de usuário
#define BOTAO_B 6 // Saída de usuário
#define BOTAO_JOYSTICK 22 //Reset button

#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 11

#define BUZZER 21  

#define maxUsuarios 10
uint16_t usuariosAtivos = 0; 

SemaphoreHandle_t xSemaphoreUsuarios;
SemaphoreHandle_t xDisplayMutex;


// ISR do botão A (incrementa o semáforo de contagem)
void gpio_callback(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Tarefa que registra entrada
void vEntradaTask(void *params)
{
    char bufferUsuarios[32];
    char bufferVagas[32];

    while (true)
    {
        if(gpio_get(BOTAO_A) == 0){ //vERIFICA SE O a FOI PRESSIONADO
            if (xSemaphoreTake(xSemaphoreUsuarios, 0) == pdTRUE)
            {
                usuariosAtivos ++; 
                // Atualiza display com a nova contagem
                ssd1306_fill(&ssd, 0);
                sprintf(bufferUsuarios, "Usuarios: %d", usuariosAtivos);
                sprintf(bufferUsuarios, "Vagas: %d", (10 - usuariosAtivos));
                ssd1306_draw_string(&ssd, "Entrada", 36, 10);
                ssd1306_draw_string(&ssd, "OK!", 48, 19);
                ssd1306_draw_string(&ssd, bufferUsuarios, 5, 44);
                ssd1306_draw_string(&ssd, bufferVagas, 5, 52);
                ssd1306_send_data(&ssd);

            } else {
                // Sistema cheio – emitir beep e exibir aviso
                tocar_frequencia(800, 300); // duração em ms
                ssd1306_fill(&ssd, 0);
                ssd1306_draw_string(&ssd, "Sistema Cheio!", 5, 25);
                ssd1306_send_data(&ssd);
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
         vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void vSaidaTask(void *params)
{
    char bufferUsuarios[32];
    char bufferVagas[32];

    while (true)
    {
        if(gpio_get(BOTAO_B) == 0){ //vERIFICA SE O a FOI PRESSIONADO
            if (usuariosAtivos > 0)
            {
                xSemaphoreGive(xSemaphoreUsuarios);
                usuariosAtivos --; 

                    // Atualiza display com a nova contagem
                    ssd1306_fill(&ssd, 0);
                    sprintf(bufferUsuarios, "Usuarios: %d", usuariosAtivos);
                    sprintf(bufferUsuarios, "Vagas: %d", (10 - usuariosAtivos));
                    ssd1306_draw_string(&ssd, "Saida", 36, 10);
                    ssd1306_draw_string(&ssd, "OK!", 48, 19);
                    ssd1306_draw_string(&ssd, bufferUsuarios, 5, 44);
                    ssd1306_draw_string(&ssd, bufferVagas, 5, 52);
                    ssd1306_send_data(&ssd);
                } else {
                    ssd1306_fill(&ssd, 0);
                    ssd1306_draw_string(&ssd, "Sala Vazia!", 20, 25);
                    ssd1306_send_data(&ssd);
                }
                vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    } 
}

// ISR para BOOTSEL e botão de evento
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_JOYSTICK)
    {
        reset_usb_boot(0, 0);
    }
    else if (gpio == BOTAO_A)
    {
        gpio_callback(gpio, events);
    }
}

int main()
{
    stdio_init_all();

    // Inicialização do display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    buzzer_init(BUZZER);
    
    // Configura os botões
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(BOTAO_JOYSTICK);
    gpio_set_dir(BOTAO_JOYSTICK, GPIO_IN);
    gpio_pull_up(BOTAO_JOYSTICK);

    //Configura LEDs
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);


    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);

    

    // Cria semáforo de contagem (máximo 10, inicial 0)
    xSemaphoreUsuarios = xSemaphoreCreateCounting(maxUsuarios, maxUsuarios);
    xDisplayMutex = xSemaphoreCreateMutex();

    // Cria tarefa
    xTaskCreate(vEntradaTask, "Contagem de Entrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vSaidaTask, "Contagem de Saída", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
