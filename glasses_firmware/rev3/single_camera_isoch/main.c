
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

extern uint32_t MUTE_DATA;
extern uint16_t In_Data_Offset;
extern uint16_t Out_Data_Offset;
extern uint8_t Stream_Buff[24];
extern uint8_t IT_Clock_Sent;

uint8_t packet_sending = 0;

static __IO uint32_t TimingDelay;

int main()
{ 
  uint8_t tx_test[PACKET_SIZE], empty[PACKET_SIZE];
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }

  config_ms_timer();
  
  for (int i = 0; i < PACKET_SIZE; i++) {
    tx_test[i] = i + 1;
    empty[i] = 0;
  }
  
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
  
  uint16_t start = 0, val = 0;
  while (1) {
    
    // Wait 5 seconds before retransmitting
//    if (TIM3->CNT > 2000) {
//      start = TIM3->CNT;
      send_packet(tx_test, PACKET_SIZE);
      
      tx_test[0] = val;
      val++;
      
      while (packet_sending == 1);
      
//      send_packet(empty, 16);      
//      packet_sending = 1;
//      while (packet_sending == 1);
      
//      delay_ms(100);
//      
//      send_packet(empty, 16);
//      
//      while (packet_sending == 1);
      
//      clear_ENDP1_packet_buffers(16);
      
//      TIM3->CNT = 0;
//    }
      

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
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM3, ENABLE);
}

void delay_ms(int delayTime)
{
  TIM3->CNT = 0;
  while((uint16_t)(TIM3->CNT) <= delayTime);
}