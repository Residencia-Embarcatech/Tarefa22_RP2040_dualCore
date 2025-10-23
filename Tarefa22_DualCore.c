#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/adc.h"

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void core1_interrupt_handler()
{
    while (multicore_fifo_rvalid())
    {
        uint32_t value = multicore_fifo_pop_blocking();
        printf("Hello World!! Core 2\nValor Recebido: %d\n\n", value);
    }

    multicore_fifo_clear_irq();
    
}

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
    multicore_launch_core1(core1_entry);
    int valor = 0;

    while (true) {
        printf("Hello, world! - Núcleo 1\n");
        multicore_fifo_push_blocking(valor++);

        sleep_ms(1000);
    }
}
