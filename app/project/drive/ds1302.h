#ifndef __DS1302_H__
#define __DS1302_H__

#include "at32f403a_407.h"

#define DS_RST_PORT GPIOC
#define DS_RST_PIN GPIO_PINS_13

#define DS_IO_PORT GPIOC
#define DS_IO_PIN GPIO_PINS_14

#define DS_CLK_PORT GPIOC
#define DS_CLK_PIN GPIO_PINS_15

#define DS_RST_H gpio_bits_set(DS_RST_PORT, DS_RST_PIN)
#define DS_RST_L gpio_bits_reset(DS_RST_PORT, DS_RST_PIN)

#define DS_CLK_H gpio_bits_set(DS_CLK_PORT, DS_CLK_PIN)
#define DS_CLK_L gpio_bits_reset(DS_CLK_PORT, DS_CLK_PIN)

#define DS_IO_H gpio_bits_set(DS_IO_PORT, DS_IO_PIN)
#define DS_IO_L gpio_bits_reset(DS_IO_PORT, DS_IO_PIN)
#define DS_GET_IO_DATA gpio_input_data_bit_read(DS_IO_PORT, DS_IO_PIN)

#define CHANGE_PORT_MODE(gpio_x, pins, mode)                         \
    do                                                               \
    {                                                                \
        gpio_init_type gpio_initstructure = {                        \
            .gpio_pins = (pins),                                     \
            .gpio_mode = (mode),                                     \
            .gpio_out_type = GPIO_OUTPUT_OPEN_DRAIN,                 \
            .gpio_pull = GPIO_PULL_NONE,                             \
            .gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE};    \
                                                                     \
        if ((mode) == GPIO_MODE_INPUT || (mode) == GPIO_MODE_OUTPUT) \
        {                                                            \
            gpio_init((gpio_x), &gpio_initstructure);                \
        }                                                            \
    } while (0)

#define DS1302_SECOND_REG 0x80
#define DS1302_MINUTE_REG 0x82
#define DS1302_HOUR_REG 0x84
#define DS1302_DAY_REG 0x86
#define DS1302_MONTH_REG 0x88
#define DS1302_WEEK_REG 0x8A
#define DS1302_YEAR_REG 0x8C
#define DS1302_READ_REG 0x01
#define DS1302_WRITE_REG 0x00
#define DS1302_CONTROL_REG 0x8E

uint8_t DS1302_Read_Byte(uint8_t reg_addr);

void DS1302_Write_Byte(uint8_t reg_addr, uint8_t data);

#endif
