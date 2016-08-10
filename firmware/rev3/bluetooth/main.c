/**
  ******************************************************************************
  * @file    bluetooth/main.c 
  * @author  Colin Stern
  * @version V1.0.0
  * @date    8-August-2016
  * @brief   Main program body - referred to as the btserver script
  ******************************************************************************
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
char *bdaddress = "CCFA0071AF72"; //The address of the device to connect to
uint32_t MAX_INT = 2147483647;
/* Private function prototypes -----------------------------------------------*/
static void USART_Config(void);
static void SysTickConfig(void);
char GetCharUSART1(void);
void SendStringUSART1(char *message);
void Delay(__IO uint32_t nTime);
int SendData(char *data);
void ConnectAndroid(void);
void SendAsPackets(char *message);
unsigned int getChecksum(char *data);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32l1xx_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32l1xx.c file
     */
  
 
//  ConnectAndroid(); //This is the address of our android device
//  
//  SendData("What hath God wrought");
  
   /* USART configuration -----------------------------------------------------*/
  USART_Config();
  
  /* SysTick configuration ---------------------------------------------------*/
  SysTickConfig();
  
  /* Change name -------------------------------------------------------------*/
//  SendStringUSART1("AT+AB LocalName btserver  \r\n");
  
//  SendStringUSART1("AT+AB SPPConnect CCFA0071AF72 ");
  while(1) {
    int i = 0;
    while (i < 1000000) i++;
    SendData("What hath God wrought\n");
  }
}




/**
Connects the microcontroller to the android device with the specified address.
@param bdaddress The address of device to connect to
*/
void ConnectAndroid()
{
  /* USART configuration -----------------------------------------------------*/
  USART_Config();
  
  /* SysTick configuration ---------------------------------------------------*/
  SysTickConfig();
  
  /* Change name -------------------------------------------------------------*/
  SendStringUSART1("AT+AB LocalName btserver  \r\n");
  
  /* Enable bond -------------------------------------------------------------*/
//  char bondConcat[80];
//  strcpy(bondConcat, "AT+AB EnableBond ");
//  strcat(bondConcat, bdaddress);
//  strcat(bondConcat, " \r\n");
//  SendStringUSART1(bondConcat);
  
  /* Initiate bond -----------------------------------------------------------*/  
//  char initConcat[80];
//  strcpy(initConcat, "AT+AB Bond ");
//  strcat(initConcat, bdaddress);
////  strcat(initConcat, " 1234");
//  strcat(initConcat, " \r\n");
//  SendStringUSART1(initConcat); 
  
  /* Connect -----------------------------------------------------------------*/  
  char connectConcat[80];
  strcpy(connectConcat, "AT+AB SPPConnect ");
  strcat(connectConcat, bdaddress);
  strcat(connectConcat, " \r\n");
  SendStringUSART1(connectConcat);
}

/**
Breaks a character array into pieces of 512 bytes, adds a header containing the number and
hash of each piece (packet), and sends the packet.
@param message The message to be sent.
*/
void SendAsPackets(char *message)
{
int packetNum = 0;
char *s;
s = message;
while(*s)
  {
    char tempPacket[1024]; //arbitrary size
    int i;
    strcpy(tempPacket, "|"); //initial character for data, this is what should be split on
    for (i = 0; i < 512; i ++) 
    {
    strcat(tempPacket, s++); //append data - not sure why s++ works and not *s++
    }
    strcat(tempPacket, "|"); //end character for data
    
    char packet[1024]; //we'll send this one
    strcpy(packet, "|"); //initial character for the whole packet
    char num[16];
    sprintf(num, "%i", packetNum);
    strcat(packet, num); //packet number
    strcat(packet, "|"); //separator between packet number and hash
    
    unsigned int hash = getChecksum(tempPacket);
    char temp[16];
    sprintf(temp, "%i", hash);
    strcat(packet, num); //data hash
    strcat(packet, tempPacket); //data added
    
    sendData(packet);
    
    char OK = 'O'; //this is what the app will send as OK
    char ack = getCharUSART1();
      if (ack == OK)
        continue;
      else
        *s -= 512; //go back and retransmit the packet again
  }
}

unsigned int getChecksum(char *data)
{
  unsigned int checksum = 0;
  int sizeOfData = strlen(data);
  int i;
  for (i = 0; i < sizeOfData; i++)
  {
    checksum += data[i];
    checksum %= MAX_INT;
  }
  return checksum;
}


/**
Sends data to a connected device. This device must be connected to the android
when this method is called, or else the data will not be sent.
@param data The data to be sent.
*/
int SendData(char *data)
{
  SendStringUSART1(data);
  return 0;
}

/**
Writes a string to the Tx pin of the bluetooth module, one character at a time.
@param message The message to be sent.
*/
void SendStringUSART1(char *message) {
char *s;
s = message;
while(*s)
  {
     while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); 
     USART_SendData(USARTx, *s++); 
     /*Check that transmission has finished*/
     while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
  }
}

char GetCharUSART1(void){
	char ch;
// Wait until the USART1 Receive Data Register is not empty
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	ch = (USART_ReceiveData(USARTx) & 0xFF);
	return ch;
}

/**
  * @brief  Configures the USART Peripheral.
  * @param  None
  * @retval None
  */
static void USART_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);
  
  /* Enable USART clock */
  USARTx_APBPERIPHCLOCK(USARTx_CLK, ENABLE);
  
  /* Connect PXx to USARTx_Tx */
  GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);
  
  /* Connect PXx to USARTx_Rx */
  //GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);
  
  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
  //GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  //GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
  - BaudRate = 115200 baud  
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200*3;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
  
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable USART */
  USART_Cmd(USARTx, ENABLE);
}

/**
  * @brief  Configure a SysTick Base time to 10 ms.
  * @param  None
  * @retval None
  */
static void SysTickConfig(void)
{
  /* Setup SysTick Timer for 10ms interrupts  */
  if (SysTick_Config(SystemCoreClock / 100))
  {
    /* Capture error */
    while (1);
  }
  /* Configure the SysTick handler priority */
//  NVIC_SetPriority(SysTick_IRQn, 0x0);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF ORIGINAL FILE****/