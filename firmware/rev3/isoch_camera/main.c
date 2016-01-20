
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "stonyman.h"
#include "eye_models.h"
#include "assert.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "diskio.h"
#include "stdbool.h"

// TODO: Clean out these externs

extern uint32_t MUTE_DATA;
extern uint16_t In_Data_Offset;
extern uint16_t Out_Data_Offset;
extern uint8_t Stream_Buff[24];
extern uint8_t IT_Clock_Sent;

// FIXME: Remove this
extern uint32_t sd_ptr;

#ifdef CIDER_TRACKING
extern float last_r;
#endif

volatile uint8_t packet_sending = 0;

static __IO uint32_t TimingDelay;
extern uint8_t pred[2];

uint32_t time_elapsed = 0;

int main()
{   
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }

  config_ms_timer();
  config_us_delay();

  // TODO: Switch to stony_init_default()
#ifdef OUTMODE
  stony_init(39, 50, 41,
            2, SMH_SELAMP_3V3);
#else
  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
#endif
  
#ifdef SD_SEND
  init_sd();
#elif defined(USB_SEND)
  init_usb();
#endif
  
  // TODO: Adapt this to new stonyman_conf settings, generally clean it up
#ifdef CIDER_TRACKING
  
  // CIDER uses a very different loop structure than other run modes
  cider_loop();
  
#else
  
  // If CIDER is *off*, use the standard frame-collect loop
  while (1) {
#ifdef USB_SEND
    clear_ENDP1_packet_buffers();
    while (packet_sending == 1);
#endif

    // Record a video frame if configured
#if defined(OUT_VIDEO_ON) && defined(EYE_VIDEO_ON)
    stony_dual();
#elif defined(OUT_VIDEO_ON) || defined(EYE_VIDEO_ON)
    stony_single();
#endif
    
    // Do one ANN eye tracking iteration if it wasn't done implicitly in stony_*
#if defined(EYE_VIDEO_OFF) && defined(ANN_TRACKING)
    stony_ann();
#endif
    
#ifdef USB_SEND
    while (packet_sending == 1);
#endif
  }
  
#endif // CIDER_TRACKING
  
  return 0;
}

#ifdef CIDER_TRACKING
void cider_loop()
{  
  bool use_ann = true;
  
  pred[0] = 255;
  pred[1] = 255;
  
  while (1) {
#ifdef USB_SEND
    clear_ENDP1_packet_buffers();
    while (packet_sending == 1);
#endif

    if (use_ann) {
#if defined(EYE_VIDEO_ON) && defined(OUT_VIDEO_ON)
      stony_dual();
#elif defined(EYE_VIDEO_ON)
      stony_single();
#else
      stony_ann();
#endif

      use_ann = false;
      last_r = 0;
    }
    else {
      if (run_cider() < 0)
        use_ann = true;
      
#if defined(EYE_VIDEO_ON) && defined(OUT_VIDEO_ON)
      mark_cider_packet(use_ann);
      stony_dual2(false);
#elif defined(EYE_VIDEO_ON)
      mark_cider_packet(use_ann);
      stony_single2(false);
#else
      // FIXME: write this function (split finish_tx?)
//      send_cider_packet();
#endif
    }

#ifdef USB_SEND
    while (packet_sending == 1);
#endif

  }
}
#endif // CIDER_TRACKING

#ifdef SD_SEND
void init_sd()
{
  assert (disk_initialize(0) == SD_OK);
  disk_erase(0, SD_EOF_OFFSET, 1);
}
#endif // SD_SEND

#ifdef USB_SEND
void init_usb()
{
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
}
#endif // USB_SEND

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
  TIM4->CNT = 0;
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
  TIM_TimeBaseStructure.TIM_Period = UINT32_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM5, ENABLE);
  TIM5->CNT = 0;
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
