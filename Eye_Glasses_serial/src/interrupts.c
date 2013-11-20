/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#ifdef __XC32
    #include <xc.h>          /* Defines special funciton registers, CP0 regs  */
#endif

#include <plib.h>            /* Include to use PIC32 peripheral libraries     */
#include <sys/attribs.h>     /* For __ISR definition                          */
#include <stdint.h>          /* For uint32_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */
#include "camera/stonyman.h" //to include ROWS and COLS
#include "sys_config.h"


/******************************************************************************/
/* Interrupt Vector Options                                                   */
/******************************************************************************/
/*                                                                            */
/* VECTOR NAMES:                                                              */
/*                                                                            */
/* _CORE_TIMER_VECTOR          _COMPARATOR_2_VECTOR                           */
/* _CORE_SOFTWARE_0_VECTOR     _UART_2A_VECTOR                                */
/* _CORE_SOFTWARE_1_VECTOR     _I2C_2A_VECTOR                                 */
/* _EXTERNAL_0_VECTOR          _SPI_2_VECTOR                                  */
/* _TIMER_1_VECTOR             _SPI_2A_VECTOR                                 */
/* _INPUT_CAPTURE_1_VECTOR     _I2C_4_VECTOR                                  */
/* _OUTPUT_COMPARE_1_VECTOR    _UART_3_VECTOR                                 */
/* _EXTERNAL_1_VECTOR          _UART_2_VECTOR                                 */
/* _TIMER_2_VECTOR             _SPI_3A_VECTOR                                 */
/* _INPUT_CAPTURE_2_VECTOR     _I2C_3A_VECTOR                                 */
/* _OUTPUT_COMPARE_2_VECTOR    _UART_3A_VECTOR                                */
/* _EXTERNAL_2_VECTOR          _SPI_4_VECTOR                                  */
/* _TIMER_3_VECTOR             _I2C_5_VECTOR                                  */
/* _INPUT_CAPTURE_3_VECTOR     _I2C_2_VECTOR                                  */
/* _OUTPUT_COMPARE_3_VECTOR    _FAIL_SAFE_MONITOR_VECTOR                      */
/* _EXTERNAL_3_VECTOR          _RTCC_VECTOR                                   */
/* _TIMER_4_VECTOR             _DMA_0_VECTOR                                  */
/* _INPUT_CAPTURE_4_VECTOR     _DMA_1_VECTOR                                  */
/* _OUTPUT_COMPARE_4_VECTOR    _DMA_2_VECTOR                                  */
/* _EXTERNAL_4_VECTOR          _DMA_3_VECTOR                                  */
/* _TIMER_5_VECTOR             _DMA_4_VECTOR                                  */
/* _INPUT_CAPTURE_5_VECTOR     _DMA_5_VECTOR                                  */
/* _OUTPUT_COMPARE_5_VECTOR    _DMA_6_VECTOR                                  */
/* _SPI_1_VECTOR               _DMA_7_VECTOR                                  */
/* _I2C_3_VECTOR               _FCE_VECTOR                                    */
/* _UART_1A_VECTOR             _USB_1_VECTOR                                  */
/* _UART_1_VECTOR              _CAN_1_VECTOR                                  */
/* _SPI_1A_VECTOR              _CAN_2_VECTOR                                  */
/* _I2C_1A_VECTOR              _ETH_VECTOR                                    */
/* _SPI_3_VECTOR               _UART_4_VECTOR                                 */
/* _I2C_1_VECTOR               _UART_1B_VECTOR                                */
/* _CHANGE_NOTICE_VECTOR       _UART_6_VECTOR                                 */
/* _ADC_VECTOR                 _UART_2B_VECTOR                                */
/* _PMP_VECTOR                 _UART_5_VECTOR                                 */
/* _COMPARATOR_1_VECTOR        _UART_3B_VECTOR                                */
/*                                                                            */
/* Refer to the device specific .h file in the C32/XC32 Compiler              */
/* pic32mx\include\proc directory for a complete Vector and IRQ mnemonic      */
/* listings for the PIC32 device.                                             */
/*                                                                            */
/* PRIORITY OPTIONS:                                                          */
/*                                                                            */
/* (default) IPL0AUTO, IPL1, IPL2, ... IPL7 (highest)                         */
/*                                                                            */
/* Example Shorthand Syntax                                                   */
/*                                                                            */
/* void __ISR(<Vector Name>,<PRIORITY>) user_interrupt_routine_name(void)     */
/* {                                                                          */
/*     <Clear Interrupt Flag>                                                 */
/* }                                                                          */
/*                                                                            */
/* For more interrupt macro examples refer to the C compiler User Guide in    */
/* the C compiler /doc directory.                                             */
/*                                                                            */
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/*****************************************************************************
 * Function:  		void CoreTimerHandler(void)
 * PreCondition:
 * Input:           None
 * Output:          None
 * Side Effects:
 * Overview:        FatFs requires a 1ms tick timer to aid
 *					with low level function timing
 * Note:            Initial Microchip version adapted to work into ISR routine
 * every tick = 1ms
 *****************************************************************************/
extern volatile BYTE rtcYear, rtcMon, rtcMday;	// RTC starting values
extern volatile BYTE rtcHour, rtcMin, rtcSec;
extern volatile int minSec;
extern volatile unsigned long tick;	// Used for time benchmarking purposes

void __attribute((interrupt(ipl6), vector(_TIMER_1_VECTOR), nomips16)) CoreTimerHandler(void)
{
	static const BYTE dom[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	//static int div1k;
	BYTE n;

	// clear the interrupt flag
	IFS0CLR = _IFS0_T1IF_MASK;

	disk_timerproc();	// call the low level disk IO timer functions
	tick++;				// increment the benchmarking timer
        minSec++;

	// implement a 'fake' RTCC
	//if (++div1k >= 1000) {
		//div1k = 0;
	// implement a 'fake' RTCC
	if (minSec >= 1000) {
		minSec = 0;
		if (++rtcSec >= 60) {
			rtcSec = 0;
			if (++rtcMin >= 60) {
				rtcMin = 0;
				if (++rtcHour >= 24) {
					rtcHour = 0;
					n = dom[rtcMon - 1];
					if ((n == 28) && !(rtcYear & 3)) n++;
					if (++rtcMday > n) {
						rtcMday = 1;
						if (++rtcMon > 12) {
							rtcMon = 1;
							rtcYear++;
						}
					}
				}
			}
		}
	}

}


/*****************************************************************************
 * Function:  		void adc_get_data(void)
 * PreCondition:
 * Input:           None
 * Output:          None
 * Side Effects:
 * Overview:        get data from adc buffer
 * Note:
 *****************************************************************************/
extern char row_count;
extern char col_count;
extern int subsample_index;
extern char is_subsampling;
extern int adc_data;
extern unsigned short val[ROWS*COLS];
extern long pixel_count;

void __ISR(_ADC_VECTOR,ipl7) adc_get_data(void)
{
    //mPORTGClearBits(BIT_15);
    adc_data=ReadADC10(0);

    if (is_subsampling) {
        val[subsample_index] = adc_data;
        subsample_index++;
    } else {
        pixel_count++;
        val[row_count*COLS+col_count]=adc_data;
        col_count++;
    }
    

    mAD1ClearIntFlag();
}


/*****************************************************************************
 * Function:  		void adc_get_data(void)
 * PreCondition:
 * Input:           None
 * Output:          None
 * Side Effects:
 * Overview:        receive uart data
 * Note:
 *****************************************************************************/
extern char uart_buffer[1024];
extern int command_check(char * buf);
// UART 1 interrupt handler
// it is set at priority level 2
void __ISR(_UART1_VECTOR, ipl2) IntUART1Handler(void)
{
	// Is this an RX interrupt?
	if(INTGetFlag(INT_SOURCE_UART_RX(UART1)))
	{
            // Echo what we just received.
            UINT8 temp_r;
            int num_r=0;
            //PutCharacter(uart_buffer);
            //WriteString(uart_buffer);
            
            WriteString("ACK:");
            
            while(num_r<1024)
            {
                //while(!UARTReceivedDataIsAvailable(UART1));
                temp_r = UARTGetDataByte(UART1);
                if(temp_r == '\0')
                {
                    break;
                }
                else
                {
                    if(temp_r=='\r'||temp_r=='\n')
                    {
                        uart_buffer[num_r]='\r';
                        num_r++;
                        PutCharacter('\r');
                        uart_buffer[num_r]='\n';
                        num_r++;
                        PutCharacter('\n');
                        break;
                    }
                    else
                    {
                        PutCharacter(temp_r);
                        uart_buffer[num_r]=temp_r;
                        num_r++;
                    }
                       
                }
            }
            // Clear the RX interrupt Flag
	    INTClearFlag(INT_SOURCE_UART_RX(UART1));
	}
        command_check(uart_buffer);

	// We don't care about TX interrupt
	if ( INTGetFlag(INT_SOURCE_UART_TX(UART1)) )
	{
		INTClearFlag(INT_SOURCE_UART_TX(UART1));
	}
}
