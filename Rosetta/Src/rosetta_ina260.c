#include "rosetta_ina260.h"

#define INA260_REG_CURRENT        0x01u
#define INA260_REG_BUS_VOLTAGE    0x02u
#define INA260_REG_POWER          0x03u
#define INA260_REG_MANUFACTURER   0xFEu
#define INA260_REG_DIE_ID         0xFFu

#define INA260_MANUFACTURER_TI    0x5449u
#define INA260_DIE_ID_EXPECTED    0x2270u

static bool read_u16(I2C_HandleTypeDef *hi2c, uint8_t addr7, uint8_t reg, uint16_t *value)
{
    uint8_t raw[2];
    if (HAL_I2C_Mem_Read(hi2c, (uint16_t)(addr7 << 1), reg, I2C_MEMADD_SIZE_8BIT,
                         raw, 2, 50) != HAL_OK)
        return false;

    *value = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    return true;
}

bool RosettaINA260_Probe(I2C_HandleTypeDef *hi2c, RosettaINA260Data *dev)
{
    for (uint8_t addr = 0x40; addr <= 0x4F; addr++)
    {
        if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr << 1), 2, 20) != HAL_OK)
            continue;

        uint16_t mfg = 0, die = 0;
        if (!read_u16(hi2c, addr, INA260_REG_MANUFACTURER, &mfg)) continue;
        if (!read_u16(hi2c, addr, INA260_REG_DIE_ID, &die)) continue;

        if (mfg == INA260_MANUFACTURER_TI && die == INA260_DIE_ID_EXPECTED)
        {
            dev->address7 = addr;
            return RosettaINA260_Read(hi2c, dev);
        }
    }

    dev->address7 = 0;
    return false;
}

bool RosettaINA260_Read(I2C_HandleTypeDef *hi2c, RosettaINA260Data *dev)
{
    if (dev->address7 == 0) return false;

    uint16_t raw_bus = 0, raw_current = 0, raw_power = 0;
    if (!read_u16(hi2c, dev->address7, INA260_REG_BUS_VOLTAGE, &raw_bus)) return false;
    if (!read_u16(hi2c, dev->address7, INA260_REG_CURRENT, &raw_current)) return false;
    if (!read_u16(hi2c, dev->address7, INA260_REG_POWER, &raw_power)) return false;

    dev->bus_voltage_v = (float)raw_bus * 0.00125f;           /* 1.25 mV/LSB */
    dev->current_ma    = (float)(int16_t)raw_current * 1.25f; /* 1.25 mA/LSB */
    dev->power_mw      = (float)raw_power * 10.0f;            /* 10 mW/LSB */
    return true;
}
