/* 
 * File:   sys_func.h
 * Author: Boyan
 *
 * Created on November 12, 2012, 11:25 PM
 */

#ifndef SYS_FUNC_H
#define	SYS_FUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "sys_config.h"


/******************************************************************************/
/* System Level variables                                                     */
/******************************************************************************/

extern volatile BYTE rtcYear, rtcMon, rtcMday;	// RTC starting values
extern volatile BYTE rtcHour, rtcMin, rtcSec;
extern volatile unsigned long tick;	// Used for time benchmarking purposes

/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.cpp                                         */
/*                                                                            */
/******************************************************************************/

//system function set
void sys_init(void);
DWORD get_fattime(void);
void delay_one_tenth_us(UINT32 one_tenth_us);

//ADC funtion set
//void adc_init();
//void adc_start();
//int analogRead();


#ifdef	__cplusplus
}
#endif

#endif	/* SYS_FUNC_H */

