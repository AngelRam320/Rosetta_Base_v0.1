#ifndef ROSETTA_H
#define ROSETTA_H

#include "stm32f4xx_hal.h"
#include "rosetta_status.h"

typedef struct
{
    I2C_HandleTypeDef *i2c_internal; /* I2C1 */
    SPI_HandleTypeDef *spi_flash;    /* SPI3 */
    RTC_HandleTypeDef *rtc;
    SD_HandleTypeDef *sd;           /* SDIO, optional / tested on page 9 */
    TIM_HandleTypeDef *buzzer_tim;   /* TIM12, optional */
    uint32_t buzzer_channel;         /* TIM_CHANNEL_1 */
} RosettaHandles;

/* Call from USER CODE BEGIN Init, before SystemClock_Config().
   STM32F446: this does NOT enable LSE; it only selects LSE high-drive mode
   on a true cold start, while LSE is still disabled. */
void Rosetta_EarlyClockAssist(void);

void Rosetta_Init(const RosettaHandles *handles);
void Rosetta_Update(void);
const RosettaBoardStatus *Rosetta_GetStatus(void);

#endif
