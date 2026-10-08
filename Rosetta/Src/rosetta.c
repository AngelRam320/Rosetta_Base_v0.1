#include "rosetta.h"
#include "rosetta_oled.h"
#include "rosetta_ina260.h"
#include "rosetta_eeprom.h"
#include "rosetta_flash.h"
#include "rosetta_buzzer.h"
#include "rosetta_ui.h"
#include "main.h"
#include <string.h>

static RosettaHandles s_hw;
static RosettaBoardStatus s_status;
static RosettaINA260Data s_ina;
static uint32_t s_last_update = 0u;
static GPIO_PinState s_prev_key = GPIO_PIN_SET;
static uint8_t s_page = 0u;

void Rosetta_EarlyClockAssist(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    /*
     * STM32F446:
     * LSEMOD = 0 -> Low-power mode
     * LSEMOD = 1 -> High-drive mode
     *
     * Solo se puede cambiar mientras LSE está apagado.
     */
    if ((RCC->BDCR & RCC_BDCR_LSEON) == 0U)
    {
        SET_BIT(RCC->BDCR, RCC_BDCR_LSEMOD);
    }
}

static RosettaTestState flag_state(uint32_t flag)
{
    return flag ? ROSETTA_TEST_PASS : ROSETTA_TEST_FAIL;
}

static void read_clocks(void)
{
    s_status.sysclk_hz = HAL_RCC_GetSysClockFreq();
    s_status.hclk_hz   = HAL_RCC_GetHCLKFreq();
    s_status.pclk1_hz  = HAL_RCC_GetPCLK1Freq();
    s_status.pclk2_hz  = HAL_RCC_GetPCLK2Freq();
}

static void read_rtc(void)
{
    if (s_hw.rtc == NULL || s_status.lse != ROSETTA_TEST_PASS)
    {
        s_status.rtc = ROSETTA_TEST_FAIL;
        return;
    }

    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    if (HAL_RTC_GetTime(s_hw.rtc, &t, RTC_FORMAT_BIN) == HAL_OK &&
        HAL_RTC_GetDate(s_hw.rtc, &d, RTC_FORMAT_BIN) == HAL_OK)
    {
        s_status.rtc_hours   = t.Hours;
        s_status.rtc_minutes = t.Minutes;
        s_status.rtc_seconds = t.Seconds;
        s_status.rtc_date    = d.Date;
        s_status.rtc_month   = d.Month;
        s_status.rtc_year    = d.Year;
        s_status.rtc = ROSETTA_TEST_PASS;
    }
    else
    {
        s_status.rtc = ROSETTA_TEST_FAIL;
    }
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

    RosettaBuzzer_Init(s_hw.buzzer_tim, s_hw.buzzer_channel);

    read_clocks();
    s_status.hse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY));
    s_status.lse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));

    /* OLED first: it is the onboard diagnostic console. */
    if (RosettaOLED_Init(s_hw.i2c_internal))
    {
        s_status.oled = ROSETTA_TEST_PASS;
        s_status.oled_addr7 = RosettaOLED_GetAddress7();
        RosettaUI_ShowSplash();
        RosettaBuzzer_BootChime();
        HAL_Delay(900u);
    }
    else
    {
        s_status.oled = ROSETTA_TEST_FAIL;
        RosettaBuzzer_BootChime();
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
    uint32_t jedec = 0u;
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

    /* Re-check LSERDY here. If HAL needed almost its full cold-start timeout,
       this reflects the final oscillator state rather than an earlier snapshot. */
    s_status.lse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));
    read_rtc();
    update_overall();

    if (s_status.overall_pass)
        HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);

    if (s_status.oled == ROSETTA_TEST_PASS)
        RosettaUI_ShowPage(s_page, &s_status);

    s_prev_key = HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin);
    s_last_update = HAL_GetTick();
}

void Rosetta_Update(void)
{
    uint32_t now = HAL_GetTick();

    GPIO_PinState key = HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin);
    if (s_prev_key == GPIO_PIN_SET && key == GPIO_PIN_RESET)
    {
        HAL_Delay(20u);
        if (HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == GPIO_PIN_RESET)
        {
            RosettaBuzzer_Click();
            s_page++;
            if (s_page >= ROSETTA_UI_PAGE_COUNT) s_page = 0u;

            if (s_status.oled == ROSETTA_TEST_PASS)
                RosettaUI_ShowPage(s_page, &s_status);
        }
    }
    s_prev_key = key;

    if ((now - s_last_update) >= 500u)
    {
        s_last_update = now;

        s_status.lse = flag_state(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));
        read_clocks();
        read_rtc();

        if (s_status.ina260 == ROSETTA_TEST_PASS &&
            RosettaINA260_Read(s_hw.i2c_internal, &s_ina))
        {
            s_status.bus_voltage_v = s_ina.bus_voltage_v;
            s_status.current_ma = s_ina.current_ma;
            s_status.power_mw = s_ina.power_mw;
        }

        update_overall();
        HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin,
                          s_status.overall_pass ? GPIO_PIN_SET : GPIO_PIN_RESET);

        if (s_status.oled == ROSETTA_TEST_PASS)
            RosettaUI_ShowPage(s_page, &s_status);
    }
}

const RosettaBoardStatus *Rosetta_GetStatus(void)
{
    return &s_status;
}
