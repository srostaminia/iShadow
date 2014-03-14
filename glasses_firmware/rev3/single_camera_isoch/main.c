
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_regs.h"

extern uint32_t MUTE_DATA;
extern uint16_t In_Data_Offset;
extern uint16_t Out_Data_Offset;
extern uint8_t Stream_Buff[24];
extern uint8_t IT_Clock_Sent;

uint8_t packet_sending = 0;

static __IO uint32_t TimingDelay;

int main()
{ 
  uint8_t tx_test[16];
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  for (int i = 0; i < 16; i++) {
    tx_test[i] = 0;
  }
  
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
  
  uint8_t value = 0;
  tx_test[0] = 0;
  while (1) {
    for (int i = 1; i < 16; i++) {
      value++;
      tx_test[i] = value;
    }
    
    while (packet_sending == 1);
    
    send_packet(tx_test, 16);
  };
  
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