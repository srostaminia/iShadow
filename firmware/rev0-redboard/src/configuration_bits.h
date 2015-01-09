/* 
 * File:   configuration_bits.h
 * Author: Boyan
 *
 * Created on November 13, 2012, 11:44 AM
 */

/******************************************************************************/
/* Configuration Bits
/******************************************************************************/

    #pragma config FPLLMUL  = MUL_20        // PLL Multiplier 20, 2
    #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
    #pragma config FPLLODIV = DIV_1         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = HS            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
    #pragma config DEBUG    = ON            // Background Debugger Enable
    #pragma config WDTPS    = PS64          // Watchdog Timer Postscaler (1:32)
    #pragma config FWDTEN   = OFF            // Watchdog Timer Enable (WDT Enabled)