#ifndef __ADC_FILTER_H
#define __ADC_FILTER_H

#include "at32f403a_407.h"


#define WINDOW_SIZE 10
typedef struct
{
    float input_data[WINDOW_SIZE];
    uint16_t sum;
	uint16_t cur_count;
	adc_channel_select_type adc_channel;
}
filter_t;

extern filter_t s1_adc;
extern filter_t s2_adc;
extern filter_t s1_current;
extern filter_t s2_current;
u16 get_adcval(adc_channel_select_type adc_channel);
uint32_t move_average_filter(filter_t *filter);
uint8_t filter_init(filter_t * filter,adc_channel_select_type adc_channel);
#endif