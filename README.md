# Painel de Casa Inteligente com Dual-Core 🏠

Este projeto foi desenvolvido para a placa **BitDogLab**, baseada no microcontrolador **RP2040**, e tem como objetivo demonstrar a utilização dos dois núcleos de processamento (Core 0 e Core 1) do MCU. O sistema simula um painel de "casa inteligente", exibindo dados em tempo real de temperatura, obtidos pelo sensor **AHT20**, e de luminosidade, pelo sensor **BH1750**, em um **Display OLED SSD1306**.

---

## 📌 Sobre o Projeto

O Painel de Casa Inteligente foi desenvolvido como projeto prático da residência tecnológica **EmbarcaTech**.

A proposta é integrar sensores e um display, dividindo as responsabilidades entre os dois núcleos do RP2040 para garantir paralelismo e eficiência. O **Núcleo 0** fica dedicado à aquisição contínua dos dados dos sensores, enquanto o **Núcleo 1** se responsabiliza exclusivamente pela atualização da interface do usuário (display), sem que uma tarefa interfira na outra.

---

## 🧠 Como funciona

O sistema divide as tarefas de forma que cada núcleo execute funções críticas de forma independente[cite: 19]. A comunicação entre eles é feita via FIFO, utilizando interrupções.

### 🔵 Núcleo 0 (Core 0): Coleta de Dados

* Este núcleo é inicializado pela função `main` e é responsável por inicializar todos os periféricos: os barramentos I2C, os sensores (AHT20, BH1750) e o display (SSD1306).
* Ele também inicializa o Núcleo 1, instruindo-o a executar a função `core1_entry`.
* Em seu loop principal (`while(true)`), o Núcleo 0 realiza a leitura contínua dos sensores, armazena os valores de temperatura e luminosidade em variáveis globais (`data.temperature` e `lux`) e envia um sinal (flag) para o Núcleo 1.

### 🔄 Comunicação Inter-Core (FIFO)

* Para notificar o Núcleo 1 de que novos dados estão prontos, o Núcleo 0 envia uma flag boolean (`can_access`) para a fila (FIFO) do multicore usando `multicore_fifo_push_blocking`.
* Essa ação dispara uma interrupção de SIO (`SIO_IRQ_PROC1`) no Núcleo 1.

### 🟢 Núcleo 1 (Core 1): Interface do Usuário

* Ao ser inicializado (`core1_entry`), o Núcleo 1 configura a função `core1_interrupt_handler` como sua rotina de tratamento de interrupção para a fila do multicore e entra em um loop de espera (`tight_loop_contents`).
* Quando a interrupção é recebida (ou seja, quando o Núcleo 0 envia a flag), a ISR `core1_interrupt_handler` é executada.
* Esta função lê os dados das variáveis globais, formata os valores de temperatura e luminosidade em strings e chama a função `update_display` para atualizar as informações no painel OLED.

---

## 🛠️ Hardware Utilizado

| Componente | Conexão | Função |
| --- | --- | --- |
| BitDogLab (RP2040) | - | Placa principal |
| Sensor AHT20 | I2C0 (SDA: 0, SCL: 1) | Medição de Temperatura |
| Sensor BH1750 | I2C0 (SDA: 0, SCL: 1) | Medição de Luminosidade (Lux) |
| Display OLED SSD1306 | I2C1 (SDA: 14, SCL: 15) | Exibição do painel |

---

## 📁 Utilização

Atendendo aos requisitos de organização da residência, o arquivo CMakeLists.txt está configurado para facilitar a importação do projeto no Visual Studio Code.

Segue as instruções:

1.  Na barra lateral, clique em **Raspberry Pi Pico Project** e depois em **Import Project**.

    ![image](https://github.com/user-attachments/assets/4b1ed8c7-6730-4bfe-ae1f-8a26017d1140)

2.  Selecione o diretório do projeto e clique em **Import** (utilizando a versão **2.1.1** do Pico SDK).

    ![image](https://github.com/user-attachments/assets/be706372-b918-4ade-847e-12706af0cc99)

3.  Agora, basta **compilar** e **rodar** o projeto, com a placa **BitDogLab** conectada.
