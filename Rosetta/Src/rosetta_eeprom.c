#include "rosetta_eeprom.h"

static uint8_t s_addr7 = 0;

bool RosettaEEPROM_Probe(I2C_HandleTypeDef *hi2c)
{
    /* The PCB fixes A0/A1/A2, but scanning 0x50..0x57 makes bring-up
       tolerant to an address-strap revision without hiding real faults. */
    for (uint8_t addr = 0x50; addr <= 0x57; addr++)
    {
        if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr << 1), 3, 20) != HAL_OK)
            continue;

        s_addr7 = addr;

        /* Non-destructive read verifies that the device accepts a 16-bit
           memory address, appropriate for AT24C512C / 64 KiB. */
        uint8_t dummy = 0;
        if (RosettaEEPROM_ReadByte(hi2c, 0x0000u, &dummy))
            return true;
    }

    s_addr7 = 0;
    return false;
}

uint8_t RosettaEEPROM_GetAddress7(void)
{
    return s_addr7;
}

bool RosettaEEPROM_ReadByte(I2C_HandleTypeDef *hi2c, uint16_t address, uint8_t *value)
{
    if (s_addr7 == 0) return false;

    return HAL_I2C_Mem_Read(hi2c,
                            (uint16_t)(s_addr7 << 1),
                            address,
                            I2C_MEMADD_SIZE_16BIT,
                            value,
                            1,
                            50) == HAL_OK;
}
