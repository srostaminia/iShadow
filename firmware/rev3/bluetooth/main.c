/**
  ******************************************************************************
  * @file    USART/Printf/main.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    20-April-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
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
/* Private function prototypes -----------------------------------------------*/
static void USART_Config(void);
static void SysTickConfig(void);
void SendCharUSART1(char ch);
char GetCharUSART1(void);
void USART_Communication(void);
void SendStringUSART1(void);
void Delay(__IO uint32_t nTime);
int SendImgData(void);
void ReadFile(char *name);
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
  /* USART configuration -----------------------------------------------------*/
  USART_Config();
  
  /* SysTick configuration ---------------------------------------------------*/
  SysTickConfig();
  
/*Trying to send data here, with helper functions */
//USART_Communication();
//  SendStringUSART1();

ReadFile("img.png");
  
//  while(1){
//SendStringUSART1();
////Delay(50);
//int j=0;
//while(j<20){
//  j++;
//int i=0;
//while(i<1000000) i++;
//}
//  }
}

void SendStringUSART1(void) {
char str[] = "AT+AB SPPDisconnect \r\n"; //put string here////////////////////////////////////////
char *s;
s = str;
while(*s)
{
 while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); 
 USART_SendData(USARTx, *s++); 
 /*Check that transmission has finished*/
 while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}
}

int SendImgData()
{
  ReadFile("img.png");
  return 0;
/*
open file
increment byte by byte through the buffer and send each byte
close file  
*/
}  

void USART_Communication(void){
  
//	char ch;
	while(1) {
                SendCharUSART1('0');
//                SendCharUSART1('1');
//		SendCharUSART1(0x0D);
//		SendCharUSART1(0x0A);
//		SendCharUSART1('U');
//		SendCharUSART1('S');
//		SendCharUSART1('A');
//		SendCharUSART1('R');
//		SendCharUSART1('T');
//		SendCharUSART1('1');
//		SendCharUSART1('>');
//		ch = GetCharUSART1();
                int i = 0;
                while (i<30000){
                i++;
                }
//                break;
// Get and echo USART1
//		ch = GetCharUSART1();
//		while (ch != 0x0D) {
//			SendCharUSART1(ch);
//			ch = GetCharUSART1();
//		}
	}
}

void SendCharUSART1(char ch){
// Wait until TXE is set
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
	}
//        USARTx->DR = (ch & (uint16_t)0x01FF);
//        int test = (ch & (uint16_t)0x01FF);
	USART_SendData(USART1, ch);
// Wait until the end of transmit
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{
	}
}
char GetCharUSART1(void){
	char ch;
// Wait until the USART1 Receive Data Register is not empty
//        Fill_Buffer(USARTx->DR,  0x02);
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{
	}
	ch = (USART_ReceiveData(USART1) & 0xFF);
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



//Function protypes
//void ReadFile(char *name);
void dump_buffer(void *buffer, int buffer_size);
void dump_buffer_hex(void *buffer, int bufferSize);
void saveFile();
void writeBuffer(void *buffer, int bufferSize);


//int main() 
//{
//	ReadFile("img.png");
//	// printf("Johnny boy in da haus\n");
//	return 0;
//}


void ReadFile(char *name)
{
	FILE *file;
	char *buffer;
	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	//Do what ever with buffer

	/*My stuff here*/
	// printf("%i", fileLen);
	// dump_buffer_hex(buffer, 1000);
//        saveFile(buffer, fileLen);
//	writeBuffer(buffer, fileLen);
        char *s;
        s = buffer;
        while(*s)
        {
         while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); 
         USART_SendData(USARTx, *s++); 
         /*Check that transmission has finished*/
         while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
        }
	/*My stuff ends here*/

	free(buffer);


}

void dump_buffer(void *buffer, int buffer_size)
{
  int i;

  for(i = 0;i < buffer_size;++i)
     printf("%c", ((char *)buffer)[i]);
}

void dump_buffer_hex(void *buffer, int bufferSize)
{
	int c;
	for (c=0;c<bufferSize;++c)
{
     printf("%.2X ", ((char *)buffer)	[c]);

     // put an extra space between every 4 bytes
     if (c % 4 == 3)
     {
         printf(" ");
     }

     // Display 16 bytes per line
     if (c % 16 == 15)
     {
         printf("\n");
     }
}
// Add an extra line feed for good measure
printf("\n");
}

void saveFile(void *buffer, int bufferSize)
/*avoid this function, it formats the output incorrectly*/
{
FILE *f = fopen("output.txt", "w");
if (f == NULL)
{
    printf("Error opening file!\n");
    exit(1);
}
/* my stuff */
fprintf(f, "%s\n", (char *)buffer);

	int c;
	for (c=0;c<bufferSize;++c)
{
     fprintf(f, "%.2X ", ((char *)buffer)	[c]);

     // put an extra space between every 4 bytes
     if (c % 4 == 3)
     {
         fprintf(f, " ");
     }

     // Display 16 bytes per line
     if (c % 16 == 15)
     {
         fprintf(f, "\n");
     }
}
// Add an extra line feed for good measure
fprintf(f, "\n");
/*end my stuff*/

// /* print some text */
// const char *text = "Write this to the file";
// fprintf(f, "Some text: %s\n", text);

// /* print integers and floats */
// int i = 1;
// float py = 3.1415927;
// fprintf(f, "Integer: %d, float: %f\n", i, py);

// /* printing single chatacters */
// char c = 'A';
// fprintf(f, "A character: %c\n", c);

fclose(f);
}

void writeBuffer(void *buffer, int bufferSize)
{
FILE* pFile;
pFile = fopen("output.txt","wb");

if (pFile){
    fwrite(buffer, bufferSize, 1, pFile);
    puts("Wrote to file!");
}
else{
    puts("Something wrong writing to File.");
}

fclose(pFile);
}