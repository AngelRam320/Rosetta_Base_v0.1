#ifndef ROSETTA_H
#define ROSETTA_H

#include "stm32f4xx_hal.h"
#include "rosetta_status.h"

typedef struct
{
    I2C_HandleTypeDef *i2c_internal; /* I2C1 */
    SPI_HandleTypeDef *spi_flash;    /* SPI3 */
    RTC_HandleTypeDef *rtc;
} RosettaHandles;

void Rosetta_Init(const RosettaHandles *handles);
void Rosetta_Update(void);
const RosettaBoardStatus *Rosetta_GetStatus(void);

#endif
