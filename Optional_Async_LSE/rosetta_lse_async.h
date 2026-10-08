#ifndef ROSETTA_LSE_ASYNC_H
#define ROSETTA_LSE_ASYNC_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    ROSETTA_LSE_IDLE = 0,
    ROSETTA_LSE_STARTING,
    ROSETTA_LSE_READY,
    ROSETTA_LSE_TIMEOUT,
    ROSETTA_LSE_RTC_ERROR
} RosettaLSEAsyncState;

/* Start LSE without waiting for LSERDY. Call after HAL_Init/SystemClock_Config.
   IMPORTANT: SystemClock_Config itself must NOT enable LSE, otherwise Cube/HAL
   will already have blocked before this function can run. */
void RosettaLSEAsync_Begin(uint32_t timeout_ms);

/* Poll from the main loop. When LSE becomes ready this function selects LSE as
   RTC source and initializes the supplied RTC handle. It never blocks waiting
   for the crystal. */
void RosettaLSEAsync_Update(RTC_HandleTypeDef *hrtc);

RosettaLSEAsyncState RosettaLSEAsync_GetState(void);
bool RosettaLSEAsync_IsReady(void);
uint32_t RosettaLSEAsync_GetStartupMs(void);

#endif
