
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "stonyman.h"
#include "..\predict_gaze.h"


/*********************************************************************/
//	begin
//	Initializes the vision chips for normal operation.  Sets vision
//	chip pins to low outputs, clears chip registers, sets biases and
//	config register.  If no parameters are passed in, default values
//	are used.
/*********************************************************************/

void begin(short vref,short nbias,short aobias,char gain,char selamp)
{
  //set all digital pins to output
  mPORTESetPinsDigitalOut(RESP);
  mPORTESetPinsDigitalOut(INCP);
  mPORTESetPinsDigitalOut(RESV);
  mPORTESetPinsDigitalOut(INCV);
  mPORTESetPinsDigitalOut(INPHI);

  //set all pins low
  SMH_SetAllLow;

  //set external ADC SS to high
  //ADC_SS_DDR|=ADC_SS;
  //ADC_SS_PORT|=ADC_SS;

  //clear all chip register values
  clearValues();

  //set up biases
  setBiases(vref,nbias,aobias);

  //short config0=gain+(0*8)+(16);
  //setPointerValue(SMH_SYS_CONFIG,config0);

  short config=gain+(selamp*8)+(16);
  
  //turn chip on with config value
  setPointerValue(SMH_SYS_CONFIG,config);
  //setPointerValue(SMH_SYS_CONFIG,0); //turn the cam aplifier off(save power)
  
  //if amp is used, set useAmp variable
  if(selamp==1)
   useAmp=1;
  else
   useAmp=0;
  //////////////////////////////
  //setPointerValue(SMH_SYS_ROWSEL,56);
  //setPointerValue(SMH_SYS_COLSEL,56);

}

/*********************************************************************/
//	setPointer
//	Sets the pointer system register to the desired value
/*********************************************************************/

inline void setPointer(char ptr)
{
  // clear pointer
	SMH_ResP_Pulse; // macro

  // increment pointer to desired value
  short i;
  for (i=0; i!=ptr; ++i)
    SMH_IncP_Pulse; // macro
}

/*********************************************************************/
//	setValue
//	Sets the value of the current register
/*********************************************************************/

inline void setValue(short val)
{
  // clear pointer
  SMH_ResV_Pulse; // macro

  // increment pointer
  short i;
  for (i=0; i!=val; ++i)
    SMH_IncV_Pulse; // macro
}

/*********************************************************************/
//	incValue
//	Sets the pointer system register to the desired value.  Value is
//	not reset so the current value must be taken into account
/*********************************************************************/

void incValue(short val)
{
  short i;
  for (i=0; i<val; ++i) //increment pointer
    SMH_IncV_Pulse;
}

/*********************************************************************/
//	pulseInphi
//	Operates the amplifier.  Sets inphi pin high, delays to allow
//	value time to settle, and then brings it low.
/*********************************************************************/

void pulseInphi(char delay)
{
  SMH_InPhi_SetHigh;
  delay_one_tenth_us(delay);
  SMH_InPhi_SetLow;
}

/*********************************************************************/
//	setPointerValue
//	Sets the pointer to a register and sets the value of that        //	register
/*********************************************************************/

inline void setPointerValue(char ptr,short val)
{
    setPointer(ptr);	//set pointer to register
    setValue(val);	//set value of that register
}

/*********************************************************************/
//	clearValues
//	Resets the value of all registers to zero
/*********************************************************************/

void clearValues(void)
{
	char i;
    for (i=0; i!=8; ++i)
    	setPointerValue(i,0);	//set each register to zero
}

/*********************************************************************/
//	setVREF
//	Sets the VREF register value (0-63)
/*********************************************************************/

void  setVREF(short vref)
{
  setPointerValue(SMH_SYS_VREF,vref);
}

/*********************************************************************/
//	setNBIAS
//	Sets the NBIAS register value (0-63)
/*********************************************************************/

void  setNBIAS(short nbias)
{
  setPointerValue(SMH_SYS_NBIAS,nbias);
}

/*********************************************************************/
//	setAOBIAS
//	Sets the AOBIAS register value (0-63)
/*********************************************************************/

void  setAOBIAS(short aobias)
{
  setPointerValue(SMH_SYS_AOBIAS,aobias);
}

/*********************************************************************/
//	setBiasesVdd
//	Sets biases based on chip voltage
/*********************************************************************/

void setBiasesVdd(char vddType)
{
  
  // determine biases. Only one option for now.
  switch (vddType) 
  {
    case SMH1_VDD_5V0:	//chip receives 5V
    default:
      setPointerValue(SMH_SYS_NBIAS,SMH_NBIAS_5V0);
	setPointerValue(SMH_SYS_AOBIAS,SMH_AOBIAS_5V0);
	setPointerValue(SMH_SYS_VREF,SMH_VREF_5V0);
    break;
  }
}

/*********************************************************************/
//	setBiases
//	Sets all three biases
/*********************************************************************/

void setBiases(short vref,short nbias,short aobias)
{
   	setPointerValue(SMH_SYS_NBIAS,nbias);
	setPointerValue(SMH_SYS_AOBIAS,aobias);
	setPointerValue(SMH_SYS_VREF,vref);
}

/*********************************************************************/
//	setConfig
//	Sets configuration register
//	cvdda:  (1) connect vdda, always should be connected
//	selamp: (0) bypasses amplifier, (1) connects it
//	gain: amplifier gain 1-7
//	EXAMPLE 1: To configure the chip to bypass the amplifier:
//	setConfig(0,0,1);
//	EXAMPLE 2: To use the amplifier and set the gain to 4:
//	setConfig(4,1,1);
/*********************************************************************/

void setConfig(char gain, char selamp, char cvdda)
{
   short config=gain+(selamp*8)+(cvdda*16);	//form register value

   if(selamp==1)	//if selamp is 1, set useAmp variable to 1 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     useAmp=1;
   else 
     useAmp=0;
  
   // Note that config will have the following form binary form:
   // 000csggg where c=cvdda, s=selamp, ggg=gain (3 bits)
   // Note that there is no overflow detection in the input values.
   setPointerValue(SMH_SYS_CONFIG,config);
}

/*********************************************************************/
//	setAmpGain
//	A friendlier version of setConfig.  If amplifier gain is set to 
//	zero, amplifier is bypassed.  Otherwise the appropriate amplifier
//	gain (range 1-7) is set.
/*********************************************************************/

void setAmpGain(char gain)
{
   short config;

   if((gain>0)&&(gain<8))	//if gain is a proper value, connect amp
   {
    config=gain+8+16;	//gain+(selamp=1)+(cvdda=1)
    useAmp=1;	//using amplifier
   }
   else	//if gain is zero or outside range, bypass amp
   {
    config=16;	//(cvdda=1)
    useAmp=0;	//bypassing amplifier
   }

   setPointerValue(SMH_SYS_CONFIG,config);	//set config register
}

/*********************************************************************/
//	setAnalogInput
//	Sets the analog pin for one vision chip to be an input.
//	This is for the Arduino onboard ADC, not an external ADC
/********************************************************************

void setAnalogInput(char analogInput)
{
 switch (analogInput)
 {
    case 0:
      ANALOG_DDR&=~ANALOG0;	//set chip 0 as analog input
      break;
    case 1:
      ANALOG_DDR&=~ANALOG1;
      break;
    case 2:
      ANALOG_DDR&=~ANALOG2;
      break;
    case 3:
      ANALOG_DDR&=~ANALOG3;
      break;
  }
}*/

/*********************************************************************/
//	setADCInput
//	Sets the analog pin to be a digital output and select a chip
//	to connect to the external ADC.  The state can be used to
//	deselect a particular chip as well.
/********************************************************************

void setADCInput(char ADCInput,char state)
{
  
  // make sure A# pin is set to be a digital output
  switch (ADCInput){
    case 0:
      ANALOG_DDR |=ANALOG0; // set A0 to output
      if (state)
        ANALOG_PORT |= ANALOG0; // turn on
      else
        ANALOG_PORT &= ~ANALOG0; // turn off
      break;
    case 1:
      ANALOG_DDR |= ANALOG1; // set A1 to output
      if (state)
        ANALOG_PORT |= ANALOG1; // turn on
      else
        ANALOG_PORT &= ~ANALOG1; // turn off
      break;
    case 2:
      ANALOG_DDR  |= ANALOG2; // set A2 to output
      if (state)
        ANALOG_PORT |= ANALOG2; // turn on
      else
        ANALOG_PORT &= ~ANALOG2; // turn off
      break;
    case 3:
      ANALOG_DDR |= ANALOG3; // set A3 to output
      if (state)
        ANALOG_PORT |= ANALOG3; // turn on
      else
        ANALOG_PORT &= ~ANALOG3; // turn off
      break;
  }
}*/

/*********************************************************************/
//	setBinning
//	Configures binning in the focal plane using the VSW and HSW
//	system registers. The super pixels are aligned with the top left //	of the image, e.g. "offset downsampling" is not used. This 
//	function is for the Stonyman chip only. 
//	VARIABLES:
//	hbin: set to 1, 2, 4, or 8 to bin horizontally by that amount
//	vbin: set to 1, 2, 4, or 8 to bin vertically by that amount
/*********************************************************************/

void setBinning(short hbin,short vbin)
{
   short hsw,vsw;

   switch (hbin) //horizontal binning
   {
    case 2:		//downsample by 2
      hsw = 0xAA;
      break;
    case 4:		//downsample by 4
      hsw = 0xEE;
      break;
    case 8:		//downsample by 8
      hsw = 0xFE;
      break;
    default:	//no binning
      hsw = 0x00;
   }

   switch (vbin) 	//vertical binning
   {
    case 2:		//downsample by 2
      vsw = 0xAA;
      break;
    case 4:		//downsample by 4
      vsw = 0xEE;
      break;
    case 8:		//downsample by 8
      vsw = 0xFE;
      break;
    default:	//no binning
      vsw = 0x00;
    }

  //set switching registers
  setPointerValue(SMH_SYS_HSW,hsw);
  setPointerValue(SMH_SYS_VSW,vsw);
}

/*********************************************************************/
//	calcMask
//	Expose the vision chip to uniform texture (such as a white piece
//	of paper placed over the optics).  Take an image using the 
//	getImage function.  Pass the short "img" array and the "size"
//	number of pixels, along with a unsigned char "mask" array to hold
//	the FPN mask and mask_base for the FPN mask base.  Function will
//	populate the mask array and mask_base variable with the FPN mask,
//	which can then be used with the applMask function. 
/*********************************************************************/

void calcMask(short *img, short size, unsigned char 	*mask,short *mask_base)
{
 	*mask_base = 10000; // e.g. "high"

 	int i;
      for (i=0; i<size; ++i)
        if (img[i]<(*mask_base))	//find the min value for mask_base
          *mask_base = img[i];

      // generate calibration mask
      for (i=0; i<size; ++i)
        mask[i] = img[i] - *mask_base;	//subtract min value for mask
}

/*********************************************************************/
//	applyMask
//	given the "mask" and "mask_base" variables calculated in        //	calcMask, and a current image, this function will subtract the
//	mask to provide a calibrated image.
/*********************************************************************/

void applyMask(short *img, short size, unsigned char 	*mask, short mask_base)
{
	 // Subtract calibration mask
	int i;
  	for (i=0; i<size;++i)
	{
    		img[i] -= mask_base+mask[i];  //subtract FPN mask
    		img[i]=-img[i];          //negate image so it displays 						//	properly
 	 }
}

/*********************************************************************/
//	getImage
//	This function acquires a box section of a Stonyman or Hawksbill 
//	and saves to image array img.  Note that images are read out in //	raster manner (e.g. row wise) and stored as such in a 1D array. //	In this case the pointer img points to the output array. 
//
//	VARIABLES: 
//	img (output): pointer to image array, an array of signed shorts
//	rowstart: first row to acquire
//	numrows: number of rows to acquire
//	rowskip: skipping between rows (useful if binning is used)
//	colstart: first column to acquire
//	numcols: number of columns to acquire
//	colskip: skipping between columns
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): which analog input to use
//	
//	EXAMPLES:
//	getImage(img,16,8,1,24,8,1,SMH1_ADCTYPE_ONBOARD,0): 
//	Grab an 8x8 window of pixels at raw resolution starting at row 
//	16, column 24, from chip using onboard ADC at input 0
//	getImage(img,0,14,8,0,14,8,SMH1_ADCTYPE_MCP3201,2): 
//	Grab entire Stonyman chip when using
//	8x8 binning. Grab from input 2.
/*********************************************************************/

void getImage(short *img, unsigned char rowstart, unsigned char numrows, unsigned char rowskip, unsigned char colstart, unsigned char numcols, unsigned char colskip)//, char ADCType,char anain)
{
  short *pimg = img; // pointer to output image array
  short val;
  unsigned char chigh,clow;
  unsigned char row,col;
  
/*  if(ADCType==SMH1_ADCTYPE_ONBOARD)	//if using onboard ADC
     setAnalogInput(anain);		//set analog input to Arduino
  else if(ADCType==SMH1_ADCTYPE_MCP3201_2)
  { 
     setAnalogInput(anain);
     ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }
  else	//if using external ADC
  {
    setADCInput(anain,1); // enable chip
    ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }*/

  // Go to first row
  setPointerValue(SMH_SYS_ROWSEL,rowstart);
 
  // Loop through all rows
  for (row=0; row<numrows; ++row) {
    
    // Go to first column
    setPointerValue(SMH_SYS_COLSEL,colstart);
    
    // Loop through all columns
    for (col=0; col<numcols; ++col) {
      
      // settling delay
      delay_one_tenth_us(DELAY_TIME);

      // pulse amplifier if needed
	if (useAmp) 
        pulseInphi(10);
      
      // get data value
      delay_one_tenth_us(DELAY_TIME);
      
      // get pixel value from ADC
/*      switch (ADCType)
      {
        case SMH1_ADCTYPE_ONBOARD:	//onboard Arduino ADC
           val = analogRead(anain); // acquire pixel
	    break;
        case SMH1_ADCTYPE_MCP3001:  // Micrchip 10 bit
           ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<5;
           val += (clow&0xF8)>>3;
           ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        case SMH1_ADCTYPE_MCP3201:  // Microchip 12 bit
        case SMH1_ADCTYPE_MCP3201_2:
	     ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<7;
           val += (clow&0xFE)>>1;
	     ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        default:
           val = 555;
          break;
      }*/
      
      *pimg = val; // store pixel
      pimg++; // advance pointer
      incValue(colskip); // go to next column
    }
    setPointer(SMH_SYS_ROWSEL);
    incValue(rowskip); // go to next row
  }

  /*if((ADCType!=SMH1_ADCTYPE_ONBOARD)&&(ADCType!=SMH1_ADCTYPE_MCP3201_2))
   setADCInput(anain,0); // disable chip*/

}

/*********************************************************************/
//	getImageRowSum
//	This function acquires a box section of a Stonyman or Hawksbill 
//	and saves to image array img.  However, each row of the image
//	is summed and returned as a single value.
//	Note that images are read out in 
//	raster manner (e.g. row wise) and stored as such in a 1D array. 
//	In this case the pointer img points to the output array. 
//
//	VARIABLES: 
//	img (output): pointer to image array, an array of signed shorts
//	rowstart: first row to acquire
//	numrows: number of rows to acquire
//	rowskip: skipping between rows (useful if binning is used)
//	colstart: first column to acquire
//	numcols: number of columns to acquire
//	colskip: skipping between columns
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): which analog input to use
//	
//	EXAMPLES:
//	getImage(img,16,8,1,24,8,1,SMH1_ADCTYPE_ONBOARD,0): 
//	Grab an 8x8 window of pixels at raw resolution starting at row 
//	16, column 24, from chip using onboard ADC at input 0
//	getImage(img,0,14,8,0,14,8,SMH1_ADCTYPE_MCP3201,2): 
//	Grab entire Stonyman chip when using
//	8x8 binning. Grab from input 2.
/********************************************************************

void getImageRowSum(short *img, unsigned char rowstart, unsigned char numrows, unsigned char rowskip, unsigned char colstart, unsigned char numcols, unsigned char colskip, char ADCType,char anain)
{
  short *pimg = img; // pointer to output image array
  short val,total=0;
  unsigned char chigh,clow;
  unsigned char row,col;
  
  if(ADCType==SMH1_ADCTYPE_ONBOARD)	//if using onboard ADC
     setAnalogInput(anain);		//set analog input to Arduino
  else if(ADCType==SMH1_ADCTYPE_MCP3201_2)
  { 
     setAnalogInput(anain);
     ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }
  else	//if using external ADC
  {
    setADCInput(anain,1); // enable chip
    ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }

  // Go to first row
  setPointerValue(SMH_SYS_ROWSEL,rowstart);
 
  // Loop through all rows
  for (row=0; row<numrows; ++row) {
    
    // Go to first column
    setPointerValue(SMH_SYS_COLSEL,colstart);
  
    total=0;
    
    // Loop through all columns
    for (col=0; col<numcols; ++col) {
      
      // settling delay
      __delay_cycles(SYS_CLK_MHZ);

      // pulse amplifier if needed
	if (useAmp) 
        pulseInphi(2);
      
      // get data value
      __delay_cycles(SYS_CLK_MHZ);
      
      // get pixel value from ADC
      switch (ADCType) 
      {
        case SMH1_ADCTYPE_ONBOARD:	//onboard Arduino ADC
           val = analogRead(anain); // acquire pixel
	    break;
        case SMH1_ADCTYPE_MCP3001:  // Micrchip 10 bit
           ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<5;
           val += (clow&0xF8)>>3;
           ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        case SMH1_ADCTYPE_MCP3201:  // Microchip 12 bit
        case SMH1_ADCTYPE_MCP3201_2:
	     ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<7;
           val += (clow&0xFE)>>1;
	     ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        default:
           val = 555;
          break;
      }
      
      total+=val;	//sum values along row
      incValue(colskip); // go to next column
    }
	
    *pimg = total>>4; // store pixel divide to avoid overflow
    pimg++; // advance pointer

    setPointer(SMH_SYS_ROWSEL);
    incValue(rowskip); // go to next row
  }

  if((ADCType!=SMH1_ADCTYPE_ONBOARD)&&(ADCType!=SMH1_ADCTYPE_MCP3201_2))
   setADCInput(anain,0); // disable chip

}*/

/*********************************************************************/
//	getImageColSum
//	This function acquires a box section of a Stonyman or Hawksbill 
//	and saves to image array img.  However, each col of the image
//	is summed and returned as a single value.
//	Note that images are read out in 
//	raster manner (e.g. row wise) and stored as such in a 1D array. 
//	In this case the pointer img points to the output array. 
//
//	VARIABLES: 
//	img (output): pointer to image array, an array of signed shorts
//	rowstart: first row to acquire
//	numrows: number of rows to acquire
//	rowskip: skipping between rows (useful if binning is used)
//	colstart: first column to acquire
//	numcols: number of columns to acquire
//	colskip: skipping between columns
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): which analog input to use
//	
//	EXAMPLES:
//	getImage(img,16,8,1,24,8,1,SMH1_ADCTYPE_ONBOARD,0): 
//	Grab an 8x8 window of pixels at raw resolution starting at row 
//	16, column 24, from chip using onboard ADC at input 0
//	getImage(img,0,14,8,0,14,8,SMH1_ADCTYPE_MCP3201,2): 
//	Grab entire Stonyman chip when using
//	8x8 binning. Grab from input 2.
/********************************************************************

void getImageColSum(short *img, unsigned char rowstart, 					unsigned char numrows, unsigned char 					rowskip, unsigned char colstart, unsigned 					char numcols, unsigned char colskip, char 					ADCType,char anain)
{
  short *pimg = img; // pointer to output image array
  short val,total=0;
  unsigned char chigh,clow;
  unsigned char row,col;
  
  if(ADCType==SMH1_ADCTYPE_ONBOARD)	//if using onboard ADC
     setAnalogInput(anain);		//set analog input to Arduino
  else if(ADCType==SMH1_ADCTYPE_MCP3201_2)
  { 
     setAnalogInput(anain);
     ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }
  else	//if using external ADC
  {
    setADCInput(anain,1); // enable chip
    ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }

  // Go to first col
  setPointerValue(SMH_SYS_COLSEL,colstart);
 
  // Loop through all cols
  for (col=0; col<numcols; ++col) {
    
    // Go to first row
    setPointerValue(SMH_SYS_ROWSEL,rowstart);
  
    total=0;
    
    // Loop through all rows
    for (row=0; row<numrows; ++row) {
      
      // settling delay
      __delay_cycles(SYS_CLK_MHZ);

      // pulse amplifier if needed
	if (useAmp) 
        pulseInphi(2);
      
      // get data value
      __delay_cycles(SYS_CLK_MHZ);
      
      // get pixel value from ADC
      switch (ADCType) 
      {
        case SMH1_ADCTYPE_ONBOARD:	//onboard Arduino ADC
           val = analogRead(anain); // acquire pixel
	    break;
        case SMH1_ADCTYPE_MCP3001:  // Micrchip 10 bit
           ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<5;
           val += (clow&0xF8)>>3;
           ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        case SMH1_ADCTYPE_MCP3201:  // Microchip 12 bit
        case SMH1_ADCTYPE_MCP3201_2:
	     ADC_SS_PORT &= ~ADC_SS;  // turn SS low to start conversion
           chigh=SPI.transfer(0);   // get high byte
           clow=SPI.transfer(0);    // get low byte
           val = ((short)(chigh&0x1F))<<7;
           val += (clow&0xFE)>>1;
	     ADC_SS_PORT |= ADC_SS;   // SS high to stop
          break;
        default:
           val = 555;
          break;
      }
      
      total+=val;	//sum value along column
      incValue(rowskip); // go to next row
    }
	
    *pimg = total>>4; // store pixel
    pimg++; // advance pointer

    setPointer(SMH_SYS_COLSEL);
    incValue(colskip); // go to next col
  }

  if((ADCType!=SMH1_ADCTYPE_ONBOARD)&&(ADCType!=SMH1_ADCTYPE_MCP3201_2))
   setADCInput(anain,0); // disable chip

}*/


/*********************************************************************/
//	findMax
//	Searches over a block section of a Stonyman or Hawksbill chip
//	to find the brightest pixel. This function is intended to be used //	for things like finding the location of a pinhole in response to //	a bright light.
//
//	VARIABLES: 
//	rowstart: first row to search
//	numrows: number of rows to search
//	rowskip: skipping between rows (useful if binning is used)
//	colstart: first column to search
//	numcols: number of columns to search
//	colskip: skipping between columns
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): which analog input to use
//	rowwinner: (output) pointer to variable to write row of brightest //	pixel
//	colwinner: (output) pointer to variable to write column of 
//	brightest pixel
//
//	EXAMPLE:
//	FindMaxSlow(8,104,1,8,104,1,SMH1_ADCTYPE_ONBOARD,0,&rowwinner,
//	&colwinner): 
//	Search rows 8...104 and columns 8...104 for brightest pixel, with 
//	onboard ADC, chip 0
/********************************************************************

void findMax(unsigned char rowstart, unsigned char 					numrows, unsigned char rowskip, unsigned 					char colstart, unsigned char numcols, 					unsigned char colskip, char ADCType,char 					anain,unsigned char *max_row, unsigned 					char *max_col)
{
  unsigned short maxval=5000,minval=0,val;
  unsigned char row,col,bestrow,bestcol;
  unsigned char chigh,clow;

  
  if(ADCType==SMH1_ADCTYPE_ONBOARD)	//if using onboard ADC
     setAnalogInput(anain);	//set Arduino analog input
  else
  {
    setADCInput(anain,1); // enable chip
    ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }

  // Go to first row
  setPointerValue(SMH_SYS_ROWSEL,rowstart);

  // Loop through all rows
  for (row=0; row<numrows; ++row) {

    // Go to first column
    setPointerValue(SMH_SYS_COLSEL,colstart);

    // Loop through all columns
    for (col=0; col<numcols; ++col) {

      // settling delay
      __delay_cycles(SYS_CLK_MHZ);

      // pulse amplifier if needed
	if (useAmp) 
        pulseInphi(2);
      
      // get data value
      __delay_cycles(SYS_CLK_MHZ);
      
      // get pixel from ADC
      switch (ADCType) {
        case SMH1_ADCTYPE_ONBOARD:	//onboard Arduino ADC
           val = analogRead(anain); // acquire pixel
	    break;
        case SMH1_ADCTYPE_MCP3001: // Micrchip 10 bit
           ADC_SS_PORT &= ~ADC_SS; // turn SS low to start conversion
           chigh=SPI.transfer(0); // get high byte
           clow=SPI.transfer(0); // get low byte
           val = ((short)(chigh&0x1F))<<5;
           val += (clow&0xF8)>>3;
           ADC_SS_PORT |= ADC_SS; // SS high to stop
          break;
        case SMH1_ADCTYPE_MCP3201: // Microchip 12 bit
	     ADC_SS_PORT &= ~ADC_SS; // turn SS low to start conversion
           chigh=SPI.transfer(0); // get high byte
           clow=SPI.transfer(0); // get low byte
           val = ((short)(chigh&0x1F))<<7;
           val += (clow&0xFE)>>1;
	     ADC_SS_PORT |= ADC_SS; // SS high to stop
          break;
        default:
           val = 555;
          break;
      }

	if(useAmp)	//amplifier is inverted
	{
		if (val>minval) 	//find max val (bright)
		{
       	 bestrow=row;
       	 bestcol=col;
      	 minval=val;
      	}
	}
	else		//unamplified
	{
      	if (val<maxval) 	//find min val (bright)
		{
        	 bestrow=row;
        	 bestcol=col;
        	 maxval=val;
      	}
	}

      incValue(colskip); // go to next column
    }
    setPointer(SMH_SYS_ROWSEL);
    incValue(rowskip); // go to next row
  }

  if(ADCType!=SMH1_ADCTYPE_ONBOARD)
   setADCInput(anain,0); // disable chip

  // Optionally we can comment out these next three items
  //Serial.print("bestrow = "); Serial.println((short)bestrow);
  //Serial.print("bestcol = "); Serial.println((short)bestcol);
  //Serial.print("maxval = "); Serial.println((short)maxval);

  *max_row = bestrow;
  *max_col = bestcol;
}*/

/*********************************************************************/
//	chipToMatlab
//	This function dumps the entire contents of a Stonyman or 
//	Hawksbill chip to the Serial monitor in a form that may be copied
//	into Matlab. The image is written be stored in matrix Img.
//
//	VARIABLES: 
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): Selects one analog input
/*********************************************************************/
extern char row_count;
extern char col_count;
extern int subsample_index;
extern char is_subsampling;
extern unsigned short val[ROWS*COLS];
extern FIL fil;
extern UINT bw;

void chipToMatlab(int image_count)//,char ADCType, char anain)
{
  unsigned char row,col;
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /*
  // Result code
  FRESULT rc;
  // File system object
  //FATFS fatfs;
  // File object
  FIL fil;
  UINT bw;

  //Board_ledOn(LED1);                                     //turn on red LED to show we are accessing the SD card

  // Register volume work area (never fails)
  //f_mount(0, &fatfs);

  char *filename;
  char buffer[30];
  sprintf(buffer, "%d", image_count);
  filename=buffer;

  // Open file
  rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
  if (rc)
  {
      Nop();	// Debug
  }
  */

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  row_count=0;
  setPointerValue(SMH_SYS_ROWSEL,0); // set row = 0
  delay_one_tenth_us(2000);
  //setPointerValue(SMH_SYS_COLSEL,0);
  for (row=0; row<ROWS; ++row)
  {
    setPointerValue(SMH_SYS_COLSEL,0); // set column = 0
    //setPointerValue(SMH_SYS_ROWSEL,0);
    col_count=0;
    for (col=0; col<COLS; ++col)
    {
      // settling delay
      delay_one_tenth_us(DELAY_TIME);
      // pulse amplifier if needed
	    if (useAmp) 
            pulseInphi(DELAY_TIME);
      
      // get data value
      #ifdef FAST_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      //while(col!=col_count)
      //{
      //}
      mPORTGToggleBits(BIT_15);
      adc_start();
      while(col==col_count)
      {

      }      
      #endif

      #ifdef MEDIUM_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      //while(col!=col_count)
      //{
      //}
      mPORTGSetBits(BIT_15);
      adc_start();
      while(col==col_count)
      {

      }
      #endif


      #ifdef QUALITY_MODE
      delay_one_tenth_us(20);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      //mPORTGSetBits(BIT_15);
      val[row*COLS+col]=analogRead();
      //mPORTGClearBits(BIT_15);      
      #endif
      
      // increment column
      SMH_IncV_Pulse;//incValue(1);
    }
    setPointer(SMH_SYS_ROWSEL); // point to row
    //setPointer(SMH_SYS_COLSEL);
    SMH_IncV_Pulse;//incValue(1); // increment row
    row_count++;
  }

  //rc = f_close(&fil);
  //Board_ledOff(LED1);

}

 extern unsigned short mask[NUM_SUBSAMPLE][2];
void subsample_capture(unsigned short mask_off[NUM_SUBSAMPLE][2])//,char ADCType, char anain)
{
  unsigned short lastRow,lastCol;
  int i, j;

  setPointerValue(SMH_SYS_ROWSEL, mask[0][0]); // initialize row
  setPointerValue(SMH_SYS_COLSEL,0); // set column = 0
  
  lastRow = mask[0][0];
  lastCol = 0;

  subsample_index = 0;
  is_subsampling = 1;

  delay_one_tenth_us(2000);
  //setPointerValue(SMH_SYS_COLSEL,0);
  for (i = 0; i < NUM_SUBSAMPLE; i++)
  {
      if (mask[i][0] != lastRow)
      {
          setPointer(SMH_SYS_ROWSEL);
          char diff = mask[i][0] - lastRow;
          for (j = 0; j < diff; j++)
          {
              SMH_IncV_Pulse;
          }
          lastRow = mask[i][0];

          setPointerValue(SMH_SYS_COLSEL,mask[i][1]);
          lastCol = mask[i][1];
      } else {
          for (j = 0; j < mask[i][1] - lastCol; j++)
          {
              SMH_IncV_Pulse;
          }
          lastCol = mask[i][1];
      }
    //setPointerValue(SMH_SYS_ROWSEL,0);

      // settling delay
      delay_one_tenth_us(DELAY_TIME);
      // pulse amplifier if needed
	    if (useAmp)
            pulseInphi(DELAY_TIME);

      // get data value
      #ifdef FAST_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      //while(col!=col_count)
      //{
      //}
      mPORTGToggleBits(BIT_15);
      adc_start();
      while(col==col_count)
      {

      }
      #endif

      #ifdef MEDIUM_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      //while(col!=col_count)
      //{
      //}
      mPORTGSetBits(BIT_15);
      adc_start();
      while(subsample_index <= i)
      {

      }
      #endif


      #ifdef QUALITY_MODE
      delay_one_tenth_us(20);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      //mPORTGSetBits(BIT_15);
      val[row*COLS+col]=analogRead();
      //mPORTGClearBits(BIT_15);
      #endif
  }

  is_subsampling = 0;
  //rc = f_close(&fil);
  //Board_ledOff(LED1);

}

/*********************************************************************/
//	sectionToMatlab
//	This function dumps a box section of a Stonyman or Hawksbill 
//	to the Serial monitor in a form that may be copied into Matlab.
//	The image is written to be stored in matrix Img.
//
//	VARIABLES: 
//	rowstart: first row to acquire
//	numrows: number of rows to acquire
//	rowskip: skipping between rows (useful if binning is used)
//	colstart: first column to acquire
//	numcols: number of columns to acquire
//	colskip: skipping between columns
//	ADCType: which ADC to use, defined ADC_TYPES
//	anain (0,1,2,3): which analog input to use
//
//	EXAMPLES:
//	sectionToMatlab(16,8,1,24,8,1,SMH1_ADCTYPE_ONBOARD,0): 
//	Grab an 8x8 window of pixels at raw resolution starting at row 
//	16, column 24, from onboard ADC at chip 0
//	sectionToMatlab(0,14,8,0,14,8,SMH1_ADCTYPE_ONBOARD,2): 
//	Grab entire Stonyman chip when using 8x8 binning. Grab from input
//	2.
/********************************************************************

void sectionToMatlab(unsigned char rowstart, unsigned char numrows, unsigned char rowskip, unsigned char colstart, unsigned char numcols, unsigned char colskip, char ADCType, unsigned char anain)
{
  short val;
  unsigned char row,col;
  unsigned char clow,chigh;

  if(ADCType==SMH1_ADCTYPE_ONBOARD)	//if onboard ADC
     setAnalogInput(anain);
  else
  {
    setADCInput(anain,1); // enable chip
    ADC_SS_PORT |= ADC_SS; // make sure SS is high
  }

  Serial.println("Img = [");
  setPointerValue(SMH_SYS_ROWSEL,rowstart);

  for (row=0; row<numrows; row++) {

    setPointerValue(SMH_SYS_COLSEL,colstart);

    for (col=0; col<numcols; col++) {
      // settling delay
      __delay_cycles(SYS_CLK_MHZ);

      // pulse amplifier if needed
      if (useAmp) 
        pulseInphi(2);
      
	__delay_cycles(SYS_CLK_MHZ);

      // get pixel from ADC
      switch (ADCType) {
        case SMH1_ADCTYPE_ONBOARD:
           val = analogRead(anain); // acquire pixel
	    break;
        case SMH1_ADCTYPE_MCP3001: // Micrchip 10 bit
           ADC_SS_PORT &= ~ADC_SS; // turn SS low to start conversion
           chigh=SPI.transfer(0); // get high byte
           clow=SPI.transfer(0); // get low byte
           val = ((short)(chigh&0x1F))<<5;
           val += (clow&0xF8)>>3;
           ADC_SS_PORT |= ADC_SS; // SS high to stop
          break;
        case SMH1_ADCTYPE_MCP3201: // Microchip 12 bit
	     ADC_SS_PORT &= ~ADC_SS; // turn SS low to start conversion
           chigh=SPI.transfer(0); // get high byte
           clow=SPI.transfer(0); // get low byte
           val = ((short)(chigh&0x1F))<<7;
           val += (clow&0xFE)>>1;
	     ADC_SS_PORT |= ADC_SS; // SS high to stop
          break;
        default:
           val = 555;
          break;
      }

      incValue(colskip);
      Serial.print(val);
      Serial.print(" ");
    }
    setPointer(SMH_SYS_ROWSEL);
    incValue(rowskip); // go to next row
    Serial.println(" ");
  }
  Serial.println("];");

  if(ADCType!=SMH1_ADCTYPE_ONBOARD)
    setADCInput(anain,0); // disable chip

}*/

extern int adc_data;
extern volatile BYTE rtcHour;
extern volatile BYTE rtcMin;
extern volatile BYTE rtcSec;
extern volatile int minSec;
extern long pixel_count;
void single_pixel(char row, char col)
{
   char *message;
   char buffer[10];
   int start_time=0, current_time=0;
   int pixel=0;
   pixel_count=0;

   int number_of_data=1000;
   unsigned short single_pixel_val[number_of_data];

  setPointerValue(SMH_SYS_ROWSEL,row);
  setPointerValue(SMH_SYS_COLSEL,col);
  // settling delay
  delay_one_tenth_us(DELAY_TIME);
  // pulse amplifier if needed
  //if (useAmp)
  //{
      //pulseInphi(DELAY_TIME);
  //}

  WriteString("start!");
  current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
  sprintf(buffer, "Start Time=+%d\r\n",current_time);
  message=buffer;
  WriteString(message);
  int i=0;
  for(i=0;i<112*112*number_of_data;i++)
  {
      // settling delay
      //delay_one_tenth_us(DELAY_TIME);
      // pulse amplifier if needed
      if (useAmp)
            pulseInphi(DELAY_TIME);

      // get data value
      #ifdef FAST_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      mPORTGToggleBits(BIT_15);
      adc_start();
      #endif

      #ifdef MEDIUM_MODE
      delay_one_tenth_us(DELAY_TIME);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      adc_start();
      while(pixel==pixel_count)
      {

      }
      pixel=pixel_count;
      if(pixel%(112*112)==0)
      {
          sprintf(buffer, "%d\r\n", adc_data);
          message=buffer;
          WriteString(message);
          single_pixel_val[(i+1)/(112*112)]=adc_data;
      }
      #endif


      #ifdef QUALITY_MODE
      delay_one_tenth_us(20);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      //mPORTGSetBits(BIT_15);
      val[row*COLS+col]=analogRead();
      //mPORTGClearBits(BIT_15);
      #endif
      //delay_one_tenth_us(100000);//10ms
  }
  current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
  sprintf(buffer, "%d,End Time=+%d\r\n",pixel_count,current_time);
  message=buffer;
  WriteString(message);
  f_write(&fil, single_pixel_val, 2*number_of_data, &bw);
}

void pixel_binning(char hsw, char vsw)
{
   char *message;
   char buffer[10];
   int start_time=0, current_time=0;
   int pixel=0;
   pixel_count=0;

  // 
  setPointerValue(SMH_SYS_HSW,hsw);
  setPointerValue(SMH_SYS_VSW,vsw);
  
  //calculate respect row and col;
  //to be filled in fomula
  char cols=1;
  char rows=1;
  unsigned short pixel_val_binning[cols*rows];
  
  // settling delay
  delay_one_tenth_us(DELAY_TIME);

  char row, col;
  setPointerValue(SMH_SYS_ROWSEL,0); // set row = 0
  delay_one_tenth_us(2000);
  for (row=0; row<rows; ++row)
  {
    setPointerValue(SMH_SYS_COLSEL,0); // set column = 0
    for (col=0; col<cols; ++col)
    {
        // pulse amplifier if needed
        if (useAmp)
        {
            pulseInphi(DELAY_TIME);
        }
    

      // get data value
      #ifdef FAST_MODE
      delay_one_tenth_us(2);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      mPORTGToggleBits(BIT_15);
      adc_start();
      #endif

      #ifdef MEDIUM_MODE
      delay_one_tenth_us(DELAY_TIME);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      // get pixel from ADC
      adc_start();
      while(pixel==pixel_count)
      {

      }
      pixel=pixel_count;

      sprintf(buffer, "%d\r\n", adc_data);
      message=buffer;
      WriteString(message);
      pixel_val_binning[cols*row+col]=adc_data;
      #endif


      #ifdef QUALITY_MODE
      delay_one_tenth_us(20);//DELAY_TIME);1us too smaller, 1.5us start get noisyy
      //mPORTGSetBits(BIT_15);
      val[row*COLS+col]=analogRead();
      //mPORTGClearBits(BIT_15);
      #endif
      //delay_one_tenth_us(500000);//100ms
      }
  }
  f_write(&fil, pixel_val_binning, 2*rows*cols, &bw);
}