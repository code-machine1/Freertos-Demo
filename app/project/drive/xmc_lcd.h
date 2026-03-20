/**
  **************************************************************************
  * @file     xmc_lcd.h
  * @brief    xmc_lcd header file
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __XMC_LCD_H
#define __XMC_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f403a_407.h"

#include <stdio.h>

/** @addtogroup AT32F407_periph_examples
  * @{
  */

/** @addtogroup 407_XMC_lcd_8bit
  * @{
  */

/** @defgroup XMC_lcd_8bit_address_of_data_and_command
  * @{
  */

/* Addresses for writing commands and data (xmc_a0) */
#define  XMC_LCD_COMMAND                 0x6c000000
#define  XMC_LCD_DATA                    0x6c000001



/* DMA Configuration */
#define LCD_DMA_CHANNEL                  DMA1_CHANNEL1       //  π”√DMA1_CHANNEL1
#define LCD_DMA_IRQn                     DMA1_Channel1_IRQn
#define LCD_DMA_IRQHandler               DMA1_Channel1_IRQHandler

/**
  * @}
  */

/** @defgroup XMC_lcd_8bit_reset_line
  * @{
  */

/* LCD reset line control macros */
#define LCD_RESET_HIGH                   gpio_bits_set(GPIOA,GPIO_PINS_6)
#define LCD_RESET_LOW                    gpio_bits_reset(GPIOA,GPIO_PINS_6)

/**
  * @}
  */

/* Color definitions (RGB565 format) */
#define WHITE                            0xFFFF
#define BLACK                            0x0000
#define BLUE                             0x001F
#define BRED                             0xF81F
#define GRED                             0xFFE0
#define GBLUE                            0x07FF
#define RED                              0xF800
#define MAGENTA                          0xF81F
#define GREEN                            0x07E0
#define CYAN                             0x7FFF
#define YELLOW                           0xFFE0
#define BROWN                            0xBC40
#define BRRED                            0xFC07
#define GRAY                             0x8430

/* Display direction definitions */
typedef enum
{
    LCD_DIRECTION_PORTRAIT = 0,     // Portrait orientation (width < height)
    LCD_DIRECTION_LANDSCAPE = 1     // Landscape orientation (width > height)
} LCD_DirectionType;

/* LCD information structure */
typedef struct
{
    uint16_t width;                 // LCD screen width in pixels
    uint16_t height;                // LCD screen height in pixels
    uint16_t id;                    // LCD panel ID
    LCD_DirectionType direction;    // Current display orientation
    uint8_t scan_mode;              // Current scanning mode
} LCD_InfoType;

/* ASCII font size definitions */
typedef enum
{
    FONT_12X6,    // Corresponds to ascii_1206[][12]
    FONT_16X8,    // Corresponds to ascii_1608[][16]
    FONT_24X12,   // Corresponds to ascii_2412[][48]
    FONT_32X16,   // Corresponds to ascii_3216[][64]
    FONT_40X20    // Corresponds to ascii_4020[][120]
} AsciiFontSize;

/* LCD dimension definitions */
#define LCD_PORTRAIT_WIDTH   320
#define LCD_PORTRAIT_HEIGHT  480
#define LCD_LANDSCAPE_WIDTH  480
#define LCD_LANDSCAPE_HEIGHT 320

/* Display mode selection (choose one) */
#define LCD_DISPLAY_PORTRAIT             0   // Portrait mode (320x480)
#define LCD_DISPLAY_LANDSCAPE            1   // Landscape mode (480x320)

/* Scanning direction definitions (correspond to ILI9488 0x36 register settings) */
#define LCD_SCAN_LR_TB                   0x00  // Left to right, top to bottom
#define LCD_SCAN_LR_BT                   0x80  // Left to right, bottom to top (MY=1)
#define LCD_SCAN_RL_TB                   0x40  // Right to left, top to bottom (MX=1)
#define LCD_SCAN_RL_BT                   0xC0  // Right to left, bottom to top (MY=1, MX=1)
#define LCD_SCAN_TB_LR                   0x20  // Top to bottom, left to right (MV=1)
#define LCD_SCAN_TB_RL                   0x60  // Top to bottom, right to left (MV=1, MX=1)
#define LCD_SCAN_BT_LR                   0xA0  // Bottom to top, left to right (MV=1, MY=1)
#define LCD_SCAN_BT_RL                   0xE0  // Bottom to top, right to left (MV=1, MY=1, MX=1)

/* Common scanning modes (data format) */
#define LCD_SCAN_NORMAL                  LCD_SCAN_LR_TB      // Normal scanning
#define LCD_SCAN_REVERSE                 LCD_SCAN_RL_BT      // Full reverse scanning

/** @defgroup XMC_lcd_8bit_exported_functions
  * @{
  */

void xmc_init(void);
void lcd_init(void);
void lcd_wr_command(uint8_t command);
void lcd_wr_data(uint8_t data);
void lcd_set_block(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);
void lcd_write_one_point(uint16_t color);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_clear(uint16_t color);
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);
uint8_t lcd_show_ascii_char(uint16_t x, uint16_t y, char ch,
                            uint16_t color, uint16_t bg_color, AsciiFontSize font_size);
void lcd_show_ascii_string(uint16_t x, uint16_t y, const char *str,
                           uint16_t color, uint16_t bg_color, AsciiFontSize font_size);

uint16_t lcd_show_simple_number(uint16_t x, uint16_t y, int32_t num,
                                uint16_t color, uint16_t bg_color, AsciiFontSize font_size);

uint16_t lcd_show_number(uint16_t x, uint16_t y, int32_t num,
                         uint16_t color, uint16_t bg_color, AsciiFontSize font_size, uint8_t show_digits);
uint16_t lcd_show_simple_float(uint16_t x, uint16_t y, float fnum, 
                              uint16_t color, uint16_t bg_color, AsciiFontSize font_size);
uint16_t lcd_show_float(uint16_t x, uint16_t y, float fnum, 
                       uint16_t color, uint16_t bg_color, AsciiFontSize font_size, uint8_t decimal_digits);

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
void lcd_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_rounded_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t radius, uint16_t color);


void lcd_show_flash_img(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint8_t Pic_Num);
 
uint16_t lcd_read_id(void);

/* New function: Display image from internal array */
void lcd_show_internal_img(uint16_t x_start, uint16_t y_start, 
                          uint16_t img_width, uint16_t img_height, const uint8_t *img_data);
/* New function: Display image from internal array using DMA */
void lcd_show_internal_img_dma(uint16_t x_start, uint16_t y_start, 
                              uint16_t img_width, uint16_t img_height, const uint8_t *img_data);

 
 

/* Initialize display mode and scanning direction */
void lcd_set_display_mode(uint8_t display_mode, uint8_t scan_mode);

/* Get LCD information structure pointer */
LCD_InfoType *LCD_GetInfo(void);


/* use DMA write data */
void lcd_wr_data_dma(uint8_t *data, uint32_t length);     
void lcd_clear_dma(uint16_t color);                         
void lcd_fill_dma(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);  


/* DMA status */
extern volatile uint8_t lcd_dma_transfer_complete;


extern LCD_InfoType g_lcd_info;
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}

#endif

#endif