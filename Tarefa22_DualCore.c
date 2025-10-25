#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "aht20.h"
#include "bh1750.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

// Estrutura para armazenar os dados do AHT20
AHT20_Data data;
float temperature = 0.0f;  
// Váriavel para leitura da luminosidade
uint16_t lux = 0;
//Variavel para a estrutura do display
ssd1306_t ssd;          


#define I2C_PORT i2c0               
#define I2C_SDA 0                   
#define I2C_SCL 1                   
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define endereco 0x3C

#define LUX 0
#define TEMP 1

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}
// Fim do trecho para modo BOOTSEL com botão B

/**
 * @brief Atualiza o painel (display ssd1306) com os dados de temperatura e luminosidade.
 * @param char* temp_str - String contendo a temperatura formatada.
 * @param char* lux_str - String contendo a luminosidade formatada.
 */
void update_display(char* temp_str, char* lux_str)
{
    ssd1306_fill(&ssd, false);                           // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);       // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, true);            // Desenha uma linha
    ssd1306_line(&ssd, 3, 37, 123, 37, true);            // Desenha uma linha
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);  // Desenha uma string
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);   // Desenha uma string
    ssd1306_line(&ssd, 63, 25, 63, 60, true);            // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, "Temp: ", 14, 41);             // Desenha uma string
    ssd1306_draw_string(&ssd, temp_str, 14, 52);             // Desenha uma string
    ssd1306_draw_string(&ssd, "Lux: ", 73, 41);             // Desenha uma string
    ssd1306_draw_string(&ssd, lux_str, 73, 52);            // Desenha uma string
    ssd1306_send_data(&ssd);                            // Atualiza o display

}

/**
 * @brief Handler da interrupção do core 1 para atualizar o display.
 */
void core1_interrupt_handler()
{
    while (multicore_fifo_rvalid())
    {
        uint32_t value = multicore_fifo_pop_blocking();
        if (value == true)
        {
            // Formata as strings para exibição
            char temp_str[16];
            char lux_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.2f", data.temperature);
            snprintf(lux_str, sizeof(lux_str), "%d", lux);

            // Atualiza o display com os novos valores
            printf("Core 1 > Atualizando painel da casa...\n");
            update_display(temp_str, lux_str);
        }
    }
    multicore_fifo_clear_irq();
    
}

/**
 * @brief Função do core 1 que aguarda por dados do core 0 para atualizar o display.
 */
void core1_entry()
{
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    while (true)
    {
        tight_loop_contents();
    }
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    // Inicializa o I2C para os sensores
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o BHT1750
    bh1750_init(I2C_PORT, I2C_SDA, I2C_SCL);
    
    // Inicializa o AHT20
    aht20_reset(I2C_PORT);
    aht20_init(I2C_PORT);

    //Inicializa o I2C para o display
    // I2C do Display funcionando em 400Khz.
    i2c_init(I2C_PORT_DISP, 400 * 1000);

    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);                    
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);                    
    gpio_pull_up(I2C_SDA_DISP);                                        
    gpio_pull_up(I2C_SCL_DISP);                                        
    //Inicializa o display ssd1306                                           
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT_DISP); 
    ssd1306_config(&ssd);                                              
    ssd1306_send_data(&ssd);                                           
    // Limpa o display.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicia o segundo núcleo
    multicore_launch_core1(core1_entry);
    
    //Variavel para controle de acesso à variáveis globais
    volatile bool can_access;

    while (true) {
        can_access = true;
        // Lê os dados do AHT20
         if (!aht20_read(I2C_PORT, &data))
         {
            can_access = false;
            printf("Core 0 > Erro na leitura do AHT20\n");
         }
        

        lux = bh1750_read_lux();
        printf("Core 0 > Temperatura: %.2f C, Luminosidade: %d lux\n\n", data.temperature, lux);

        multicore_fifo_push_blocking((uint32_t)can_access); // Envia luminosidade para o core 1
        sleep_ms(1000);
    }
}
