#ifndef ROSETTA_EEPROM_H
#define ROSETTA_EEPROM_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

bool RosettaEEPROM_Probe(I2C_HandleTypeDef *hi2c);
uint8_t RosettaEEPROM_GetAddress7(void);
bool RosettaEEPROM_ReadByte(I2C_HandleTypeDef *hi2c, uint16_t address, uint8_t *value);

/* Intentionally no automatic write test at boot:
   the base self-test is non-destructive. */

#endif
