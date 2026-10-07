#include "rosetta.h"
#include "rosetta_oled.h"
#include "rosetta_ina260.h"
#include "rosetta_eeprom.h"
#include "rosetta_flash.h"
#include "rosetta_ui.h"
#include "main.h"
#include <string.h>

static RosettaHandles s_hw;
static RosettaBoardStatus s_status;
static RosettaINA260Data s_ina;
static uint32_t s_last_update = 0;
static GPIO_PinState s_prev_key = GPIO_PIN_SET;
static uint8_t s_page = 0;

static RosettaTestState flag_state(uint32_t flag)
{
    return flag ? ROSETTA_TEST_PASS : ROSETTA_TEST_FAIL;
}

static void update_overall(void)
{
    s_status.overall_pass =
        (s_status.hse    == ROSETTA_TEST_PASS) &&
        (s_status.lse    == ROSETTA_TEST_PASS) &&
        (s_status.oled   == ROSETTA_TEST_PASS) &&
        (s_status.ina260 == ROSETTA_TEST_PASS) &&
        (s_status.eeprom == ROSETTA_TEST_PASS) &&
        (s_status.flash  == ROSETTA_TEST_PASS) &&
        (s_status.rtc    == ROSETTA_TEST_PASS);
}

void Rosetta_Init(const RosettaHandles *handles)
{
    memset(&s_status, 0, sizeof(s_status));
    memset(&s_ina, 0, sizeof(s_ina));
    s_hw = *handles;

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

    s_status.hse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY));
    s_status.lse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));

    /* OLED must be first because it becomes the onboard console. */
    if (RosettaOLED_Init(s_hw.i2c_internal))
    {
        s_status.oled = ROSETTA_TEST_PASS;
        s_status.oled_addr7 = RosettaOLED_GetAddress7();
        RosettaUI_ShowSplash();
        HAL_Delay(1400);
    }
    else
    {
        s_status.oled = ROSETTA_TEST_FAIL;
    }

    if (s_status.oled == ROSETTA_TEST_PASS) RosettaUI_ShowTesting("INA260");
    s_status.ina260 = RosettaINA260_Probe(s_hw.i2c_internal, &s_ina) ?
                      ROSETTA_TEST_PASS : ROSETTA_TEST_FAIL;
    if (s_status.ina260 == ROSETTA_TEST_PASS)
    {
        s_status.ina260_addr7 = s_ina.address7;
        s_status.bus_voltage_v = s_ina.bus_voltage_v;
        s_status.current_ma = s_ina.current_ma;
        s_status.power_mw = s_ina.power_mw;
    }

    if (s_status.oled == ROSETTA_TEST_PASS) RosettaUI_ShowTesting("EEPROM");
    s_status.eeprom = RosettaEEPROM_Probe(s_hw.i2c_internal) ?
                      ROSETTA_TEST_PASS : ROSETTA_TEST_FAIL;
    if (s_status.eeprom == ROSETTA_TEST_PASS)
        s_status.eeprom_addr7 = RosettaEEPROM_GetAddress7();

    if (s_status.oled == ROSETTA_TEST_PASS) RosettaUI_ShowTesting("FLASH");
    uint32_t jedec = 0;
    if (RosettaFlash_ReadJEDEC(s_hw.spi_flash, &jedec))
    {
        s_status.flash_jedec_id = jedec;
        s_status.flash = RosettaFlash_IsExpectedW25Q128(jedec) ?
                         ROSETTA_TEST_PASS : ROSETTA_TEST_FAIL;
    }
    else
    {
        s_status.flash = ROSETTA_TEST_FAIL;
    }

    if (s_status.oled == ROSETTA_TEST_PASS) RosettaUI_ShowTesting("RTC / LSE");
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    if (s_status.lse == ROSETTA_TEST_PASS &&
        HAL_RTC_GetTime(s_hw.rtc, &t, RTC_FORMAT_BIN) == HAL_OK &&
        HAL_RTC_GetDate(s_hw.rtc, &d, RTC_FORMAT_BIN) == HAL_OK)
        s_status.rtc = ROSETTA_TEST_PASS;
    else
        s_status.rtc = ROSETTA_TEST_FAIL;

    update_overall();

    if (s_status.overall_pass)
        HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);

    if (s_status.oled == ROSETTA_TEST_PASS)
        RosettaUI_ShowStatus(&s_status);

    s_prev_key = HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin);
    s_last_update = HAL_GetTick();
}

void Rosetta_Update(void)
{
    uint32_t now = HAL_GetTick();

    /* Onboard KEY changes screen. Simple software debounce. */
    GPIO_PinState key = HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin);
    if (s_prev_key == GPIO_PIN_SET && key == GPIO_PIN_RESET)
    {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == GPIO_PIN_RESET)
        {
            s_page ^= 1u;
            if (s_status.oled == ROSETTA_TEST_PASS)
            {
                if (s_page == 0) RosettaUI_ShowStatus(&s_status);
                else             RosettaUI_ShowDetails(&s_status);
            }
        }
    }
    s_prev_key = key;

    /* Refresh live electrical measurements twice per second. */
    if ((now - s_last_update) >= 500u)
    {
        s_last_update = now;

        if (s_status.ina260 == ROSETTA_TEST_PASS &&
            RosettaINA260_Read(s_hw.i2c_internal, &s_ina))
        {
            s_status.bus_voltage_v = s_ina.bus_voltage_v;
            s_status.current_ma = s_ina.current_ma;
            s_status.power_mw = s_ina.power_mw;
        }

        if (s_status.oled == ROSETTA_TEST_PASS && s_page == 0)
            RosettaUI_ShowStatus(&s_status);
    }
}

const RosettaBoardStatus *Rosetta_GetStatus(void)
{
    return &s_status;
}
