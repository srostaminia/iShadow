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
#include "stm32l1xx.h"
#include "stdio.h"
#include "main.h"

/* Private variables ---------------------------------------------------------*/
char TxBuffer[] = "AT+AB LocalName hullo there";
uint8_t RxBuffer [RXBUFFERSIZE];
__IO uint8_t TxIndex = 0x00;
__IO uint8_t RxIndex = 0x00;
__IO uint8_t UsartMode = USART_MODE_TRANSMITTER;
__IO uint8_t UsartTransactionType = USART_TRANSACTIONTYPE_DATA;
int debugIt = -1;

uint8_t CmdBuffer [0x02] = {0x00, 0x00}; /* {Transaction Command, 
Number of byte to receive or to transmit} */
uint8_t AckBuffer [0x02] = {0x00, 0x00};  /* {Transaction Command, ACK command} */

__IO uint32_t TimeOut = 0x00;

/* Private function prototypes -----------------------------------------------*/
static void USART_Config(void);


/* Private functions ---------------------------------------------------------*/


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
   
    
/* Here I try to send the AT command to the module */
  debugIt = USART_GetITStatus(USARTx, USART_IT_TXE);
      if (USART_GetITStatus(USARTx, USART_IT_TXE) == SET) //should say == SET
  {   /* Send the data */
    while (TxIndex < TXBUFFERSIZE) {
        USART_SendData(USARTx, TxBuffer[TxIndex]);
        TxIndex++; 
        while ((USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)&&(TimeOut != 0x00))
        {
        }
    }
        if (TxIndex == GetVar_NbrOfData())
        {
          /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
        }
      }
    /*If Data Received send the ACK*/
    else
    {
    }
      /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(USARTx, USART_IT_RXNE) == SET)
  {
    if (UsartMode == USART_MODE_TRANSMITTER)
    {
      AckBuffer[RxIndex++] = USART_ReceiveData(USARTx);
    }
    else
    {
      /* Receive the command */
      if (UsartTransactionType == USART_TRANSACTIONTYPE_CMD)
      {
        CmdBuffer[RxIndex++] = USART_ReceiveData(USARTx);
      }
      /* Receive the USART data */
      else
      {
        RxBuffer[RxIndex++] = USART_ReceiveData(USARTx);
      }
    }
  }  
    
   
  RxBuffer[RxIndex++] = USART_ReceiveData(USARTx);            
}


/**
  * @brief  Configures the USART Peripheral.
  * @param  None
  * @retval None
  */
static void USART_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
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
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);
  
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable USART */
  USART_Cmd(USARTx, ENABLE);
}

uint8_t GetVar_NbrOfData(void)
{
  return CmdBuffer[0x01];
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
