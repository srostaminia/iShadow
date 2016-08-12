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
char *genesis = "In the beginning God created the heaven and the earth. 2 And the earth was without form, and void; and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters. 3 And God said, Let there be light: and there was light. 4 And God saw the light, that it was good: and God divided the light from the darkness. 5 And God called the light Day, and the darkness he called Night. And the evening and the morning were the first day. 6 And God said, Let there be a firmament in the midst of the waters, and let it divide the waters from the waters. 7 And God made the firmament, and divided the waters which were under the firmament from the waters which were above the firmament: and it was so. 8 And God called the firmament Heaven. And the evening and the morning were the second day. 9 And God said, Let the waters under the heaven be gathered together unto one place, and let the dry land appear: and it was so. 10 And God called the dry land Earth; and the gathering together of the waters called he Seas: and God saw that it was good. 11 And God said, Let the earth bring forth grass, the herb yielding seed, and the fruit tree yielding fruit after his kind, whose seed is in itself, upon the earth: and it was so. 12 And the earth brought forth grass, and herb yielding seed after his kind, and the tree yielding fruit, whose seed was in itself, after his kind: and God saw that it was good. 13 And the evening and the morning were the third day. 14 And God said, Let there be lights in the firmament of the heaven to divide the day from the night; and let them be for signs, and for seasons, and for days, and years: 15 And let them be for lights in the firmament of the heaven to give light upon the earth: and it was so. 16 And God made two great lights; the greater light to rule the day, and the lesser light to rule the night: he made the stars also. 17 And God set them in the firmament of the heaven to give light upon the earth, 18 And to rule over the day and over the night, and to divide the light from the darkness: and God saw that it was good. 19 And the evening and the morning were the fourth day. 20 And God said, Let the waters bring forth abundantly the moving creature that hath life, and fowl that may fly above the earth in the open firmament of heaven. 21 And God created great whales, and every living creature that moveth, which the waters brought forth abundantly, after their kind, and every winged fowl after his kind: and God saw that it was good. 22 And God blessed them, saying, Be fruitful, and multiply, and fill the waters in the seas, and let fowl multiply in the earth. 23 And the evening and the morning were the fifth day. 24 And God said, Let the earth bring forth the living creature after his kind, cattle, and creeping thing, and beast of the earth after his kind: and it was so. 25 And God made the beast of the earth after his kind, and cattle after their kind, and every thing that creepeth upon the earth after his kind: and God saw that it was good. 26 And God said, Let us make man in our image, after our likeness: and let them have dominion over the fish of the sea, and over the fowl of the air, and over the cattle, and over all the earth, and over every creeping thing that creepeth upon the earth. 27 So God created man in his own image, in the image of God created he him; male and female created he them. 28 And God blessed them, and God said unto them, Be fruitful, and multiply, and replenish the earth, and subdue it: and have dominion over the fish of the sea, and over the fowl of the air, and over every living thing that moveth upon the earth. 29 And God said, Behold, I have given you every herb bearing seed, which is upon the face of all the earth, and every tree, in the which is the fruit of a tree yielding seed; to you it shall be for meat. 30 And to every beast of the earth, and to every fowl of the air, and to every thing that creepeth upon the earth, wherein there is life, I have given every green herb for meat: and it was so. 31 And God saw every thing that he had made, and, behold, it was very good. And the evening and the morning were the sixth day.";
char *gettysburg = "Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal. Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, under God, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not perish from the earth.";
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
char *nStrcat(char *dest, const char *src, int n);
void tenSecondDelay();
void fiveSecondDelay();
void oneSecondDelay();
void nSecondDelay(int n);
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
    SendData("\n\n\n");
    i = 0;
    while (i < 1000000) i++;
//    SendAsPackets("What hath God wrought");
    SendAsPackets(gettysburg);
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
int messageLen = strlen(message);
int k = 0;
int bytesForward; //how many bytes did we send in this loop?
while (k < messageLen)
  {
    char tempPacket[1024]; //arbitrary size
    strcpy(tempPacket, "|"); //initial character for data, this is what should be split on
    
      nStrcat(tempPacket, &message[k], 256); //append data
      bytesForward = 256;
      
    strcat(tempPacket, "$"); //end character for data
    
    char packet[1024]; //we'll send this char array
    strcpy(packet, "|"); //initial character for the whole packet
    char num[16];
    sprintf(num, "%i", packetNum);
    strcat(packet, num); //packet number
    strcat(packet, "|"); //separator between packet number and hash
    
    unsigned int hash = getChecksum(tempPacket); //NOTE: checksum includes starting '|' and ending '$'
    char temp[16];
    sprintf(temp, "%i", hash);
    strcat(packet, temp); //data hash added
    strcat(packet, tempPacket); //data added
    
    SendData(packet);
    
    char OK = 'O'; //this is what the app will send as OK
    char RETRANSMIT = 'R';
    char ack = GetCharUSART1();
      if (ack == OK)
      {
        k += bytesForward; //k is the index in the message to read from
        packetNum++;
        nSecondDelay(1); //Give android some time to proces
        continue;
      }
      else
      { //go back and retransmit the packet; if OK isn't received the index doesn't move forward,
        //so we return to the top of the loop and transmit the same data again
        nSecondDelay(1); //Give android some time to proces
      }
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
This method copies the source string into the destination char array, stopping
either after coying n entries or at the end of the string, whichever is first.
@param n The maximum number of characters to copy from the src char array.
*/
char *nStrcat(char *dest, const char *src, int n)
{
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0' && j < n; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

/**
Pause for ten seconds.
*/
void tenSecondDelay()
{
  int z = 0;
  while (z < 40000000) z++;
}

/**
Pause for five seconds.
*/
void fiveSecondDelay()
{
  int z = 0;
  while (z < 20000000) z++;
}

/**
Pause for one second.
*/
void oneSecondDelay()
{
  int z = 0;
  while (z < 4000000) z++;
}

/**
Pause for n seconds.
*/
void nSecondDelay(int n)
{
  int z = 0;
  int p = n * 4000000;
  while (z < p) z++;
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
  GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);
  
  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

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