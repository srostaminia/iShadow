#include <plib.h>

#include "../sys_config.h"
#include "../sys_func.h"
#include "../fatfs/ff.h"

static char useAmp;
static char cvdda=1;

//pin define
#define RESP BIT_0
#define INCP BIT_1
#define RESV BIT_2
#define INCV BIT_3
#define INPHI BIT_4

/********************************************************************999999999999999999999999999999999999999999999999999999999999999999999999999999999*/
//MACROS to pulse chip lines

#define SMH_ResP_Pulse {LATESET=RESP;delay_one_tenth_us(DELAY_TIME);LATECLR=RESP;}
	
#define SMH_IncP_Pulse {LATESET=INCP;delay_one_tenth_us(DELAY_TIME);LATECLR=INCP;}

#define SMH_ResV_Pulse {LATESET=RESV;delay_one_tenth_us(DELAY_TIME);LATECLR=RESV;}
	
#define SMH_IncV_Pulse {LATESET=INCV;delay_one_tenth_us(DELAY_TIME);LATECLR=INCV;}

/*********************************************************************/
//MACROS for inphi (delay is inserted between high and low)

#define SMH_InPhi_SetHigh {LATESET=INPHI;}
	
#define SMH_InPhi_SetLow {LATECLR=INPHI;}

/*********************************************************************/
//MACRO to set all pins low

#define SMH_SetAllLow {LATECLR=RESP;LATECLR=INCP;LATECLR=RESV;LATECLR=INCV;LATECLR=INPHI;}

/*********************************************************************/
//SMH System Registers

#define SMH_SYS_COLSEL 0	//select column
#define SMH_SYS_ROWSEL 1	//select row
#define SMH_SYS_VSW 2		//vertical switching
#define SMH_SYS_HSW 3		//horizontal switching
#define SMH_SYS_CONFIG 5	//configuration register
#define SMH_SYS_VREF 4		//voltage reference
#define SMH_SYS_NBIAS 6		//nbias
#define SMH_SYS_AOBIAS 7	//analog out bias

/*********************************************************************/
//default values

// Supply voltage types
// Notation: AVB is A.B volts. e.g. 5V0 is 5V, 3V3 is 3.3V, etc.
#define SMH1_VDD_5V0  1

#define SMH_VREF_5V0 30		//vref for 5 volts
#define SMH_NBIAS_5V0 55	//nbias for 5 volts
#define SMH_AOBIAS_5V0 55	//aobias for 5 volts
#define SMH_GAIN_DEFAULT 0	//no amp gain
#define SMH_SELAMP_DEFAULT 0	//amp bypassed


#define SMH_VREF_4V0 33		//vref for 4.0 volts
#define SMH_NBIAS_4V0 50	//nbias for 4.0 volts
#define SMH_AOBIAS_4V0 47	//aobias for 4.0 volts
#define SMH_GAIN_4V0 2          //amp gain for 3.3 volts
#define SMH_SELAMP_4V0 1


#define SMH_VREF_3V3 41		//vref for 3.3 volts
#define SMH_NBIAS_3V3 50	//nbias for 3.3 volts
#define SMH_AOBIAS_3V3 37//33//	//aobias for 3.3 volts
#define SMH_GAIN_3V3 2//3//          //amp gain for 3.3 volts (2,37)->clean but low constract; (3,33)->diirty but high constract
#define SMH_SELAMP_3V3 1

#define SMH_VREF_3V3_outdoor 48		//vref for 3.3 volts
#define SMH_NBIAS_3V3_outdoor 50	//nbias for 3.3 volts
#define SMH_AOBIAS_3V3_outdoor 42	//aobias for 3.3 volts
#define SMH_GAIN_3V3_outdoor 3          //amp gain for 3.3 volts (2,37)->clean but low constract; (3,33)->diirty but high constract
#define SMH_SELAMP_3V3_outdoor 1

#define SMH_VREF_3V0 46		//vref for 3.0 volts
#define SMH_NBIAS_3V0 50	//nbias for 3.0 volts
#define SMH_AOBIAS_3V0 37	//aobias for 3.0 volts
#define SMH_GAIN_3V0 5          //amp gain for 3.0
#define SMH_SELAMP_3V0 1

#define SMH_VREF_2V8 47		//vref for 2.8 volts
#define SMH_NBIAS_2V8 50	//nbias for 2.8 volts
#define SMH_AOBIAS_2V8 42	//aobias for 2.8 volts
#define SMH_GAIN_2V8 5          //amp gain for 2.8
#define SMH_SELAMP_2V8 1

//for fastest speed
//ADC_PARAM3 ADC_SAMPLE_TIME_2 | ADC_CONV_CLK_PB | ADC_CONV_CLK_Tcy
//DELAY_TIME 2
//delay beofre get data value delay_one_tenth_us(2);
#define SMH_AOBIAS_3V3_F 37//33//	//aobias for 13fps fast adc 33-40
#define SMH_GAIN_3V3_F 2          //amp gain for fast adc 2-3

//medium speed
#define SMH_AOBIAS_3V3_M 37//33//	//aobias for 13fps fast adc 33-40
#define SMH_GAIN_3V3_M 2          //amp gain for fast adc 2-3(2:clean but low constract; 3:diirty but high constract)


#define ROWS 112
#define COLS 112

#define DELAY_TIME 2	//Instruction pulse delay time


/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
//	ArduEyeSMHClass
/*********************************************************************/
/*********************************************************************/

/*********************************************************************/
// Initialize the vision chip for image readout
  
  extern void begin(short vref,short nbias,short aobias,char gain,char selamp);

/*********************************************************************/
// Chip Register and Value Manipulation

  //set pointer on chip
  extern void setPointer(char ptr);

  //set value of current pointer
  extern void setValue(short val);

  //increment value of current pointer
  extern void incValue(short val);

  //pulse INPHI to operate amplifier
  extern void pulseInphi(char delay);

  //clear all register values
  extern void clearValues(void);

  //set pointer to register and set value for that register
  extern void setPointerValue(char ptr,short val);

  //set configuration register on chip
  extern void setConfig(char gain, char selamp,char cvdda);

  //select amp and set amp gain
  extern void setAmpGain(char gain);

  //set analog input to Arduino for onboard ADC
  //void setAnalogInput(char analogInput);

  //set external ADC input
  //void setADCInput(char ADCInput,char state);

  //set hsw and vsw registers to bin on-chip
  extern void setBinning(short hbin,short vbin);

/*********************************************************************/
// Bias functions

  //set individual bias values
  extern void setVREF(short vref);
  extern void setNBIAS(short nbias);
  extern void setAOBIAS(short aobias);
 
  //set biases based on Vdd
  extern void setBiasesVdd(char vddType);

  //set all bias values
  extern void setBiases(short vref,short nbias,short aobias);


/*********************************************************************/
// Image Functions

  //given an image, returns a fixed-pattern noise mask and mask_base
  extern void calcMask(short *img, short size, unsigned char *mask, short *mask_base);

  //applies pre-calculated FPN mask to an image
  extern void applyMask(short *img, short size, unsigned char *mask, short mask_base);

  //gets an image from the vision chip
  extern void getImage(short *img, unsigned char rowstart, unsigned char numrows, unsigned char rowskip, unsigned char colstart, unsigned char numcols, unsigned char colskip);//, char ADCType,char anain);

  //gets a image from the vision chip, sums each row and returns one pixel for the row
  //void getImageRowSum(short *img, unsigned char rowstart, unsigned char 	numrows, unsigned char rowskip, unsigned char colstart, unsigned 	char numcols, unsigned char colskip, char ADCType,char anain);
 
  //gets a image from the vision chip, sums each col and returns one pixel for the col
  //void getImageColSum(short *img, unsigned char rowstart, unsigned char 	numrows, unsigned char rowskip, unsigned char colstart, unsigned 	char numcols, unsigned char colskip, char ADCType,char anain);

  //takes an image and returns the maximum value row and col
  //void findMax(unsigned char rowstart, unsigned char numrows, 	unsigned char rowskip, unsigned char colstart, unsigned char 	numcols, unsigned char colskip, char ADCType,char anain,unsigned 	char *max_row, unsigned char *max_col);

  //prints the entire vision chip over serial as a Matlab array
  extern void chipToMatlab(int image_count);//,char ADCType,char anain);

  //prints a section of the vision chip over serial as a Matlab array
  //void sectionToMatlab(unsigned char rowstart, unsigned char 	numrows, unsigned char rowskip, unsigned char colstart, unsigned 	char numcols, unsigned char colskip, char ADCType, unsigned char 	anain);





