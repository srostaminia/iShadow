
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
  uint8_t tx_test[16];
//  int16_t large_test[8];
//  uint8_t *tx_test = (uint8_t*)large_test;
  
  uint8_t empty[16];
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  for (int i = 0; i < 16; i++) {
    tx_test[i] = 4;
    empty[i] = 0;
  }
  
//  for (int i = 0; i < 8; i++) {
//    large_test[i] = 3;
//  }
  
//  for (int i = 0; i < 16; i += 2) {
//    tx_test[i] = 0;
//    tx_test[i + 1] = 1;
//  }
  
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  Speaker_Config();
  
  while (1) {
    while (packet_sending == 1);
    
    send_packet(tx_test, 16);
//    tx_test[0] += 1;
    
//    while (packet_sending == 1);
//    
//    send_packet(empty, 16);
  };
  
//  while (1)
//  {
//    if (bDeviceState == CONFIGURED)
//    {
//      CDC_Receive_DATA();
//      /*Check to see if we have data yet */
//      if (Receive_length  != 0)
//      {
//        if (packet_sending == 1)
//          CDC_Send_DATA ((unsigned char*)Receive_Buffer,Receive_length);
//        Receive_length = 0;
//      }
//    }
//  }
  
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