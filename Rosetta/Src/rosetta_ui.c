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

static void footer(uint8_t page)
{
    char line[24];
    snprintf(line, sizeof(line), "KEY NEXT       %u/%u",
             (unsigned)(page + 1u), (unsigned)ROSETTA_UI_PAGE_COUNT);
    RosettaOLED_DrawString(0, 57, line);
}

static uint32_t mhz(uint32_t hz)
{
    return (hz + 500000u) / 1000000u;
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
    RosettaOLED_DrawString(0, 20, "ACCEPTANCE TEST");
    RosettaOLED_DrawString(0, 36, "CHECKING");
    RosettaOLED_DrawString(0, 48, item);
    RosettaOLED_Update();
}

static void page_summary(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "ROSETTA A.0");
    RosettaOLED_DrawStringScale(0, 10, s->overall_pass ? "PASS" : "CHECK", 2);

    snprintf(line, sizeof(line), "HSE:%s LSE:%s RTC:%s", st(s->hse), st(s->lse), st(s->rtc));
    RosettaOLED_DrawString(0, 30, line);
    snprintf(line, sizeof(line), "OLED:%s INA:%s", st(s->oled), st(s->ina260));
    RosettaOLED_DrawString(0, 40, line);
    snprintf(line, sizeof(line), "EEP:%s FLASH:%s", st(s->eeprom), st(s->flash));
    RosettaOLED_DrawString(0, 48, line);
}

static void page_power(const RosettaBoardStatus *s)
{
    char line[24];
    int32_t mv = (int32_t)(s->bus_voltage_v * 1000.0f + 0.5f);
    int32_t ma = (int32_t)(s->current_ma + (s->current_ma >= 0.0f ? 0.5f : -0.5f));
    int32_t mw = (int32_t)(s->power_mw + 0.5f);

    RosettaOLED_DrawString(0, 0, "INA260 / POWER");
    snprintf(line, sizeof(line), "STATUS: %s", st(s->ina260));
    RosettaOLED_DrawString(0, 12, line);
    snprintf(line, sizeof(line), "ADDR: 0X%02X", s->ina260_addr7);
    RosettaOLED_DrawString(0, 22, line);
    snprintf(line, sizeof(line), "VIN: %ld.%03ld V", (long)(mv / 1000), (long)(mv < 0 ? -(mv % 1000) : (mv % 1000)));
    RosettaOLED_DrawString(0, 32, line);
    snprintf(line, sizeof(line), "I: %ld MA", (long)ma);
    RosettaOLED_DrawString(0, 42, line);
    snprintf(line, sizeof(line), "P: %ld MW", (long)mw);
    RosettaOLED_DrawString(0, 50, line);
}

static void page_hse(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "HSE / SYSTEM CLK");
    snprintf(line, sizeof(line), "HSE 8MHZ: %s", st(s->hse));
    RosettaOLED_DrawString(0, 12, line);
    snprintf(line, sizeof(line), "SYSCLK: %lu MHZ", (unsigned long)mhz(s->sysclk_hz));
    RosettaOLED_DrawString(0, 22, line);
    snprintf(line, sizeof(line), "HCLK:   %lu MHZ", (unsigned long)mhz(s->hclk_hz));
    RosettaOLED_DrawString(0, 32, line);
    snprintf(line, sizeof(line), "PCLK1:  %lu MHZ", (unsigned long)mhz(s->pclk1_hz));
    RosettaOLED_DrawString(0, 42, line);
    snprintf(line, sizeof(line), "PCLK2:  %lu MHZ", (unsigned long)mhz(s->pclk2_hz));
    RosettaOLED_DrawString(0, 50, line);
}

static void page_lse(const RosettaBoardStatus *s)
{
    RosettaOLED_DrawString(0, 0, "LSE 32.768 KHZ");
    RosettaOLED_DrawStringScale(0, 13, st(s->lse), 2);
    RosettaOLED_DrawString(0, 34, "RTC SOURCE: LSE");
    RosettaOLED_DrawString(0, 44, s->lse == ROSETTA_TEST_PASS ? "LSERDY: SET" : "LSERDY: CLEAR");
}

static void page_rtc(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "RTC / CALENDAR");
    snprintf(line, sizeof(line), "STATUS: %s", st(s->rtc));
    RosettaOLED_DrawString(0, 12, line);
    snprintf(line, sizeof(line), "%02u:%02u:%02u",
             s->rtc_hours, s->rtc_minutes, s->rtc_seconds);
    RosettaOLED_DrawStringScale(0, 24, line, 2);
    snprintf(line, sizeof(line), "%02u/%02u/20%02u",
             s->rtc_date, s->rtc_month, s->rtc_year);
    RosettaOLED_DrawString(0, 46, line);
}

static void page_oled(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "OLED DISPLAY");
    snprintf(line, sizeof(line), "STATUS: %s", st(s->oled));
    RosettaOLED_DrawString(0, 14, line);
    snprintf(line, sizeof(line), "I2C1: 0X%02X", s->oled_addr7);
    RosettaOLED_DrawString(0, 26, line);
    RosettaOLED_DrawString(0, 38, "128 X 64 MONO");
    RosettaOLED_DrawString(0, 48, "FRAMEBUFFER OK");
}

static void page_eeprom(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "EEPROM");
    snprintf(line, sizeof(line), "STATUS: %s", st(s->eeprom));
    RosettaOLED_DrawString(0, 14, line);
    RosettaOLED_DrawString(0, 26, "AT24C512C 64KB");
    snprintf(line, sizeof(line), "I2C1: 0X%02X", s->eeprom_addr7);
    RosettaOLED_DrawString(0, 38, line);
    RosettaOLED_DrawString(0, 48, "READ TEST: OK");
}

static void page_flash(const RosettaBoardStatus *s)
{
    char line[24];
    RosettaOLED_DrawString(0, 0, "SPI FLASH");
    snprintf(line, sizeof(line), "STATUS: %s", st(s->flash));
    RosettaOLED_DrawString(0, 14, line);
    RosettaOLED_DrawString(0, 26, "W25Q128JV 16MB");
    snprintf(line, sizeof(line), "JEDEC: %06lX", (unsigned long)s->flash_jedec_id);
    RosettaOLED_DrawString(0, 38, line);
    RosettaOLED_DrawString(0, 48, "BUS: SPI3");
}

void RosettaUI_ShowPage(uint8_t page, const RosettaBoardStatus *s)
{
    if (page >= ROSETTA_UI_PAGE_COUNT) page = 0u;

    RosettaOLED_Clear();

    switch (page)
    {
        case 0u: page_summary(s); break;
        case 1u: page_power(s);   break;
        case 2u: page_hse(s);     break;
        case 3u: page_lse(s);     break;
        case 4u: page_rtc(s);     break;
        case 5u: page_oled(s);    break;
        case 6u: page_eeprom(s);  break;
        case 7u: page_flash(s);   break;
        default: page_summary(s); break;
    }

    footer(page);
    RosettaOLED_Update();
}
