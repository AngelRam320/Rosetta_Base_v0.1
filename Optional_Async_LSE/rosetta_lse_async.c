#include "rosetta_lse_async.h"

static RosettaLSEAsyncState s_state = ROSETTA_LSE_IDLE;
static uint32_t s_start_ms = 0u;
static uint32_t s_timeout_ms = 3000u;
static uint32_t s_startup_ms = 0u;
static bool s_rtc_initialized = false;

static HAL_StatusTypeDef rtc_init_from_lse(RTC_HandleTypeDef *hrtc)
{
    if (hrtc == NULL)
        return HAL_ERROR;

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    /* If RTC already has LSE selected (for example, VBAT kept the backup domain
       alive), do not reset the backup domain and do not destroy the calendar. */
    uint32_t rtcsel = READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL);
    if (rtcsel != 0u && rtcsel != RCC_BDCR_RTCSEL_0)
    {
        /* A different RTC source is already selected. Changing it would require
           resetting the backup domain, so report an error instead of silently
           erasing retained RTC state. */
        return HAL_ERROR;
    }

    if (rtcsel == 0u)
    {
        RCC_PeriphCLKInitTypeDef pclk = {0};
        pclk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        pclk.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
        if (HAL_RCCEx_PeriphCLKConfig(&pclk) != HAL_OK)
            return HAL_ERROR;
    }

    __HAL_RCC_RTC_ENABLE();

    hrtc->Instance = RTC;
    hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc->Init.AsynchPrediv = 127u;
    hrtc->Init.SynchPrediv = 255u;
    hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    return HAL_RTC_Init(hrtc);
}

void RosettaLSEAsync_Begin(uint32_t timeout_ms)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    s_timeout_ms = (timeout_ms == 0u) ? 3000u : timeout_ms;
    s_startup_ms = 0u;
    s_rtc_initialized = false;

    /* If VBAT kept LSE running, it is already ready and there is nothing to
       start. Otherwise request HIGH drive only for startup and enable LSE. */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET)
    {
        s_state = ROSETTA_LSE_READY;
        return;
    }

    if ((RCC->BDCR & RCC_BDCR_LSEON) == 0u)
    {
        __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
    }

    s_start_ms = HAL_GetTick();
    s_state = ROSETTA_LSE_STARTING;
}

void RosettaLSEAsync_Update(RTC_HandleTypeDef *hrtc)
{
    if (s_state == ROSETTA_LSE_READY)
    {
        if (!s_rtc_initialized)
        {
            if (rtc_init_from_lse(hrtc) == HAL_OK)
                s_rtc_initialized = true;
            else
                s_state = ROSETTA_LSE_RTC_ERROR;
        }
        return;
    }

    if (s_state != ROSETTA_LSE_STARTING)
        return;

    uint32_t now = HAL_GetTick();

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET)
    {
        s_startup_ms = now - s_start_ms;
        s_state = ROSETTA_LSE_READY;

        /* Once the oscillator has started, reduce drive to a lower steady-state
           setting. If your particular crystal proves marginal, leave HIGH. */
        __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

        if (rtc_init_from_lse(hrtc) == HAL_OK)
            s_rtc_initialized = true;
        else
            s_state = ROSETTA_LSE_RTC_ERROR;

        return;
    }

    if ((now - s_start_ms) >= s_timeout_ms)
        s_state = ROSETTA_LSE_TIMEOUT;
}

RosettaLSEAsyncState RosettaLSEAsync_GetState(void)
{
    return s_state;
}

bool RosettaLSEAsync_IsReady(void)
{
    return s_state == ROSETTA_LSE_READY;
}

uint32_t RosettaLSEAsync_GetStartupMs(void)
{
    return s_startup_ms;
}
