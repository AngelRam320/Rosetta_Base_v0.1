#ifndef ROSETTA_FLASH_H
#define ROSETTA_FLASH_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

bool RosettaFlash_ReadJEDEC(SPI_HandleTypeDef *hspi, uint32_t *jedec_id);
bool RosettaFlash_IsExpectedW25Q128(uint32_t jedec_id);

#endif
