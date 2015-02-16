
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
extern float last_r;

static __IO uint32_t TimingDelay;
extern int8_t pred[2];
extern uint16_t min, max;

int main()
{   
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }

  config_ms_timer();
  config_us_delay();
  
#ifdef OUTMODE
  stony_init(41, 50, 41,
            4, SMH_SELAMP_3V3);
#else
  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
#endif
  
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
  
  uint8_t use_ann = 1;
  int8_t last_pred[2];
  
  pred[0] = 255;
  pred[1] = 255;
  while (1) {
    clear_ENDP1_packet_buffers();
    while (packet_sending == 1);

#ifdef CIDER_MODE
    if (use_ann) {
      stony_image_dual_subsample();
      use_ann = 0;
      last_r = 0;
    }
    else {
      last_pred[0] = pred[0];
      last_pred[1] = pred[1];
      
      if (run_cider() < 0)
        use_ann = 1;
      
      stony_send_cider_image(last_pred, use_ann);
    } 
#else
//    stony_image_dual_subsample();
    stony_send_cider_image(pred, use_ann);
#endif // ifdef CIDER_MODE
    
    while (packet_sending == 1);

  }

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

void pulse_led(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Configure PA.04 (DAC_OUT1), PA.05 (DAC_OUT2) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  DAC_DeInit();
  
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  
  /* DAC Channel2 Init */
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_1, ENABLE);
  
  DAC_SetChannel1Data(DAC_Align_12b_R, 0x0); // 0x64D = 1.26V
  
  DAC_SetChannel1Data(DAC_Align_12b_R, 0x64D); // 0x64D = 1.26V
//  DAC_DualSoftwareTriggerCmd(ENABLE);
  
  while (1) {
    delay_ms(500);
    DAC_SetChannel1Data(DAC_Align_12b_R, 0x0); // 0x64D = 1.26V
    delay_ms(500);
    DAC_SetChannel1Data(DAC_Align_12b_R, 0x64D); // 0x64D = 1.26V
  }
}