#include "bh1750.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

static i2c_inst_t *bh1750_i2c = NULL; // Instância I2C usada (ex: i2c0)
static uint8_t bh1750_mode = BH1750_CONT_HRES; // Modo padrão

void bh1750_init(i2c_inst_t *i2c_instance, uint sda_pin, uint scl_pin) {
    bh1750_i2c = i2c_instance;

    // Inicializa o barramento I2C
    i2c_init(bh1750_i2c, 100 * 1000); // 100 kHz

    // Configura os pinos SDA e SCL
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    sleep_ms(100);

    // Liga o sensor
    bh1750_power_on();
}

void bh1750_power_on(void) {
    uint8_t cmd = BH1750_POWER_ON;
    i2c_write_blocking(bh1750_i2c, BH1750_ADDR, &cmd, 1, false);
    sleep_ms(10);
}

void bh1750_set_mode(uint8_t mode) {
    bh1750_mode = mode;
    i2c_write_blocking(bh1750_i2c, BH1750_ADDR, &mode, 1, false);
    sleep_ms(10);
}

uint16_t bh1750_read_raw(void) {
    uint8_t data[2];
    bh1750_set_mode(bh1750_mode);

    // Tempo de conversão típico depende do modo
    sleep_ms(180);

    // Lê 2 bytes de dados do sensor
    int ret = i2c_read_blocking(bh1750_i2c, BH1750_ADDR, data, 2, false);
    if (ret < 0) {
        printf("Erro na leitura do BH1750\n");
        return 0;
    }

    uint16_t raw = (data[0] << 8) | data[1];
    return raw;
}

float bh1750_read_lux(void) {
    uint16_t raw = bh1750_read_raw();
    if (raw == 0) return 0.0f;

    // Conversão para lux: valor / 1.2 (para modo HRES)
    return raw / 1.2f;
}
