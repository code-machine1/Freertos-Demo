#ifndef _LCD_HANDLE_H
#define _LCD_HANDLE_H
#include "lcd.h"

#define LCD_HANDLE_TIME               1
#define TEMP_REFRESH_TIME             300
#define ALARM_REFRESH_TIME            300
#define SET_TEMP_TIME                 300
#define SET_WIND_TIME                 250
#define SET_TIME_TIME                 500


#define CH_SAVE_TIME                  300



#define FIRST_START_TIME              500
#define CH_SET_TIME                   300

#define SET_ON_TIME                   300
#define SET_0FF_TIME                  300

#define SHOW_RESET_TIME               300

#define SUN_FLISH_TIME                500

#define COLD_REFRESH_TIME             600
#define DELAY_ENTER_COLD_MODE_TIME    700

#define FAN_DISPLAY_TIME              50


#define ERROR_FLISH_TIME              400

#define FLASH_COUNT                   4

typedef enum
{
	
    SHOW_INIT,
    SHOW_WIND_FLASH,
    SHOW_TEMP_ACTUAL,
    SHOW_TEMP_ACTUAL_FLOAT,
    SHOW_WIND_SET,
    SHOW_SUN,
    SHOW_ADJUST,
    SHOW_SPEKER,
    SHOW_TIMER,
    SHOW_TIME_SET,
    SHOW_DISPLAY_MODE,
    SHOW_SET_TIME,
} lcd_handle_t;

typedef struct
{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t bc;
    uint16_t fc;
    uint8_t sizey;
    uint8_t mode;
    uint8_t len;
    uint16_t *num;

} title_t;

extern uint8_t cold_mode;           
extern uint16_t cold_mode_set_wind;  
extern int16_t set_calibration_temp;
extern uint8_t refresh_set_wind_flag;
extern uint8_t set_temp_flag;
extern uint16_t set_time;
extern uint8_t set_time_flag;
extern uint8_t time_countdown_flag;
extern int16_t countdown_time;
extern uint16_t actual_temp;
extern uint16_t set_temp;
extern uint16_t set_wind;
extern uint16_t set_temp_f;
extern uint8_t lock_mode;

extern uint8_t display_mode;
extern uint8_t speak_mode;
extern  uint8_t show_ch_value_flag;

extern uint8_t save_ch_flag;
extern uint8_t show_on_flag;
extern uint8_t show_off_flag;
extern uint8_t reset_flag;
extern uint16_t last_system_set_temp;
extern uint16_t last_system_set_wind;

extern uint16_t set_temp_ch1;
extern uint16_t set_wind_ch1;
extern uint16_t set_time_ch1;
extern uint16_t set_temp_ch2;
extern uint16_t set_wind_ch2;
extern uint16_t set_time_ch2;
extern uint16_t set_temp_ch3;
extern uint16_t set_wind_ch3;
extern uint16_t set_time_ch3;
extern uint16_t set_temp_ch4;
extern uint16_t set_wind_ch4;
extern uint16_t set_time_ch4;
extern uint8_t ch;

extern uint8_t set_speak_flag;

/* 显示校准温度值 */
extern int16_t external_temp;
extern int16_t external_temp_f;
extern uint8_t set_calibration_temp_flag;
extern uint16_t last_external_temp;                //上一次温度校准值

 extern    uint16_t last_actual_temp;          //实际温度值整数
 extern    uint16_t last_actual_temp_float;    //实际温度值小数

extern uint16_t cal_time_out;
extern uint16_t last_set_time;             //上次设置值

extern uint8_t last_ch;

void LcdProc(void);

#endif
