/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <plib.h>            /* Include to use PIC32 peripheral libraries     */
#include <stdint.h>          /* For UINT32 definition                         */
#include <stdbool.h>         /* For true/false definition                     */
#include "sys_func.h"        /* variables/params used by system.cpp           */



/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.cpp                                         */
/*                                                                            */
/******************************************************************************/

/*********************************************************************
 * Function:  		void sys_init(void)
 * PreCondition:
 * Input:           None
 * Output:          None
 * Side Effects:
 * Overview:       initialize system
 * Note:
 ********************************************************************/
void sys_init(void)
{
    // Enable optimal performance
    SYSTEMConfigPerformance(GetSystemClock());
    mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks(80 MHz Peripheral clock for 80 MHz system clock)

    // Set Timer 1 to 1000 Hz ticks (for 80 MHz system clock)
    T1CONbits.TCKPS = 1; // 1:8 prescale(timer frecuncy=10MHz)
    PR1 = 10000; // Base(timer counting period), timer finish every 1ms
    TMR1 = 0; // Clear counter
    
    IPC1bits.T1IP = 6;	// Interrupt priority 6 (low)
    IFS0CLR = _IFS0_T1IF_MASK; // Disable interrupt flag
    IEC0SET = _IEC0_T1IE_MASK; // Enable timer interrupt
    T1CONbits.TON = 1; // Start timer

    //ADC interrupt set
    mAD1IntEnable(1);

    //
    UART_init();

    //Initialize Disk
    disk_initialize(0);

    //enable all interrupt
    INTEnableSystemMultiVectoredInt();

    //ADC Initialize
    #ifdef FAST_MODE
    adc_init();
    #endif

    #ifdef MEDIUM_MODE
    adc_init();
    #endif
}


/*********************************************************************
 * Function:  		DWORD get_fattime(void)
 * PreCondition:
 * Input:           None
 * Output:          Time
 * Side Effects:
 * Overview:        when writing fatfs requires a time stamp
 *					in this exmaple we are going to use a counter
 *					If the starter kit has the 32kHz crystal
 *					installed then the RTCC could be used instead
 * Note:
 ********************************************************************/
DWORD get_fattime(void)
{
	DWORD tmr;

	INTDisableInterrupts();
	tmr =    (((DWORD)rtcYear - 80) << 25)
			| ((DWORD)rtcMon << 21)
			| ((DWORD)rtcMday << 16)
			| (WORD)(rtcHour << 11)
			| (WORD)(rtcMin << 5)
			| (WORD)(rtcSec >> 1);
	INTEnableInterrupts();

	return tmr;
}


/*********************************************************************
 * Function:  		DWORD get_fattime(void)
 * PreCondition:
 * Input:           number of 0.1us to delay
 * Output:          None
 * Side Effects:
 * Overview:        delay one_tenth_us*0.1us
 *                     e.g: delay_one_tenth_us(10), delay 10us
 * Note:
 ********************************************************************/

void delay_one_tenth_us(UINT32 one_tenth_us)
{
 DWORD start = _CP0_GET_COUNT();
 DWORD end = start + GetSystemClock() / 10000000 / 2 * one_tenth_us;
 if (end > start) while (_CP0_GET_COUNT() < end);
 else while (_CP0_GET_COUNT() > start || _CP0_GET_COUNT() < end);
}
