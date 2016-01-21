#ifndef __STONYMAN_H
#define __STONYMAN_H

#include "stm32l1xx.h"
#include "ishadow_conf.h"
#include "stdbool.h"

// #if defined(CIDER_TRACKING) && defined(OUTDOOR_SWITCH)
// 	#error ERROR: CANNOT USE CIDER_TRACKING AND OUTDOOR_SWITCH SIMULTANEOUSLY (STONYMAN.H)
// #endif

#if (defined(CIDER_TRACKING) && defined(ANN_TRACKING))
	#error ERROR: CANNOT USE ANN_TRACKING AND CIDER_TRACKING SIMULTANEOUSLY (STONYMAN.H)
#endif

#if defined(USB_SEND) && defined(SD_SEND)
	#error ERROR: CANNOT USE USB_SEND AND SD_SEND SIMULTANEOUSLY (STONYMAN.H)
#endif

#if !defined(USB_SEND) && !defined(SD_SEND)
	#warning WARNING: NO STORAGE METHOD SELECTED, CAMERA DATA WILL NOT BE SAVED (STONYMAN.H)
  #define TX_PIXELS       112
	#define BUFFER_HALF			TX_PIXELS
#else
	#define SEND_DATA
#endif

#if defined(USB_SEND)
	#if !defined(USB_8BIT) && !defined(USB_16BIT)
		#error ERROR: USB PIXEL TX RATE NOT SELECTED - CHOOSE USB_8BIT OR USB_16BIT (STONYMAN.H)
	#elif !defined(EYE_VIDEO_ON) && !defined(OUT_VIDEO_ON)
		#error ERROR: MUST SELECT EYE_VIDEO AND / OR OUT_VIDEO FOR STREAMING VIA USB (STONYMAN.H)
	#endif
#endif

#if defined(EYE_VIDEO_ON) == defined(EYE_VIDEO_OFF)
	#error ERROR: MUST SELECT EYE_VIDEO_ON OR EYE_VIDEO_OFF (STONYMAN.H)
#endif

#if defined(OUT_VIDEO_ON) == defined(OUT_VIDEO_OFF)
	#error ERROR: MUST SELECT OUT_VIDEO_ON OR OUT_VIDEO_OFF (STONYMAN.H)
#endif

#if defined(CIDER_TRACKING) || defined(ANN_TRACKING)
	#define EYE_TRACKING_ON
#endif

// If eye camera is being recorded at all, it is primary
#if defined(OUT_VIDEO_ON) && !defined(EYE_VIDEO_ON)
	#define OUT_CAM_PRIMARY
#else
	#define EYE_CAM_PRIMARY
#endif

#if defined(ANN_TRACKING) || defined(CIDER_TRACKING)
	#ifndef USE_PARAM_FILE
		#error ERROR: MUST ENABLE USE_PARAM_FILE IF DOING EYE TRACKING (STONYMAN.H)
	#endif

	#define EYE_TRACKING_ON

	#ifdef EYE_CAM_PRIMARY
		#define IMPLICIT_EYE_TRACKING
	#endif
#endif // defined(ANN_TRACKING) || defined(CIDER_TRACKING)

#if defined(OUT_VIDEO_ON) && defined(EYE_VIDEO_ON)
	#define N_IMG_FRAME		2
#elif defined(OUT_VIDEO_ON) || defined(EYE_VIDEO_ON)
	#define N_IMG_FRAME		1
#else
	#define N_IMG_FRAME 	0
#endif

// CIDER overrides (don't touch)
#ifdef EYE_TRACKING_ON

	#if !defined(COLUMN_COLLECT)
		#define COLUMN_COLLECT
	#endif

	#if !defined(USE_PARAM_FILE)
		#define USE_PARAM_FILE
	#endif

#endif  // EYE_TRACKING_ON

#ifdef OUTDOOR_SWITCH
	#define OUTDOOR_THRESH          300
#endif

#define LED_LOW         0

//#define LED_HIGH        0        
#define LED_HIGH          0x59E       // 1.15V
//#define LED_HIGH        0x5D1         // 1.25V
//#define LED_HIGH        0x64D
//#define LED_HIGH        0x746

#ifdef SD_SEND

	#define SD_ROWS         32
	#define TX_PIXELS				SD_ROWS * 112
	#define BUFFER_HALF			SD_ROWS * 112

	//#define ECAM_OFFSET     5376
	#define SD_BLOCKS       (SD_ROWS * 112 * 4) / 512
	#define SD_EOF_OFFSET		((112 * 112 * 2) / 512) * N_IMG_FRAME

	#if (112 % SD_ROWS != 0)

		#define SD_MOD_OFFSET (112 % SD_ROWS) * 112
		#define SD_MOD_BLOCKS ((112 % SD_ROWS) * 112 * 4 ) / 512

		#if ((112 % SD_ROWS) * 112 * 4 ) % 512 != 0
			#error ERROR: SD_ROWS INVALID, DOES NOT ALIGN TO 512B BOUNDARY
		#endif
	#endif // (112 % SD_ROWS != 0)

#endif // SD_SEND

#ifdef USB_SEND

	#if defined(USB_16BIT) && defined(USB_8BIT)
		#error ERROR: CANNOT DEFINE BOTH USB_16BIT AND USB_8BIT (STONYMAN.H)
	#elif !defined(USB_16BIT) && !defined(USB_8BIT)
		#error ERROR: MUST DEFINE ONE OF USB_16BIT OR USB_8BIT (STONYMAN.H)
	#endif

	#ifdef USB_16BIT
		#define USB_PIXELS      			92
		#define BUFFER_HALF						92

		#define USB16_FINAL_PAD				(12544 % 92)
	#else
		#define USB_PIXELS      			112
		#define BUFFER_HALF						224
	#endif

	#define TX_PIXELS					USB_PIXELS

	#define USB_PACKET_SIZE		PACKET_SIZE

#endif // USB_SEND

#if defined(USB_SEND) && defined(USB_8BIT)
	#define DO_8BIT_CONV

	#define RESIZE_PIXEL(X) 			(((X) >> 2) & 0xFF)
	#define CAST_TX_BUFFER(X)			((uint8_t *)(X))
#else
	#define RESIZE_PIXEL(X) 			(X)
	#define CAST_TX_BUFFER(X)			((uint16_t *)(X))
#endif

#define DHR12R1_OFFSET      ((uint32_t)0x00000008)
#define DHR12R2_OFFSET      ((uint32_t)0x00000014)

#define CAM_AHB							RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC

#define OCAM_RESP_BANK      GPIOB
#define OCAM_RESP_PIN       GPIO_Pin_0
#define OCAM_INCP_BANK      GPIOB
#define OCAM_INCP_PIN       GPIO_Pin_1
#define OCAM_RESV_BANK      GPIOB
#define OCAM_RESV_PIN       GPIO_Pin_2
#define OCAM_INCV_BANK      GPIOB
#define OCAM_INCV_PIN       GPIO_Pin_3
#define OCAM_INPH_BANK      GPIOB
#define OCAM_INPH_PIN       GPIO_Pin_4
#define OCAM_AN_BANK        GPIOA
#define OCAM_AN_PIN         GPIO_Pin_2
#define OCAM_ADC_CHAN       ADC_Channel_2

#define ECAM_RESP_BANK      GPIOC
#define ECAM_RESP_PIN       GPIO_Pin_0
#define ECAM_INCP_BANK      GPIOC
#define ECAM_INCP_PIN       GPIO_Pin_1
#define ECAM_RESV_BANK      GPIOC
#define ECAM_RESV_PIN       GPIO_Pin_2
#define ECAM_INCV_BANK      GPIOC
#define ECAM_INCV_PIN       GPIO_Pin_3
#define ECAM_INPH_BANK      GPIOC
#define ECAM_INPH_PIN       GPIO_Pin_4
#define ECAM_AN_BANK        GPIOA
#define ECAM_AN_PIN         GPIO_Pin_3
#define ECAM_ADC_CHAN       ADC_Channel_3

#define OUT_CAM             0
#define EYE_CAM             1

#ifdef OUT_CAM_PRIMARY
	#define PRIMARY_CAM						 OUT_CAM
	#define SECONDARY_CAM					 EYE_CAM

	#define PRIMARY_PARAM(PNAME)   OCAM ## _ ## PNAME
	#define SECONDARY_PARAM(PNAME) ECAM ## _ ## PNAME

	#define PRIMARY_FPN_START			 OUT_FPN_START
	#define SECONDARY_FPN_START		 EYE_FPN_START
#elif defined(EYE_CAM_PRIMARY)
	#define PRIMARY_CAM						 EYE_CAM
	#define SECONDARY_CAM					 OUT_CAM

	#define PRIMARY_PARAM(PNAME)   ECAM ## _ ## PNAME
	#define SECONDARY_PARAM(PNAME) OCAM ## _ ## PNAME

	#define PRIMARY_FPN_START			 EYE_FPN_START
	#define SECONDARY_FPN_START		 OUT_FPN_START	
#endif

#define REG_COLSEL      0  //select column
#define REG_ROWSEL      1  //select row
#define REG_VSW         2  //vertical switching
#define REG_HSW         3  //horizontal switching
#define REG_VREF        4  //voltage reference
#define REG_CONFIG      5  //configuration register
#define REG_NBIAS       6  //nbias
#define REG_AOBIAS      7  //analog out bias
#define NUM_REG         8  

#define SMH_VREF_3V3    41     //vref for 3.3 volt
#define SMH_NBIAS_3V3   50     //nbias for 3.3 volts
#define SMH_AOBIAS_3V3  37     //aobias for 3.3 volts
#define SMH_GAIN_3V3    2//3   //amp gain for 3.3 volts 
#define SMH_SELAMP_3V3  1

#define ADC1_DR_ADDRESS                 ((uint32_t)0x40012458)
#define DMA_DIR_PeripheralToMemory      ((uint32_t)0x00000000)

#define SEL_MAJOR_LINE	0					
#define SEL_MINOR_LINE	1

// TODO: Switch this to ROW_COLLECT, column should be default from now on.
#ifdef COLUMN_COLLECT

	#define MAJOR_REG			REG_COLSEL
	#define MINOR_REG			REG_ROWSEL

	#define SEL_COL				SEL_MAJOR_LINE
	#define SEL_ROW				SEL_MINOR_LINE

	#ifdef USE_PARAM_FILE
		#define FPN_OFFSET 		0
		#define FPN_T_OFFSET	(112 * 112)
	#endif

#else

	#define MAJOR_REG			REG_ROWSEL
	#define MINOR_REG			REG_COLSEL

	#define SEL_COL				SEL_MINOR_LINE
	#define SEL_ROW				SEL_MAJOR_LINE

	#ifdef USE_PARAM_FILE
		#define FPN_OFFSET 		(112 * 112)
		#define FPN_T_OFFSET	0
	#endif

#endif // COLUMN_COLLECT

// Frame data (FD) packet offsets
#define FD_TIMER_OFFSET		0
#define FD_MODEL_OFFSET		4
#define FD_PREDX_OFFSET		5
#define FD_PREDY_OFFSET		6
#define FD_CIDCOL_OFFSET	7
#define FD_CIDROW_OFFSET 	8
#define FD_CIDRAD_OFFSET	9

#define FD_NOMODEL_LENGTH 5
#define FD_ANN_LENGTH			7
#define FD_CIDER_LENGTH		10
#define FD_MAX_LENGTH			10

#ifdef USE_PARAM_FILE

	#define EYE_FPN_START		0
	#define OUT_FPN_START		(112 * 112 * 2)  
	#define PARAMS_START		(112 * 112 * 4)

	#define FPN_PRI(X, Y)		((uint16_t*)(model_data))[PRIMARY_FPN_START + FPN_OFFSET + (((X) * 112) + (Y))]
	#define FPN_T_PRI(X, Y)	((uint16_t*)(model_data))[PRIMARY_FPN_START + FPN_T_OFFSET + (((X) * 112) + (Y))]
	#define FPN_SEC(X, Y)		((uint16_t*)(model_data))[SECONDARY_FPN_START + FPN_OFFSET + (((X) * 112) + (Y))]
	#define FPN_T_SEC(X, Y)	((uint16_t*)(model_data))[SECONDARY_FPN_START + FPN_T_OFFSET + (((X) * 112) + (Y))]

	#define FPN_EYE(X, Y)		((uint16_t*)(model_data))[EYE_FPN_START + FPN_OFFSET + (((X) * 112) + (Y))]
	#define FPN_T_EYE(X, Y)	((uint16_t*)(model_data))[EYE_FPN_START + FPN_T_OFFSET + (((X) * 112) + (Y))]
	#define FPN_OUT(X, Y)		((uint16_t*)(model_data))[OUT_FPN_START + FPN_OFFSET + (((X) * 112) + (Y))]
	#define FPN_T_OUT(X, Y)	((uint16_t*)(model_data))[OUT_FPN_START + FPN_T_OFFSET + (((X) * 112) + (Y))]

#else

	#define FPN_PRI(X, Y)		0
	#define FPN_T_PRI(X, Y)	0
	#define FPN_SEC(X, Y)		0
	#define FPN_T_SEC(X, Y)	0

	#define FPN_EYE(X, Y)		0
	#define FPN_T_EYE(X, Y)	0
	#define FPN_OUT(X, Y)		0
	#define FPN_T_OUT(X, Y)	0

#endif // USE_PARAM_FILE

#define PARAM_NOMODEL			0
#define PARAM_CIDER_HIT		1
#define PARAM_CIDER_MISS  2
#define PARAM_ANN					3

void stony_init_default(void);
void stony_init(short vref, short nbias, short aobias, char gain, char selamp);
// void stony_reset(uint16_t wait_time, char gain, char selamp);

void config_adc_select(uint8_t cam);
void config_adc_default();

int stony_single();
int stony_dual();

void mark_cider_packet(bool cider_failed);

#ifdef IMPLICIT_EYE_TRACKING
int stony_single2(bool do_tracking);
int stony_dual2(bool do_tracking);
#endif

#if defined(EYE_TRACKING_ON) && defined(SD_SEND)
int stony_ann();
#endif

int stony_cider_line(uint8_t rowcol_num, uint16_t *line_buf, uint8_t rowcol_sel);

#endif // __STONYMAN_H