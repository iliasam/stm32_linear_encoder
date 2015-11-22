#ifndef __ADC_CONTROL_H
#define __ADC_CONTROL_H

#include <stdint.h>

void init_clk(void);
void adc_init(void);
void init_tim2(void);

void stop_capture(void);
void start_capture(void);

void capture_dma_start(void);

void start_clk_timer(void);
void init_capture_gpio(void);



#endif 

