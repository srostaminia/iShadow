#include "cycle_count.h"

uint32_t nStart;               //DEBUG Stopwatch start cycle counter value
uint32_t nStop;                //DEBUG Stopwatch stop cycle counter value

inline void stopwatch_reset(void)
{
    /* Enable DWT */
    DEMCR |= DEMCR_TRCENA; 
    *DWT_CYCCNT = 0;             
    /* Enable CPU cycle counter */
    DWT_CTRL |= CYCCNTENA;
}

inline uint32_t stopwatch_getticks()
{
    return CPU_CYCLES;
}

inline void stopwatch_delay(uint32_t ticks)
{
    stopwatch_reset();
    while(1)
    {
            if (stopwatch_getticks() >= ticks)
                    break;
    }
}

uint32_t CalcNanosecondsFromStopwatch()
{
    uint32_t nTemp;
    uint32_t n;

    nTemp = nStop - nStart;

    nTemp *= 1000;                          // Scale cycles by 1000.
    n = SystemCoreClock / 1000000;          // Convert Hz to MHz. SystemCoreClock = 168000000
    nTemp = nTemp / n;                      // nanosec = (Cycles * 1000) / (Cycles/microsec)

    return nTemp;
} 
