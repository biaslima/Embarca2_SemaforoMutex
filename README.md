# Painel de Controle Interativo com Acesso Concorrente

Este projeto simula um sistema de controle de entrada e saída em um laboratorio com capacidade limitada, utilizando FreeRTOS e a placa BitDogLab com RP2040.

## 🧠 Funcionalidades

- Controle de acesso com semáforo de contagem
- Reset do sistema via interrupção e semáforo binário
- Proteção do display com mutex
- Feedback visual com display OLED
- LED RGB indicando ocupação atual
- Sinalização sonora com buzzer
- Interface via botões físicos

## ⚙️ Componentes utilizados

- BitDogLab com RP2040
- Display OLED via I2C (SSD1306)
- LED RGB (3 GPIOs)
- Buzzer (PWM)
- Botões A, B e JOYSTICK (GPIO)

## 📦 Estrutura do código

- `LabGateControl.c`: lógica principal e multitarefas
- `perifericos.c/h`: funções auxiliares (buzzer, LED RGB)
- `ssd1306.c/h`: biblioteca do display
- `FreeRTOS`: gerenciamento das tarefas

## 👩‍💻 Autora

Anna Beatriz Silva Lima — [Residência EmbarcaTech 2025]
