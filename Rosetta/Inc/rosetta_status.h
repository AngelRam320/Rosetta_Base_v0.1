#ifndef ROSETTA_STATUS_H
#define ROSETTA_STATUS_H

#include <stdint.h>

typedef enum
{
    ROSETTA_TEST_UNKNOWN = 0,
    ROSETTA_TEST_PASS,
    ROSETTA_TEST_FAIL,
    ROSETTA_TEST_UNTESTED
} RosettaTestState;

typedef struct
{
    RosettaTestState hse;
    RosettaTestState lse;
    RosettaTestState oled;
    RosettaTestState ina260;
    RosettaTestState eeprom;
    RosettaTestState flash;
    RosettaTestState rtc;

    uint8_t oled_addr7;
    uint8_t ina260_addr7;
    uint8_t eeprom_addr7;
    uint32_t flash_jedec_id;

    float bus_voltage_v;
    float current_ma;
    float power_mw;

    uint8_t overall_pass;
} RosettaBoardStatus;

#endif
