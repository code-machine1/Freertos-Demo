#ifndef __USR_TOUCH_H
#define __USR_TOUCH_H
 
#include <stdlib.h>
#include <stdio.h>
#include "at32f403a_407.h"

#include "soft_i2c.h"




#define CTP_MAX_TOUCH               2
 
typedef struct
{
   uint8_t ptNum                    : 4;
   uint8_t tpDown                   : 1;
   uint8_t tpPress                  : 1;
   uint8_t res                      : 2;
} Status_bit;
 
typedef struct
{
    uint16_t xpox[CTP_MAX_TOUCH];
    uint16_t ypox[CTP_MAX_TOUCH];
    union
    {
        uint8_t status;
        Status_bit status_bit;
    };
}stru_pos;

extern stru_pos usr_pos;

void usr_touchInit( void );
uint8_t usr_ScanTouchProcess( stru_pos *pPos);



#endif    /* __USR_TOUCH_H */