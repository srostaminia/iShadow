#include <plib.h>					// Peripheral Library
#include "../sys_config.h"
#include <stdint.h>          /* For UINT32 definition                         */
#include <stdbool.h>         /* For true/false definition                     */

/*********************************************************************
 * ADC function set
 ********************************************************************/
//ADC initialize
void adc_init()
{
  //ANO as adc input
  PORTSetPinsAnalogIn(IOPORT_B, BIT_0);

  // configure and enable the ADC
  CloseADC10();// ensure the ADC is off before setting the configuration

  // configure to sample AN0
  SetChanADC10( ADC_PARAM_INPUT);

  // configure ADC and enable it
  OpenADC10( ADC_PARAM1, ADC_PARAM2, ADC_PARAM3, ADC_PARAM4, ADC_PARAM5 );

  //ADC interrupt
  ConfigIntADC10(ADC_PARAM_INT);

  //
  mAD1ClearIntFlag();

  // Now enable the ADC logic
  EnableADC10();
}

//start ADC sampling and converting
void adc_start()
{
    AcquireADC10();
}

//
int analogRead()
{
	int analogValue;
        uint8_t channelNumber=0;

	/* Ensure that the pin associated with the analog channel is in analog
	** input mode, and select the channel in the input mux.
	*/
	AD1PCFG = ~(1 << channelNumber);
	AD1CHS = (channelNumber & 0xFFFF) << 16;
	AD1CON1	=	0; //Ends sampling, and starts converting

	//Set up for manual sampling
	AD1CSSL	=	0;
	AD1CON3	=	0x0002;	//Tad = internal 6 Tpb
	AD1CON2	=	0x0000;////select AVDD, AVSS

	//Turn on ADC
	AD1CON1SET	=	0x8000;

	//Start sampling
	AD1CON1SET	=	0x0002;

	//Delay for a bit
	delay_one_tenth_us(20);//2us->7.3fps 1.5us->7.6fps 1us->8fps(start noisy)

	//Start conversion
	AD1CON1CLR	=	0x0002;

	//Wait for conversion to finish
	while (!(AD1CON1 & 0x0001));


	//Read the ADC Value
	analogValue	=	ADC1BUF0;

	return (analogValue);
}
