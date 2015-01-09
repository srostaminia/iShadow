#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

#include "stm32l1xx.h"

/**
  * @brief  SD FLASH SDIO Interface
  */ 
#define SD_DETECT_PIN                    GPIO_Pin_7                  /* PC.07 */
#define SD_DETECT_EXTI_LINE              EXTI_Line7
#define SD_DETECT_EXTI_PIN_SOURCE        EXTI_PinSource7

#define SD_DETECT_GPIO_PORT              GPIOC                       /* GPIOC */
#define SD_DETECT_GPIO_CLK               RCC_AHBPeriph_GPIOC
#define SD_DETECT_EXTI_PORT_SOURCE       EXTI_PortSourceGPIOC
#define SD_DETECT_EXTI_IRQn              EXTI9_5_IRQn


#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40012C80)
/** 
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0x76)
/** 
  * @brief  SDIO Data Transfer Frequency (24MHz max) 
  */
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x1)


#define SD_SDIO_DMA                      DMA2
#define SD_SDIO_DMA_CLK                  RCC_AHBPeriph_DMA2
#define SD_SDIO_DMA_CHANNEL              DMA2_Channel4
#define SD_SDIO_DMA_FLAG_TC              DMA2_FLAG_TC4
#define SD_SDIO_DMA_FLAG_TE              DMA2_FLAG_TE4
#define SD_SDIO_DMA_FLAG_HT              DMA2_FLAG_HT4
#define SD_SDIO_DMA_FLAG_GL              DMA2_FLAG_GL4
#define SD_SDIO_DMA_IRQn                 DMA2_Channel4_IRQn
#define SD_SDIO_DMA_IRQHANDLER           DMA2_Channel4_IRQHandler
/**
  * @}
  */ 

void SD_LowLevel_DeInit(void);
void SD_LowLevel_Init(void);
void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize);
void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize);


#endif // __LOWLEVEL_H

