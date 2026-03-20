#include "ds1302.h"

#define DELAY_LOOP_CYCLES 10 //   5~15

void DS1302_Delay_Us(uint32_t us)
{
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    uint32_t loops = cycles / DELAY_LOOP_CYCLES;
    volatile uint32_t cnt = loops;
    while (cnt--)
    {
        __NOP();
    }
}

uint8_t DS1302_Read_Byte(uint8_t reg_addr)
{

    reg_addr |= 0x01;

    uint8_t data = 0;

    DS_RST_L;
    DS_CLK_L;

    DS_RST_H;
    DS1302_Delay_Us(5);

    for (int i = 0; i < 8; i++)
    {
        if (reg_addr & (1 << i))
        {
            DS_IO_H;
        }
        else
        {
            DS_IO_L;
        }
        DS_CLK_H;
        DS1302_Delay_Us(1);

        DS_CLK_L;
    }

    DS_IO_H;
    for (uint8_t i = 0; i < 8; i++)
    {

        data |= (DS_GET_IO_DATA << i);
        if (i < 7)
        {
            DS_CLK_H;
            DS_CLK_L;
        }
    }

    DS_RST_L;

    return data;
}

void DS1302_Write_Byte(uint8_t reg_addr, uint8_t data)
{

    reg_addr &= (0xfe);

    DS_RST_L;
    DS_CLK_L;

    DS_RST_H;
    DS1302_Delay_Us(5);

    for (int i = 0; i < 8; i++)
    {
        if (reg_addr & (1 << i))
        {
            DS_IO_H;
        }
        else
        {
            DS_IO_L;
        }

        DS_CLK_H;
        DS1302_Delay_Us(1);
        DS_CLK_L;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (1 << i))
        {
            DS_IO_H;
        }
        else
        {
            DS_IO_L;
        }
        DS_CLK_H;
        DS1302_Delay_Us(1);
        DS_CLK_L;
    }
}