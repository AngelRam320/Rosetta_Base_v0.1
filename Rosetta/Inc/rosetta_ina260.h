#ifndef ROSETTA_INA260_H
#define ROSETTA_INA260_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t address7;
    float bus_voltage_v;
    float current_ma;
    float power_mw;
} RosettaINA260Data;

bool RosettaINA260_Probe(I2C_HandleTypeDef *hi2c, RosettaINA260Data *dev);
bool RosettaINA260_Read(I2C_HandleTypeDef *hi2c, RosettaINA260Data *dev);

#endif
