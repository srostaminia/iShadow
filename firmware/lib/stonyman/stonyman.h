#ifndef __STONYMAN_H
#define __STONYMAN_H

#include "stm32l1xx.h"
#include "stonyman_conf.h"

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
#endif

#if defined(USB_SEND) && !defined(USB_8BIT) && !defined(USB_16BIT)
	#error ERROR: USB PIXEL TX RATE NOT SELECTED - CHOOSE USB_8BIT OR USB_16BIT (STONYMAN.H)
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
// FIXME: will this cause a problem with eye tracking when eye video is not being recorded?
#if defined(OUT_VIDEO_ON) && !defined(EYE_VIDEO_ON)
	#define OUT_CAM_PRIMARY
#else
	#define EYE_CAM_PRIMARY
#endif

#if defined(ANN_TRACKING) || defined(CIDER_TRACKING)
	#define EYE_TRACKING_ON

	#ifdef EYE_CAM_PRIMARY
		#define IMPLICIT_EYE_TRACKING
	#endif
#endif // defined(ANN_TRACKING) || defined(CIDER_TRACKING)

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

	#if (112 % SD_ROWS != 0)

		#define SD_MOD_OFFSET (112 % SD_ROWS) * 112
		#define SD_MOD_BLOCKS ((112 % SD_ROWS) * 112 * 4 ) / 512

		#if ((112 % SD_ROWS) * 112 * 4 ) % 512 != 0
			#error ERROR: SD_ROWS INVALID, DOES NOT ALIGN TO 512B BOUNDARY
		#endif

	#endif // (112 % SD_ROWS != 0)

#endif // SD_SEND

#ifdef USB_SEND

	#define FRAME_DATA_LENGTH			10

	#if defined(USB_16BIT) && defined(USB_8BIT)
		#error ERROR: CANNOT DEFINE BOTH USB_16BIT AND USB_8BIT (STONYMAN.H)
	#elif !defined(USB_16BIT) && !defined(USB_8BIT)
		#error ERROR: MUST DEFINE ONE OF USB_16BIT OR USB_8BIT (STONYMAN.H)
	#endif

	#ifdef USB_16BIT
		#define USB_PIXELS      			92
		#define BUFFER_HALF						92
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
	#define CAST_PIXEL_BUFFER(X)	((uint8_t *)(X))
#else
	#define RESIZE_PIXEL(X) 			(X)
	#define CAST_PIXEL_BUFFER(X)	((uint16_t *)(X))
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

// TODO: Figure out how to make primary / secondary camera switching play nicely with all the FPN options...
// BINGO! Just have a fixed order and fill in unused masks with zeros! :D :D :D
// We'll need to start storing metadata in the binary mask / param files so we can figure out what they contain after the fact, just FYI. Ugh.

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

#define SEL_ROW              1
#define SEL_COL              0

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


// TODO: Switch this to ROW_COLLECT, column should be default from now on.
#ifdef COLUMN_COLLECT

	#define MAJOR_REG			REG_COLSEL
	#define MINOR_REG			REG_ROWSEL

	#ifdef USE_PARAM_FILE
		#define FPN_OFFSET 		0
		#define FPN_T_OFFSET	(112 * 112)
	#endif

#else

	#define MAJOR_REG			REG_ROWSEL
	#define MINOR_REG			REG_COLSEL

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

// CIDER parameters
#ifdef CIDER_TRACKING

	#define SPEC_THRESH     150
	#define CONV_OFFSET     4
	// Percentile value for cross model pixel clamping
	#define CIDER_PERCENTILE        10

	// #define BH(X)           *((float*)(model_data + bh_offset + ((X) * 2)))  
	// #define BO(X)           *((float*)(model_data + bo_offset + ((X) * 2)))
	// #define MASK(X, Y)      model_data[mask_offset + ((X) * 2) + (Y)]
	// #define WHO(X, Y)       *((float*)(model_data + who_offset + ((X) * 4) + ((Y) * 2)))
	// #define WIH(X, Y)       *((float*)(model_data + wih_offset + ((X) * num_hidden * 2) + ((Y) * 2)))
	// #define ROW_FPN(X)          model_data[fpn_offset + (X)]
	// #define COL_FPN(X)      model_data[col_fpn_offset + (X)]

#endif  // CIDER_TRACKING

#define PARAM_NOMODEL			0
#define PARAM_CIDER_HIT		1
#define PARAM_CIDER_MISS  2
#define PARAM_ANN					3

void pulse_resv(uint8_t cam);
void pulse_incv(uint8_t cam);
void pulse_resp(uint8_t cam);
void pulse_incp(uint8_t cam);
void pulse_inph(unsigned short time, uint8_t cam);
void clear_values(uint8_t cam);
void set_pointer_value(char ptr, short val, uint8_t cam);
void inc_pointer_value(char ptr, short val, uint8_t cam);
void set_pointer(char ptr, uint8_t cam);
void set_value(short val, uint8_t cam);
void inc_value(short val, uint8_t cam);
void set_biases(short vref, short nbias, short aobias, uint8_t cam);

void usb_finish_tx(uint8_t *frame_data, uint8_t packet_length);

void stony_init_default(void);
void stony_init(short vref, short nbias, short aobias, char gain, char selamp);
void stony_reset(uint16_t wait_time, char gain, char selamp);
void stony_pin_config();
void adc_dma_init();
void dac_init();

int stony_single();
int stony_dual();

#if defined(EYE_TRACKING_ON) && defined(SD_SEND)
int stony_ann();
#endif

#endif // __STONYMAN_H