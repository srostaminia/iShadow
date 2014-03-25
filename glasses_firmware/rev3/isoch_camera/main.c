
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "libstony.h"

extern uint32_t MUTE_DATA;
extern uint16_t In_Data_Offset;
extern uint16_t Out_Data_Offset;
extern uint8_t Stream_Buff[24];
extern uint8_t IT_Clock_Sent;

volatile uint8_t packet_sending = 0;

extern int fpn_offset;

static __IO uint32_t TimingDelay;
//extern uint16_t pred_img[112][112];
extern int8_t pred[2];
extern uint16_t min, max;

int main()
{ 
//  uint8_t tx_test[PACKET_SIZE];
//  
//  for (int i = 0; i < PACKET_SIZE; i++) {
//    tx_test[i] = i + 1;
//  }
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }

  config_ms_timer();
  config_us_delay();
  
  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
  
  pred[0] = 255;
  pred[1] = 255;
  while (1) {
    clear_ENDP1_packet_buffers();
    while (packet_sending == 1);
    
//    stony_image_single();
    stony_image_dual_subsample();
    while (packet_sending == 1);
    
//    for (int i = 0; i < 11; i++) {
//      packet_sending = 1;
//      while (packet_sending == 1);
//    }
    
//    delay_ms(100);
  }
  
//  uint16_t val = 0;
//  while (1) {
//      send_packet(tx_test, PACKET_SIZE);
//      
//      tx_test[0] = val;
//      val++;
//      
//      while (packet_sending == 1);
//  }
//  
  return 0;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void config_ms_timer()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM4, ENABLE);
}

#pragma inline=never
void delay_ms(int delayTime)
{
  TIM4->CNT = 0;
  while((uint16_t)(TIM4->CNT) <= delayTime);
}

void config_us_delay()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM5, ENABLE);
}

#pragma inline=never
void delay_us(int delayTime)
{
//  uint16_t start = TIM5->CNT;
//  while((uint16_t)(TIM5->CNT - start) <= delayTime);
  
  for (int i = 0; i < delayTime; i++) {
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
  }
}