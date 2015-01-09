/* 
 * File:   sys_variables.h
 * Author: Boyan
 *
 * Created on November 12, 2012, 11:50 PM
 */

#ifndef SYS_VARIABLES_H
#define	SYS_VARIABLES_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "camera/stonyman.h"

/******************************************************************************/
/* System Level variables                                                     */
/******************************************************************************/

volatile BYTE rtcYear = 0, rtcMon = 0, rtcMday = 0;	// RTC starting values
volatile BYTE rtcHour = 0, rtcMin = 0, rtcSec = 0;
volatile int minSec;
volatile unsigned long tick;	// Used for time benchmarking purposes
int start_signal=0;
int num_frames=0;
int file_set=1;//the name of image data set

//uart receive buffer
char uart_buffer[1024];

//Stonyman variables
char row_count=0;
char col_count=0;
int subsample_index=0;
char is_subsampling=0;
int adc_data=0;
unsigned short val[ROWS*COLS];
long pixel_count=0;


#ifdef	__cplusplus
}
#endif

#endif	/* SYS_VARIABLES_H */

