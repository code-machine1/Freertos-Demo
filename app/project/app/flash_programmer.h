#ifndef FLASH_PROGRAMMER_H
#define FLASH_PROGRAMMER_H

#include <stdint.h>

#define FLASH_BIN_START_ADDR    0x00000000

typedef void (*flash_prog_write_callback_t)(uint32_t addr, uint32_t len);

void flash_programmer_init(void);
void flash_programmer_set_callbacks(flash_prog_write_callback_t on_start, flash_prog_write_callback_t on_done);
uint8_t flash_programmer_put_byte(uint8_t byte);
uint32_t flash_programmer_get_written(void);

#endif