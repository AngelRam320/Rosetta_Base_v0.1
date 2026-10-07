#include "rosetta_ui.h"
#include "rosetta_oled.h"
#include "rosetta_logo.h"
#include <stdio.h>

static const char *st(RosettaTestState s)
{
    switch (s)
    {
        case ROSETTA_TEST_PASS:     return "OK";
        case ROSETTA_TEST_FAIL:     return "ERR";
        case ROSETTA_TEST_UNTESTED: return "--";
        default:                    return "?";
    }
}

void RosettaUI_ShowSplash(void)
{
    RosettaOLED_DrawBitmap128x64(rosetta_logo_128x64);
    RosettaOLED_Update();
}

void RosettaUI_ShowTesting(const char *item)
{
    RosettaOLED_Clear();
    RosettaOLED_DrawStringScale(0, 0, "ROSETTA", 2);
    RosettaOLED_DrawString(0, 20, "SELF TEST");
    RosettaOLED_DrawString(0, 36, "CHECKING");
    RosettaOLED_DrawString(0, 48, item);
    RosettaOLED_Update();
}

void RosettaUI_ShowStatus(const RosettaBoardStatus *s)
{
    char line[24];
    uint32_t mv = (uint32_t)(s->bus_voltage_v * 1000.0f + 0.5f);
    int32_t ma = (int32_t)(s->current_ma + (s->current_ma >= 0.0f ? 0.5f : -0.5f));

    RosettaOLED_Clear();

    RosettaOLED_DrawString(0, 0, "ROSETTA A.0");

    snprintf(line, sizeof(line), "INA:%s EE:%s", st(s->ina260), st(s->eeprom));
    RosettaOLED_DrawString(0, 10, line);

    snprintf(line, sizeof(line), "FLASH:%s OLED:%s", st(s->flash), st(s->oled));
    RosettaOLED_DrawString(0, 20, line);

    snprintf(line, sizeof(line), "HSE:%s LSE:%s", st(s->hse), st(s->lse));
    RosettaOLED_DrawString(0, 30, line);

    snprintf(line, sizeof(line), "RTC:%s", st(s->rtc));
    RosettaOLED_DrawString(0, 40, line);

    snprintf(line, sizeof(line), "VIN:%lu.%03luV",
             (unsigned long)(mv / 1000u), (unsigned long)(mv % 1000u));
    RosettaOLED_DrawString(0, 50, line);

    snprintf(line, sizeof(line), "I:%ldmA", (long)ma);
    RosettaOLED_DrawString(72, 50, line);

    RosettaOLED_Update();
}

void RosettaUI_ShowDetails(const RosettaBoardStatus *s)
{
    char line[24];

    RosettaOLED_Clear();
    RosettaOLED_DrawString(0, 0, "BOARD DETAILS");

    snprintf(line, sizeof(line), "OLED:0X%02X", s->oled_addr7);
    RosettaOLED_DrawString(0, 12, line);

    snprintf(line, sizeof(line), "INA :0X%02X", s->ina260_addr7);
    RosettaOLED_DrawString(0, 22, line);

    snprintf(line, sizeof(line), "EEP :0X%02X", s->eeprom_addr7);
    RosettaOLED_DrawString(0, 32, line);

    snprintf(line, sizeof(line), "FLASH:%06lX", (unsigned long)s->flash_jedec_id);
    RosettaOLED_DrawString(0, 42, line);

    RosettaOLED_DrawString(0, 54, "KEY: NEXT");
    RosettaOLED_Update();
}
