/**
  ******************************************************************************
  *                              (c) Copyright 2024, tangmingfei2013@126.com
  *                                      All Rights Reserved
  * @file    usr_touch.c
  * @author  mingfei tang
  * @version V1.0.0
  * @date    2018/03/25
  * @description:
    This source code and any compilation or derivative thereof is the proprietary
    information of mingfei.tang and is confidential in nature.
    Under no circumstances is this software to be combined with any
    Open Source Software in any way or placed under an Open Source License
    of any type without the express written permission of mingfei.tang
  ******************************************************************************
**/
/* Includes ------------------------------------------------------------------*/
#include "usr_touch.h"
#include "xmc_lcd.h"
#include "ft6336.h"
extern LCD_InfoType g_lcd_info;

stru_pos usr_pos;
const uint16_t touch_press_reg[2]={FT_TP1_REG,FT_TP2_REG};
static int touch_isOK;
 
void usr_touchInit( void )
{
   touch_isOK = ft6336_Init();
}
 
uint8_t usr_ScanTouchProcess( stru_pos *pPos)
{
    uint8_t buf[4];
    uint8_t i = 0, value;
    uint8_t set = FT_FALSE;;
    uint8_t pointNub = 0;
    static uint8_t cnt = 0;
    
    if( touch_isOK == FT_FALSE )
        return set;
    
    cnt++;
    if((cnt%10)==0 || cnt<10)
    {
        // read number of touch points 
        ft6336_rd_register(FT_REG_NUM_FINGER,&pointNub,1);
		
        pointNub= pointNub&0x0f;
        if( pointNub && (pointNub < 3) )
        {
            cnt=0;
            // read the point value 
            pPos->status_bit.tpDown = 1;
            pPos->status_bit.tpPress = 1;
            pPos->status_bit.ptNum = pointNub;
            
            for( i=0; i < CTP_MAX_TOUCH; i++)
            {
                for( int j = 0; j< 4;j++)
                {
                    ft6336_rd_sigleReg( touch_press_reg[i]+j, &value);
                    buf[j] = value;
                }
                
                if( pPos->status_bit.ptNum )
                {
                    switch(g_lcd_info.direction)
                    {
                        case 0:
                            pPos->xpox[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
                            pPos->ypox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
						 
                            break;
                        case 1:

                            pPos->ypox[i]=g_lcd_info.height-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
                            pPos->xpox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
						   
						    
                            break;
//                        case 2:
//                            pPos->xpox[i]=lcddev.width-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
//                            pPos->ypox[i]=lcddev.height-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
//                            break;
//                        case 3:
//                            pPos->ypox[i] = ((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
//                            pPos->xpox[i] = lcddev.width-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
//                            break;
                    } 
                    //printf("x[%d]:%d,y[%d]:%d\r\n",i,pPos->xpox[i],i,pPos->ypox[i]);
                }
            } 
            
            set = FT_TRUE;
            if( pPos->xpox[0]==0 && pPos->ypox[0]==0){
                pPos->status = 0;
            }
        }
		else
		{
		    pPos->status_bit.tpDown = 0;
            pPos->status_bit.tpPress = 0;
            pPos->status_bit.ptNum = 0;
		}
    }
    
    if( pPos->status_bit.ptNum == 0)
    { 
        if(  pPos->status_bit.tpDown )
        {
            pPos->status_bit.tpPress = 0;
        }
        else
        { 
            pPos->xpox[0] = 0xffff;
            pPos->ypox[0] = 0xffff;
            pPos->status = 0;
        }
    }
    
    if( cnt>240 )
       cnt=10;
    
    return set;
}
 
 
/* End of this file */ 