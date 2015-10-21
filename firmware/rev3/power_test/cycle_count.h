/*
 * File:   cycle_count.h
 * Author: Addison
 *
* Created on March 8, 2014, 8:38 PM
 */

#ifndef CYCLE_COUNT_H
#define CYCLE_COUNT_H

#include "stm32l1xx.h"

#define DEMCR_TRCENA    0x01000000

/* Core Debug registers */
#define DEMCR           (*((volatile uint32_t *)0xE000EDFC))
#define DWT_CTRL        (*(volatile uint32_t *)0xe0001000)
#define CYCCNTENA       (1<<0)
#define DWT_CYCCNT      ((volatile uint32_t *)0xE0001004)
#define CPU_CYCLES      *DWT_CYCCNT

#define STOPWATCH_START { nStart = *((volatile unsigned int *)0xE0001004);}//DWT_CYCCNT;
#define STOPWATCH_STOP  { nStop = *((volatile unsigned int *)0xE0001004);}

void stopwatch_reset(void);
uint32_t stopwatch_getticks();
void stopwatch_delay(uint32_t ticks);
uint32_t CalcNanosecondsFromStopwatch();

#endif // CYCLE_COUNT_H