/**
  **************************************************************************
  * @file     xmc_lcd.c
  * @brief    xmc_lcd configuration program
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

#include "at32f403a_407.h"
#include "xmc_lcd.h"
#include "picture.h"
#include "lcdfont.h"
#include "at32_spiflash.h"
#include "perf_counter.h"
/** @addtogroup AT32F407_periph_examples
  * @{
  */

/** @addtogroup 407_XMC_lcd_8bit
  * @{
  */

/* Global LCD information structure */
LCD_InfoType g_lcd_info =
{
    .width = LCD_PORTRAIT_WIDTH,
    .height = LCD_PORTRAIT_HEIGHT,
    .id = 0,
    .direction = LCD_DIRECTION_PORTRAIT,
    .scan_mode = LCD_SCAN_NORMAL
};

/******************************************************************************
 * Function Description: Get LCD information structure pointer
 * Input Parameters: None
 * Return Value: Pointer to LCD information structure
 ******************************************************************************/
LCD_InfoType *LCD_GetInfo(void)
{
    return &g_lcd_info;
}

/******************************************************************************
 * Function Description: Update LCD information
 * Input Parameters: display_mode - Display mode
 *                   scan_mode - Scanning direction
 * Return Value: None
 ******************************************************************************/
static void lcd_update_info(uint8_t display_mode, uint8_t scan_mode)
{
    g_lcd_info.scan_mode = scan_mode;

    if (display_mode == LCD_DISPLAY_LANDSCAPE)
    {
        g_lcd_info.direction = LCD_DIRECTION_LANDSCAPE;
        g_lcd_info.width = LCD_LANDSCAPE_WIDTH;
        g_lcd_info.height = LCD_LANDSCAPE_HEIGHT;
    }
    else
    {
        g_lcd_info.direction = LCD_DIRECTION_PORTRAIT;
        g_lcd_info.width = LCD_PORTRAIT_WIDTH;
        g_lcd_info.height = LCD_PORTRAIT_HEIGHT;
    }
}

/**
  * @brief  Set LCD display mode and scanning direction
  * @param  display_mode: Display mode (LCD_DISPLAY_PORTRAIT or LCD_DISPLAY_LANDSCAPE)
  * @param  scan_mode: Scanning direction (one of LCD_SCAN_xxx)
  * @retval none
  * @note   This function should be called after lcd_init(), determines screen orientation
  */
void lcd_set_display_mode(uint8_t display_mode, uint8_t scan_mode)
{
    uint8_t madctl_value;
    /* The input value is scanning direction, add BGR bit (0x08) */
    madctl_value = scan_mode | 0x08;  // 0x08 for BGR order (common for ILI9488)
    /* Configure the memory data access control register (0x36) */
    lcd_wr_command(0x36);
    lcd_wr_data(madctl_value);

    /* Set display area and GRAM range */
    if (display_mode == LCD_DISPLAY_LANDSCAPE)
    {
        /* Landscape mode (480x320) */
        lcd_wr_command(0x2A);  // Column address set (X range)
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x01);
        lcd_wr_data(0xDF);     // 480-1=479=0x1DF
        lcd_wr_command(0x2B);  // Page address set (Y range)
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x01);
        lcd_wr_data(0x3F);     // 320-1=319=0x13F
    }
    else
    {
        /* Portrait mode (320x480) - default */
        lcd_wr_command(0x2A);  // Column address set (X range)
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x01);
        lcd_wr_data(0x3F);     // 320-1=319=0x13F
        lcd_wr_command(0x2B);  // Page address set (Y range)
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x01);
        lcd_wr_data(0xDF);     // 480-1=479=0x1DF
    }

    /* Update LCD information */
    lcd_update_info(display_mode, scan_mode);
}

/**
  * @brief  Write command to LCD
  * @param  command : Command to be written
  * @retval none
  */
void lcd_wr_command(uint8_t command)
{
    *(__IO uint8_t *) XMC_LCD_COMMAND = command;
}

/**
  * @brief  Write data to LCD
  * @param  data : Data to be written
  * @retval none
  */
void lcd_wr_data(uint8_t data)
{
    *(__IO uint8_t *) XMC_LCD_DATA = data;
}


/**
 * @brief  使用DMA1通道1传输数据到LCD
 * @param  data_buf: 待传输的数据缓冲区
 * @param  data_len: 传输数据长度（字节数，0~65535）
 * @retval 无
 */


/**
  * @brief  Read LCD panel ID
  * @param  none
  * @retval LCD panel ID
  */
uint16_t lcd_read_id(void)
{
    uint16_t id = 0;
    /* Read ID via command 0xDA */
    *(__IO uint8_t *) XMC_LCD_COMMAND = 0xDA;
    id = *(__IO uint8_t *) XMC_LCD_DATA;
    id = (id << 8) | (*(__IO uint8_t *) XMC_LCD_DATA);
    return id;
}

/**
  * @brief  Set row and column coordinates for LCD operation
  * @param  xstart : Starting row coordinate
  * @param  ystart : Starting column coordinate
  * @param  xend : Ending row coordinate
  * @param  yend : Ending column coordinate
  * @retval none
  */
void lcd_set_block(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend)
{
    /* Set row coordinates (0x2A command) */
    lcd_wr_command(0x2a);
    lcd_wr_data(xstart >> 8);
    lcd_wr_data(xstart);
    lcd_wr_data(xend >> 8);
    lcd_wr_data(xend);
    /* Set column coordinates (0x2B command) */
    lcd_wr_command(0x2b);
    lcd_wr_data(ystart >> 8);
    lcd_wr_data(ystart);
    lcd_wr_data(yend >> 8);
    lcd_wr_data(yend);
    /* Enable memory write (0x2C command) */
    lcd_wr_command(0x2c);
}

/**
  * @brief  Write one pixel data to LCD
  * @param  color : Pixel color in RGB565 format
  * @retval none
  */
void lcd_write_one_point(uint16_t color)
{
    lcd_wr_data(color >> 8);  // High byte
    lcd_wr_data(color);       // Low byte
}


 

/**
  * @brief  Draw a single point on LCD
  * @param  x : X coordinate of the point
  * @param  y : Y coordinate of the point
  * @param  color : Color of the point in RGB565 format
  * @retval none
  */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_block(x, y, x, y);  // Set block to single pixel
    lcd_write_one_point(color);
}

/**
  * @brief  Clear entire LCD screen with specified color
  * @param  color : Fill color in RGB565 format
  * @retval none
  */
void lcd_clear(uint16_t color)
{
    uint32_t i;
    LCD_InfoType *lcd_info;
    /* Get LCD information */
    lcd_info = LCD_GetInfo();
    /* Calculate total pixels */
    uint32_t total_pixels = (uint32_t)lcd_info->width * lcd_info->height;
    /* Set block to entire screen */
    lcd_set_block(0, 0, lcd_info->width - 1, lcd_info->height - 1);

    /* Write color data to all pixels */
    for (i = 0; i < total_pixels; i++)
    {
        lcd_write_one_point(color);
    }
}

/**
  * @brief  Fill a specified rectangular area with color
  * @param  x1: Top-left X coordinate
  * @param  y1: Top-left Y coordinate
  * @param  x2: Bottom-right X coordinate
  * @param  y2: Bottom-right Y coordinate
  * @param  color: Fill color (RGB565 format)
  * @retval none
  */
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t i, pixel_count;
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    /* Get screen boundary information */
    LCD_InfoType *lcd_info = LCD_GetInfo();

    if (x1 >= lcd_info->width || y1 >= lcd_info->height)
    {
        return;  /* Start coordinates out of screen range */
    }

    if (x2 >= lcd_info->width)
    {
        x2 = lcd_info->width - 1;  /* Limit X to maximum screen width */
    }

    if (y2 >= lcd_info->height)
    {
        y2 = lcd_info->height - 1;  /* Limit Y to maximum screen height */
    }

    /* Ensure x1 <= x2, y1 <= y2 */
    if (x1 > x2)
    {
        uint16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if (y1 > y2)
    {
        uint16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    /* Calculate total pixels in the rectangle */
    pixel_count = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);
    /* Set block to the target rectangle */
    lcd_set_block(x1, y1, x2, y2);

    /* Write color data to all pixels in the block */
    for (i = 0; i < pixel_count; i++)
    {
        lcd_wr_data(color_high);  /* Write high 8 bits */
        lcd_wr_data(color_low);   /* Write low 8 bits */
    }
}

/**
  * @brief  Draw a line on LCD
  * @param  x_start: Start X coordinate
  * @param  y_start: Start Y coordinate
  * @param  x_end: End X coordinate
  * @param  y_end: End Y coordinate
  * @param  color: Line color (RGB565 format)
  * @retval none
  * @note   Uses Bresenham's algorithm, supports all line angles
  */
void lcd_draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
    int16_t dx, dy;
    int16_t x, y;
    int16_t step_x, step_y;
    int16_t error, error2;

    /* Calculate differences and step directions for x and y */
    if (x_end > x_start)
    {
        dx = x_end - x_start;
        step_x = 1;
    }
    else
    {
        dx = x_start - x_end;
        step_x = -1;
    }

    if (y_end > y_start)
    {
        dy = y_end - y_start;
        step_y = 1;
    }
    else
    {
        dy = y_start - y_end;
        step_y = -1;
    }

    /* Initialize Bresenham's algorithm */
    x = x_start;
    y = y_start;

    if (dx > dy)
    {
        /* Slope < 1, use x as reference */
        error = dx >> 1;  /* dx/2 */

        while (x != x_end)
        {
            lcd_draw_point(x, y, color);
            error -= dy;

            if (error < 0)
            {
                y += step_y;
                error += dx;
            }

            x += step_x;
        }

        lcd_draw_point(x, y, color);  /* Draw end point */
    }
    else
    {
        /* Slope >= 1, use y as reference */
        error = dy >> 1;  /* dy/2 */

        while (y != y_end)
        {
            lcd_draw_point(x, y, color);
            error -= dx;

            if (error < 0)
            {
                x += step_x;
                error += dy;
            }

            y += step_y;
        }

        lcd_draw_point(x, y, color);  /* Draw end point */
    }
}

/**
  * @brief  Display an ASCII character on LCD
  * @param  x: X coordinate of top-left corner
  * @param  y: Y coordinate of top-left corner
  * @param  ch: ASCII character to display (0x20-0x7E)
  * @param  color: Character color
  * @param  bg_color: Background color
  * @param  font_size: Font size (AsciiFontSize enum)
  * @retval Character width in pixels, 0 if failed
  */
uint8_t lcd_show_ascii_char(uint16_t x, uint16_t y, char ch,
                            uint16_t color, uint16_t bg_color, AsciiFontSize font_size)
{
    const unsigned char *char_bitmap = NULL;
    uint8_t font_col = 0, font_row = 0;
    uint8_t bytes_per_col = 0;
    uint16_t font_offset = 0;
    uint8_t show_width = 0, show_height = 0;

    switch (font_size)
    {
    case FONT_12X6:
        if (ch < 0x20 || ch > 0x7E)  /* Only support printable ASCII */
        {
            return 0;
        }

        font_offset = ch - 0x20;  /* Calculate offset in font array */
        char_bitmap = ascii_1206[font_offset];
        font_col = 12;
        font_row = 6;
        bytes_per_col = 1;
        show_width = font_row;
        show_height = font_col;
        break;

    case FONT_16X8:
        font_offset = ch - 0x20;
        char_bitmap = ascii_1608[font_offset];
        font_col = 16;
        font_row = 8;
        bytes_per_col = 1;
        show_width = 8;
        show_height = 16;
        break;

    case FONT_24X12:
        font_offset = ch - 0x20;
        char_bitmap = ascii_2412[font_offset];
        font_col = 24;
        font_row = 12;
        bytes_per_col = 2;
        show_width = 12;
        show_height = 24;
        break;

    case FONT_32X16:
        font_offset = ch - 0x20;
        char_bitmap = ascii_3216[font_offset];
        font_col = 32;
        font_row = 16;
        bytes_per_col = 2;
        show_width = 16;
        show_height = 32;
        break;

    case FONT_40X20:
        font_offset = ch - 0x20;
        char_bitmap = ascii_4020[font_offset];
        font_col = 40;
        font_row = 20;
        bytes_per_col = 3;
        show_width = 20;
        show_height = 40;
        break;

    default:
        return 0;
    }

    LCD_InfoType *lcd_info = LCD_GetInfo();

    /* Check if character is within screen bounds */
    if (x + show_width > lcd_info->width || y + show_height > lcd_info->height)
    {
        return 0;
    }

    lcd_set_block(x, y, x + show_width - 1, y + show_height - 1);

    for (uint8_t show_r = 0; show_r < show_height; show_r++)
    {
        uint8_t orig_c = show_r;  /* Original column index */

        for (uint8_t show_c = 0; show_c < show_width; show_c++)
        {
            uint8_t orig_r = show_c;  /* Original row index */
            const unsigned char *col_data = &char_bitmap[orig_c * bytes_per_col];
            uint8_t byte_idx = orig_r / 8;  /* Which byte contains the target bit */
            uint8_t bit_pos = orig_r % 8;   /* Bit position in the byte */
            uint8_t bit_val = (col_data[byte_idx] >> bit_pos) & 0x01;  /* Extract bit value */
            /* Determine pixel color: bit_val=1 for character color, 0 for background */
            uint16_t pixel_color = (bit_val == 1) ? color : bg_color;
            lcd_wr_data(pixel_color >> 8);
            lcd_wr_data(pixel_color & 0xFF);
        }
    }

    return show_width;
}

/**
  * @brief  Display an ASCII string on LCD
  * @param  x: X coordinate of top-left corner
  * @param  y: Y coordinate of top-left corner
  * @param  str: Pointer to null-terminated string
  * @param  color: Character color
  * @param  bg_color: Background color
  * @param  font_size: Font size (AsciiFontSize enum)
  * @retval none
  */
void lcd_show_ascii_string(uint16_t x, uint16_t y, const char *str,
                           uint16_t color, uint16_t bg_color, AsciiFontSize font_size)
{
    uint16_t curr_x = x;

    while (*str != '\0')  /* Loop until end of string */
    {
        curr_x += lcd_show_ascii_char(curr_x, y, *str, color, bg_color, font_size);
        str++;
    }
}

/**
 * @brief  Display integer numbers (supports positive/negative numbers, zeros)
 * @param  x: X coordinate of top-left corner
 * @param  y: Y coordinate of top-left corner
 * @param  num: Number to display (range: -999999 ~ 999999)
 * @param  color: Character color (e.g., BLACK)
 * @param  bg_color: Background color (e.g., WHITE)
 * @param  font_size: Font size (FONT_12X6/FONT_16X8, etc.)
 * @param  show_digits: Number of digits to display (0=auto, >0=fixed digits with leading zeros)
 * @retval Total display width in pixels
 * @note   Uses ASCII character display with matching color/background/font settings
 */
uint16_t lcd_show_number(uint16_t x, uint16_t y, int32_t num,
                         uint16_t color, uint16_t bg_color, AsciiFontSize font_size, uint8_t show_digits)
{
    uint8_t digit_buf[10] = {0}; // Stores each digit (max 9 digits + sign)
    uint8_t digit_cnt = 0;       // Actual number of digits
    uint16_t curr_x = x;         // Current display X coordinate
    uint8_t is_negative = 0;     // Whether the number is negative
    uint8_t char_width = 0;      // Width of each character (matches font size)

    // 1. Get character width based on font size
    switch (font_size)
    {
    case FONT_12X6:
        char_width = 6;
        break;  // 12X6 font translates to 6px width

    case FONT_16X8:
        char_width = 8;
        break;  // 16X8 font translates to 8px width

    case FONT_24X12:
        char_width = 12;
        break;  // 24X12 font translates to 12px width

    case FONT_32X16:
        char_width = 16;
        break;  // 32X16 font translates to 16px width

    case FONT_40X20:
        char_width = 20;
        break;  // 40X20 font translates to 20px width

    default:
        return 0;
    }

    // 2. Handle negative numbers
    if (num < 0)
    {
        is_negative = 1;
        num = -num; // Convert to positive for processing
    }

    // 3. Handle zero specifically
    if (num == 0)
    {
        digit_buf[digit_cnt++] = '0';
    }

    // 4. Extract each digit (stores 3,2,1 for 123)
    while (num > 0 && digit_cnt < sizeof(digit_buf))
    {
        digit_buf[digit_cnt++] = '0' + (num % 10);
        num /= 10;
    }

    // 5. Add leading zeros for fixed digit display (e.g., 5 → 005 when show_digits=3)
    if (show_digits > 0 && digit_cnt < show_digits)
    {
        uint8_t zero_cnt = show_digits - digit_cnt;

        for (uint8_t i = digit_cnt; i < show_digits; i++)
        {
            digit_buf[i] = '0';
        }

        digit_cnt = show_digits;
    }

    // 6. Display negative sign if needed
    if (is_negative)
    {
        curr_x += lcd_show_ascii_char(curr_x, y, '-', color, bg_color, font_size);
    }

    // 7. Display digits in reverse order (3,2,1 → 1,2,3)
    for (int8_t i = digit_cnt - 1; i >= 0; i--)
    {
        curr_x += lcd_show_ascii_char(curr_x, y, digit_buf[i], color, bg_color, font_size);
    }

    // 8. Return total display width
    return curr_x - x;
}

/**
 * @brief  Simplified number display (auto-digit, no leading zeros)
 * @param  x/y: Coordinates
 * @param  num: Number to display
 * @param  color/bg_color: Colors
 * @param  font_size: Font size
 * @retval Display width
 */
uint16_t lcd_show_simple_number(uint16_t x, uint16_t y, int32_t num,
                                uint16_t color, uint16_t bg_color, AsciiFontSize font_size)
{
    // Call with show_digits=0 for auto-digit display
    return lcd_show_number(x, y, num, color, bg_color, font_size, 0);
}

/**
 * @brief  Display floating-point numbers with configurable decimal places
 * @param  x/y: Coordinates of top-left corner
 * @param  fnum: Floating-point number to display
 * @param  color/bg_color: Character and background colors
 * @param  font_size: Font size
 * @param  decimal_digits: Number of decimal places to display
 * @retval Total display width in pixels
 */
uint16_t lcd_show_float(uint16_t x, uint16_t y, float fnum,
                        uint16_t color, uint16_t bg_color, AsciiFontSize font_size, uint8_t decimal_digits)
{
    uint16_t curr_x = x;
    int32_t int_part = 0;    // Integer part of the number
    uint32_t decimal_part = 0; // Decimal part (converted to integer)
    uint8_t is_negative = 0; // Whether the number is negative
    uint8_t char_width = 0;  // Character width matching font size

    // 1. Get character width based on font size
    switch (font_size)
    {
    case FONT_12X6:
        char_width = 6;
        break;

    case FONT_16X8:
        char_width = 8;
        break;

    case FONT_24X12:
        char_width = 12;
        break;

    case FONT_32X16:
        char_width = 16;
        break;

    case FONT_40X20:
        char_width = 20;
        break;

    default:
        return 0;
    }

    // 2. Handle negative numbers
    if (fnum < 0)
    {
        is_negative = 1;
        fnum = -fnum; // Convert to positive for processing
    }

    // 3. Separate integer and decimal parts
    int_part = (int32_t)fnum; // Get integer part (e.g., 3 from 3.14)
    float decimal_f = fnum - int_part; // Get decimal part (e.g., 0.14 from 3.14)

    // 4. Process decimal part: convert to integer (e.g., 0.14 → 14 for 2 decimal places)
    if (decimal_digits > 0)
    {
        // Calculate multiplier (10^decimal_digits, e.g., 100 for 2 decimal places)
        uint32_t decimal_mult = 1;

        for (uint8_t i = 0; i < decimal_digits; i++)
        {
            decimal_mult *= 10;
        }

        // Convert to integer with rounding (e.g., 0.199999 → 0.2 for 1 decimal place)
        decimal_part = (uint32_t)(decimal_f * decimal_mult + 0.5);

        // Handle carry-over (e.g., 9.99 with 1 decimal place → 10.0)
        if (decimal_part >= decimal_mult)
        {
            int_part += 1;
            decimal_part = 0;
        }
    }

    // 5. Display negative sign if needed
    if (is_negative)
    {
        curr_x += lcd_show_ascii_char(curr_x, y, '-', color, bg_color, font_size);
    }

    // 6. Display integer part (auto-digit)
    curr_x += lcd_show_number(curr_x, y, int_part, color, bg_color, font_size, 0);

    // 7. Display decimal point and decimal part (if needed)
    if (decimal_digits > 0)
    {
        // Display decimal point
        curr_x += lcd_show_ascii_char(curr_x, y, '.', color, bg_color, font_size);
        // Display decimal part with fixed digits
        curr_x += lcd_show_number(curr_x, y, decimal_part, color, bg_color, font_size, decimal_digits);
    }

    // 8. Return total display width
    return curr_x - x;
}

/**
 * @brief  Simplified float display (fixed 2 decimal places)
 * @param  x/y: Coordinates
 * @param  fnum: Floating-point number
 * @param  color/bg_color: Colors
 * @param  font_size: Font size
 * @retval Display width
 */
uint16_t lcd_show_simple_float(uint16_t x, uint16_t y, float fnum,
                               uint16_t color, uint16_t bg_color, AsciiFontSize font_size)
{
    // Fixed 2 decimal places via general function
    return lcd_show_float(x, y, fnum, color, bg_color, font_size, 2);
}


/**
  * @brief  绘制矩形边框
  * @param  x1: 左上角X坐标
  * @param  y1: 左上角Y坐标
  * @param  x2: 右下角X坐标
  * @param  y2: 右下角Y坐标
  * @param  color: 线条颜色
  * @retval none
  */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();

    // 边界检查与坐标调整
    if (x1 >= lcd_info->width || y1 >= lcd_info->height ||
            x2 >= lcd_info->width || y2 >= lcd_info->height)
    {
        return;
    }

    if (x1 > x2)
    {
        uint16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if (y1 > y2)
    {
        uint16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    // 绘制四条边
    lcd_draw_line(x1, y1, x2, y1, color);  // 上边
    lcd_draw_line(x1, y2, x2, y2, color);  // 下边
    lcd_draw_line(x1, y1, x1, y2, color);  // 左边
    lcd_draw_line(x2, y1, x2, y2, color);  // 右边
}

/**
  * @brief  绘制圆形
  * @param  x0: 圆心X坐标
  * @param  y0: 圆心Y坐标
  * @param  radius: 圆半径
  * @param  color: 线条颜色
  * @retval none
  */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();
    int16_t x, y;
    int16_t dx, dy;
    int16_t err;
    int16_t s;

    // 边界检查（确保圆完全在屏幕内）
    if (x0 + radius >= lcd_info->width || x0 - radius < 0 ||
            y0 + radius >= lcd_info->height || y0 - radius < 0)
    {
        return;
    }

    x = 0;
    y = radius;
    err = 3 - 2 * radius;  // 中点圆算法初始误差

    while (x <= y)
    {
        // 绘制8个对称点
        lcd_draw_point(x0 + x, y0 + y, color);
        lcd_draw_point(x0 - x, y0 + y, color);
        lcd_draw_point(x0 + x, y0 - y, color);
        lcd_draw_point(x0 - x, y0 - y, color);
        lcd_draw_point(x0 + y, y0 + x, color);
        lcd_draw_point(x0 - y, y0 + x, color);
        lcd_draw_point(x0 + y, y0 - x, color);
        lcd_draw_point(x0 - y, y0 - x, color);
        x++;

        if (err > 0)
        {
            y--;
            err += 4 * (x - y) + 10;
        }
        else
        {
            err += 4 * x + 6;
        }
    }
}


/**
  * @brief  填充矩形区域
  * @param  x1: 左上角X坐标
  * @param  y1: 左上角Y坐标
  * @param  x2: 右下角X坐标
  * @param  y2: 右下角Y坐标
  * @param  color: 填充颜色
  * @retval none
  */



void lcd_fill_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();

    // 坐标调整（确保x1<=x2, y1<=y2）
    if (x1 > x2)
    {
        uint16_t t = x1;
        x1 = x2;
        x2 = t;
    }

    if (y1 > y2)
    {
        uint16_t t = y1;
        y1 = y2;
        y2 = t;
    }

    // 边界裁剪
    if (x1 >= lcd_info->width || y1 >= lcd_info->height || x2 < 0 || y2 < 0)
    {
        return;
    }

    if (x2 >= lcd_info->width)
    {
        x2 = lcd_info->width - 1;
    }

    if (y2 >= lcd_info->height)
    {
        y2 = lcd_info->height - 1;
    }

    // 调用现有填充函数
    lcd_fill(x1, y1, x2, y2, color);
}



/**
  * @brief  绘制圆角矩形边框
  * @param  x1: 左上角X坐标
  * @param  y1: 左上角Y坐标
  * @param  x2: 右下角X坐标
  * @param  y2: 右下角Y坐标
  * @param  radius: 圆角半径
  * @param  color: 线条颜色
  * @retval none
  */


void lcd_draw_rounded_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t radius, uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();

    // 坐标调整与边界检查
    if (x1 > x2)
    {
        uint16_t t = x1;
        x1 = x2;
        x2 = t;
    }

    if (y1 > y2)
    {
        uint16_t t = y1;
        y1 = y2;
        y2 = t;
    }

    if (x1 >= lcd_info->width || y1 >= lcd_info->height || x2 < 0 || y2 < 0)
    {
        return;
    }

    // 限制半径不超过矩形边长的一半
    uint16_t max_radius = (x2 - x1) > (y2 - y1) ? (y2 - y1) / 2 : (x2 - x1) / 2;

    if (radius > max_radius)
    {
        radius = max_radius;
    }

    // 绘制四条边（避开圆角部分）
    lcd_draw_line(x1 + radius, y1, x2 - radius, y1, color);          // 上边缘
    lcd_draw_line(x1 + radius, y2, x2 - radius, y2, color);          // 下边缘
    lcd_draw_line(x1, y1 + radius, x1, y2 - radius, color);          // 左边缘
    lcd_draw_line(x2, y1 + radius, x2, y2 - radius, color);          // 右边缘
    // 绘制四个圆角（利用圆形函数绘制1/4圆弧）
    lcd_draw_circle(x1 + radius, y1 + radius, radius, color);        // 左上圆角
    lcd_draw_circle(x2 - radius, y1 + radius, radius, color);        // 右上圆角
    lcd_draw_circle(x1 + radius, y2 - radius, radius, color);        // 左下圆角
    lcd_draw_circle(x2 - radius, y2 - radius, radius, color);        // 右下圆角
}

/**
 * @brief  Display image from external SPI Flash to LCD
 * @param  x_start: LCD display start X coordinate
 * @param  y_start: LCD display start Y coordinate
 * @param  img_width: Image width in pixels (original, not offset)
 * @param  img_height: Image height in pixels (original, not offset)
 * @param  pic_index: Image index in pic_add array (0-53)
 * @retval None
 * @note   1. Image format: RGB565 (2 bytes per pixel)
 *         2. BLOCK_SIZE = 4096 (match SPI Flash erase block size for efficiency)
 *         3. Read_data uses static to reduce stack usage (single-task safe)
 */
// Image address table in external Flash (54 images total, index 0-53)
const uint32_t g_flash_pic_addr[54] = {
    0x00000000, 0x0004B000, 0x00096000, 0x000E1000, 0x0012C000, 0x00177000,
    0x001C2000, 0x0020D000, 0x00258000, 0x002A3000, 0x002EE000, 0x00339000,
    0x00384000, 0x003CF000, 0x0041A000, 0x00465000, 0x004B0000, 0x004FB000,
    0x00546000, 0x00591000, 0x005DC000, 0x00627000, 0x00672000, 0x006BD000,
    0x00708000, 0x00753000, 0x0079E000, 0x007E9000, 0x00834000, 0x0087F000,
    0x008CA000, 0x00915000, 0x00960000, 0x009AB000, 0x009F6000, 0x00A41000,
    0x00A8C000, 0x00AD7000, 0x00B22000, 0x00B6D000, 0x00BB8000, 0x00C03000,
    0x00C4E000, 0x00C99000, 0x00CE4000, 0x00D2F000, 0x00D7A000, 0x00DC5000,
    0x00E10000, 0x00E5B000, 0x00EA6000, 0x00EF1000, 0x00F3C000, 0x00F87000
};

#define FLASH_READ_BLOCK_SIZE 4096  // Align with SPI Flash block size (4KB typical)
#define MAX_PIC_INDEX        53     // Max valid image index (0-53)

void lcd_show_flash_img(uint16_t x_start, uint16_t y_start, 
                        uint16_t img_width, uint16_t img_height, uint8_t pic_index)
{
    // Step 1: Parameter validity check (prevent out-of-bounds)
    // 1.1 Check image index range
    if (pic_index > MAX_PIC_INDEX)
    {
        return; // Invalid index, exit directly
    }
    
    // 1.2 Get LCD screen info and check display area
    LCD_InfoType *p_lcd_info = LCD_GetInfo();
    uint16_t x_end = x_start + img_width - 1;
    uint16_t y_end = y_start + img_height - 1;
    
    if (x_start >= p_lcd_info->width || y_start >= p_lcd_info->height ||
        x_end >= p_lcd_info->width || y_end >= p_lcd_info->height)
    {
        return; // Display area out of LCD screen
    }

    // Step 2: Calculate image data size and Flash address
    uint32_t total_data_len = (uint32_t)img_width * img_height * 2; // RGB565: 2 bytes/pixel
    uint32_t flash_pic_addr = g_flash_pic_addr[pic_index];
    static uint8_t flash_read_buf[FLASH_READ_BLOCK_SIZE] = {0}; // Static to save stack
    
    // Step 3: Set LCD display window (avoid redefining original width/height)
    lcd_set_block(x_start, y_start, x_end, y_end);

    // Step 4: Read data from Flash and write to LCD (unified logic for all block sizes)
    while (total_data_len > 0)
    {
        // Determine current read length (min of remaining data and block size)
        uint32_t curr_read_len = (total_data_len > FLASH_READ_BLOCK_SIZE) ? 
                                 FLASH_READ_BLOCK_SIZE : total_data_len;
        
        // Read data from external Flash
        spiflash_read(flash_read_buf, flash_pic_addr, curr_read_len);
        
        // Write data to LCD (byte by byte, match lcd_wr_data interface)
        for (uint32_t i = 0; i < curr_read_len; i++)
        {
            lcd_wr_data(flash_read_buf[i]);
        }

        // Update address and remaining length
        flash_pic_addr += curr_read_len;
        total_data_len -= curr_read_len;
    }
}

/**
 * @brief  Display image from internal array (RAM or Flash) to LCD
 * @param  x_start: LCD display start X coordinate
 * @param  y_start: LCD display start Y coordinate
 * @param  img_width: Image width in pixels
 * @param  img_height: Image height in pixels
 * @param  img_data: Pointer to image data array (RGB565 format, 2 bytes per pixel)
 * @retval None
 * @note   1. Image format: RGB565 (2 bytes per pixel)
 *         2. Image data must be stored in memory (RAM or Flash)
 *         3. This function uses CPU to transfer data, for large images consider using DMA version
 */
void lcd_show_internal_img(uint16_t x_start, uint16_t y_start,
                           uint16_t img_width, uint16_t img_height, const uint8_t *img_data)
{
    // Step 1: Parameter validity check
    LCD_InfoType *p_lcd_info = LCD_GetInfo();
    uint16_t x_end = x_start + img_width - 1;
    uint16_t y_end = y_start + img_height - 1;
    
    if (img_data == NULL)
    {
        return; // Null pointer check
    }
    
    if (x_start >= p_lcd_info->width || y_start >= p_lcd_info->height ||
        x_end >= p_lcd_info->width || y_end >= p_lcd_info->height)
    {
        return; // Display area out of LCD screen
    }

    // Step 2: Calculate image data size
    uint32_t total_data_len = (uint32_t)img_width * img_height * 2; // RGB565: 2 bytes/pixel
    
    // Step 3: Set LCD display window
    lcd_set_block(x_start, y_start, x_end, y_end);

    // Step 4: Write image data to LCD (byte by byte)
    for (uint32_t i = 0; i < total_data_len; i++)
    {
        lcd_wr_data(img_data[i]);
    }
}

/**
 * @brief  Display image from internal array using DMA for faster transfer
 * @param  x_start: LCD display start X coordinate
 * @param  y_start: LCD display start Y coordinate
 * @param  img_width: Image width in pixels
 * @param  img_height: Image height in pixels
 * @param  img_data: Pointer to image data array (RGB565 format, 2 bytes per pixel)
 * @retval None
 * @note   1. Image format: RGB565 (2 bytes per pixel)
 *         2. Image data must be stored in RAM (DMA cannot directly access Flash)
 *         3. This function requires DMA to be initialized before use
 *         4. For large images, DMA transfer is significantly faster than CPU
 */
void lcd_show_internal_img_dma(uint16_t x_start, uint16_t y_start,
                               uint16_t img_width, uint16_t img_height, const uint8_t *img_data)
{
    // Step 1: Parameter validity check
    LCD_InfoType *p_lcd_info = LCD_GetInfo();
    uint16_t x_end = x_start + img_width - 1;
    uint16_t y_end = y_start + img_height - 1;
    
    if (img_data == NULL)
    {
        return; // Null pointer check
    }
    
    if (x_start >= p_lcd_info->width || y_start >= p_lcd_info->height ||
        x_end >= p_lcd_info->width || y_end >= p_lcd_info->height)
    {
        return; // Display area out of LCD screen
    }

    // Step 2: Calculate image data size
    uint32_t total_data_len = (uint32_t)img_width * img_height * 2; // RGB565: 2 bytes/pixel
    
    // Step 3: Set LCD display window
    lcd_set_block(x_start, y_start, x_end, y_end);

    // Step 4: Use DMA to transfer image data
    // Note: Need to cast away const for DMA function (DMA won't modify the data)
    lcd_wr_data_dma((uint8_t *)img_data, total_data_len);
}


/* DMA transfer status flag */
volatile uint8_t lcd_dma_transfer_complete = 1;

 
void lcd_wr_data_dma(uint8_t *data, uint32_t length)
{
    
     //while(lcd_dma_transfer_complete == 0);
    
    
    // lcd_dma_transfer_complete = 0;
    
     
    LCD_DMA_CHANNEL->dtcnt = length;
    LCD_DMA_CHANNEL->paddr = (uint32_t)data;
    LCD_DMA_CHANNEL->maddr = XMC_LCD_DATA;
    
    
    dma_channel_enable(LCD_DMA_CHANNEL, TRUE);
    
     
    // while(lcd_dma_transfer_complete == 0);
}

void lcd_clear_dma(uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();
    
     
    lcd_set_block(0, 0, lcd_info->width - 1, lcd_info->height - 1);
    
     
    uint32_t total_pixels = (uint32_t)lcd_info->width * lcd_info->height;
    uint32_t total_bytes = total_pixels * 2;
    
     
    #define CLEAR_BATCH_SIZE 512  
    static uint8_t clear_buffer[CLEAR_BATCH_SIZE];
    
    
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;
    
   
    for(uint32_t i = 0; i < CLEAR_BATCH_SIZE; i += 2)
    {
        clear_buffer[i] = color_high;
        clear_buffer[i + 1] = color_low;
    }
    
    
    uint32_t remaining = total_bytes;
    
    while(remaining > 0)
    {
        uint32_t current_size = (remaining > CLEAR_BATCH_SIZE) ? CLEAR_BATCH_SIZE : remaining;
        
         
        lcd_wr_data_dma(clear_buffer, current_size);
        
        remaining -= current_size;
    }
}

void lcd_fill_dma(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_InfoType *lcd_info = LCD_GetInfo();
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;

    
    if (x1 >= lcd_info->width || y1 >= lcd_info->height)
        return;

    if (x2 >= lcd_info->width)
        x2 = lcd_info->width - 1;

    if (y2 >= lcd_info->height)
        y2 = lcd_info->height - 1;

    
    if (x1 > x2)
    {
        uint16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if (y1 > y2)
    {
        uint16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

     
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    uint32_t total_pixels = (uint32_t)width * height;
    uint32_t total_bytes = total_pixels * 2;
    
     
    lcd_set_block(x1, y1, x2, y2);
    
    
    #define FILL_BATCH_SIZE 512   
    static uint8_t fill_buffer[FILL_BATCH_SIZE];
    
    
    for (uint32_t i = 0; i < FILL_BATCH_SIZE; i += 2)
    {
        fill_buffer[i] = color_high;
        fill_buffer[i + 1] = color_low;
    }
    
    
    uint32_t remaining = total_bytes;
    
    while (remaining > 0)
    {
        uint32_t current_size = (remaining > FILL_BATCH_SIZE) ? FILL_BATCH_SIZE : remaining;
        
         
        lcd_wr_data_dma(fill_buffer, current_size);
        
        remaining -= current_size;
    }
}


 

/**
  * @brief  Initialize LCD controller
  *         Must be called before any write/read operation on the LCD
  * @param  none
  * @retval none
  */
void lcd_init(void)
{
     delay_ms(50);
    /* Hardware reset sequence */
    LCD_RESET_HIGH;
     delay_ms(1);
    LCD_RESET_LOW;
     delay_ms(10);
    LCD_RESET_HIGH;
     delay_ms(120);
    /* Read LCD ID and store in structure */
    g_lcd_info.id = lcd_read_id();
    /* LCD initialization commands (for ILI9488 controller) */
    lcd_wr_command(0x36);
    lcd_wr_data(0x00);
    lcd_wr_command(0x3a);
    lcd_wr_data(0x55);
    lcd_wr_command(0xC0);
    lcd_wr_data(0x0F);
    lcd_wr_data(0x0F);          // 4.5V
    lcd_wr_command(0xC1);
    lcd_wr_data(0x41);             // VGH = 6*VCI   VGL = -4*VCI
    lcd_wr_command(0xC5);
    lcd_wr_data(0x00);
    lcd_wr_data(0x57);
    lcd_wr_data(0x80);        // 4E 51  CD   52
    lcd_wr_command(0xB1);
    lcd_wr_data(0xA0);
    lcd_wr_command(0xB4);
    lcd_wr_data(0x02);              // 2 DOT
    lcd_wr_command(0x36);
    lcd_wr_data(0x48);
    lcd_wr_command(0x3A);
    lcd_wr_data(0x55);           // 55  66
    lcd_wr_command(0x21);
    lcd_wr_data(0x00);            // IPS mode
    lcd_wr_command(0xE9);
    lcd_wr_data(0x00);
    lcd_wr_command(0xF7);
    lcd_wr_data(0xA9);
    lcd_wr_data(0x51);
    lcd_wr_data(0x2C);
    lcd_wr_data(0x82);
    lcd_wr_command(0xE0);  // Positive gamma correction
    lcd_wr_data(0x00);
    lcd_wr_data(0x09);
    lcd_wr_data(0x0C);
    lcd_wr_data(0x02);
    lcd_wr_data(0x0E);
    lcd_wr_data(0x05);
    lcd_wr_data(0x2F);
    lcd_wr_data(0x46);
    lcd_wr_data(0x47);
    lcd_wr_data(0x04);
    lcd_wr_data(0x0E);
    lcd_wr_data(0x0B);
    lcd_wr_data(0x2D);
    lcd_wr_data(0x36);
    lcd_wr_data(0x0F);
    lcd_wr_command(0xE1);  // Negative gamma correction
    lcd_wr_data(0x00);
    lcd_wr_data(0x10);
    lcd_wr_data(0x13);
    lcd_wr_data(0x03);
    lcd_wr_data(0x10);
    lcd_wr_data(0x06);
    lcd_wr_data(0x3A);
    lcd_wr_data(0x46);
    lcd_wr_data(0x51);
    lcd_wr_data(0x06);
    lcd_wr_data(0x10);
    lcd_wr_data(0x0D);
    lcd_wr_data(0x33);
    lcd_wr_data(0x37);
    lcd_wr_data(0x0F);
    /* Set display mode and scanning direction */
    lcd_set_display_mode(LCD_DISPLAY_LANDSCAPE, LCD_SCAN_TB_LR);
    lcd_wr_command(0x11);   // Sleep out
     delay_ms(120);
    lcd_wr_command(0x29);  // Display on
	lcd_dma_transfer_complete = 1;
}

/**
  * @}
  */

/**
  * @}
  */