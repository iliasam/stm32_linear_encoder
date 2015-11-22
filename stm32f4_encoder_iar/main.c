#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_flash.h"
#include "adc_control.h"
#include "stm32f4_discovery.h"
#include "main.h"
#include "lcd_driver.h"
#include "encoder_analyse.h"


RCC_ClocksTypeDef RCC_Clocks;
void init_clk(void);

volatile int8_t angle = 0;

void init_timer(void);

extern int32_t main_cnt;

int32_t micro_value = 0;
volatile int32_t tmp_val;

volatile int8_t zero_angle  =0;
volatile int32_t tmp_angle = 0;

volatile uint8_t tmp_val2=0;
//volatile uint8_t tmp_val3=0;

#define ENCODER_BIG_STEP 20 //20um

//in interpolation mode negative values not supported
//start value can not be set to full zero
#define USE_INTERPOLATION

int main(void)
{
  init_clk();
  RCC_GetClocksFreq(&RCC_Clocks);
  adc_init();
  init_tim2();
  init_lcd();
  
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
  
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  
  while(1)
  {
    if (STM_EVAL_PBGetState(BUTTON_USER)!=0)
    {
      main_cnt = 0;
      tmp_val2 = (uint8_t)angle/(ENCODER_BIG_STEP/4);
      main_cnt+=(int32_t)tmp_val2;
      //zero_angle = angle;
    }
    
    
#ifdef USE_INTERPOLATION
    angle = get_interpol_value();
    tmp_val = main_cnt/4;
    micro_value = tmp_val*ENCODER_BIG_STEP + angle;
    if (main_cnt<0) micro_value = -1;//negative not supported
#else
    micro_value = main_cnt*(ENCODER_BIG_STEP/4);    
#endif
    draw_number(micro_value, 1, 7);
    update_lcd();
    
    Delay_ms(10);
  }
}

void init_clk(void)
{
  
  //Настраиваем систему тактирования
  //автоматически настоена на HSI - 168MHZ
  //RCC_ClocksTypeDef RCC_ClockFreq;
  ErrorStatus HSEStartUpStatus;
  
  RCC_HSICmd(ENABLE);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  while(RCC_GetSYSCLKSource() != 0x00)
  {;}
  
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_ON);
  // Wait till HSE is ready
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if(HSEStartUpStatus == SUCCESS)
  {
    FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;
    
    //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    //FLASH_SetLatency(FLASH_Latency_2);
    
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div2);//APB2 < 90mhz
    RCC_PCLK1Config(RCC_HCLK_Div4);//<40mhz
    
    RCC_PLLConfig(RCC_PLLSource_HSE,4,168,2,7);
    //RCC_PLLConfig(RCC_PLLSource_HSE,8,336,2,7);
    RCC_PLLCmd(ENABLE);
    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)  {;}
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08) {;}
  }
}

void Delay_ms(uint32_t ms)
{
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
  for (; nCount!=0; nCount--);
}
