#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Endereço I2C do BH1750 (padrão: 0x23)
#define BH1750_ADDR 0x23

// Comandos do sensor
#define BH1750_POWER_ON   0x01
#define BH1750_POWER_DOWN 0x00
#define BH1750_RESET      0x07

// Modos de operação
#define BH1750_CONT_HRES  0x10  // Alta resolução (1 lx)
#define BH1750_CONT_HRES2 0x11  // Alta resolução 2 (0.5 lx)
#define BH1750_CONT_LRES  0x13  // Baixa resolução (4 lx)
#define BH1750_ONETIME_HRES  0x20
#define BH1750_ONETIME_HRES2 0x21
#define BH1750_ONETIME_LRES  0x23

// Inicialização (passar ponteiro para o periférico I2C usado)
void bh1750_init(i2c_inst_t *i2c_instance, uint sda_pin, uint scl_pin);

// Liga o sensor
void bh1750_power_on(void);

// Configura o modo de medição
void bh1750_set_mode(uint8_t mode);

// Lê o valor bruto da medição
uint16_t bh1750_read_raw(void);

// Lê o valor convertido em lux
float bh1750_read_lux(void);

#endif
