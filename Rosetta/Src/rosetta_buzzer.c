#include "rosetta_buzzer.h"

static TIM_HandleTypeDef *s_tim = NULL;
static uint32_t s_channel = 0u;

void RosettaBuzzer_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
    s_tim = htim;
    s_channel = channel;

    if (s_tim != NULL)
    {
        HAL_TIM_PWM_Stop(s_tim, s_channel);
        __HAL_TIM_SET_COMPARE(s_tim, s_channel, 0u);
    }
}

bool RosettaBuzzer_Available(void)
{
    return s_tim != NULL;
}

static uint32_t timer_input_clock_hz(void)
{
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t apb1_prescaled = (RCC->CFGR & RCC_CFGR_PPRE1);
    return (apb1_prescaled == 0u) ? pclk1 : (pclk1 * 2u);
}

void RosettaBuzzer_Tone(uint32_t frequency_hz, uint32_t duration_ms)
{
    if (s_tim == NULL || frequency_hz == 0u || duration_ms == 0u)
        return;

    uint32_t tim_clk = timer_input_clock_hz();
    if (tim_clk == 0u)
        return;

    /* Re-scale TIM12 to a convenient 1 MHz counter clock. */
    uint32_t prescaler = tim_clk / 1000000u;
    if (prescaler == 0u) prescaler = 1u;
    prescaler -= 1u;

    uint32_t counter_clk = tim_clk / (prescaler + 1u);
    uint32_t period = counter_clk / frequency_hz;
    if (period < 2u) period = 2u;
    if (period > 0x10000u) period = 0x10000u;

    uint32_t arr = period - 1u;
    uint32_t ccr = period / 2u;

    HAL_TIM_PWM_Stop(s_tim, s_channel);
    __HAL_TIM_SET_PRESCALER(s_tim, prescaler);
    __HAL_TIM_SET_AUTORELOAD(s_tim, arr);
    __HAL_TIM_SET_COMPARE(s_tim, s_channel, ccr);
    __HAL_TIM_SET_COUNTER(s_tim, 0u);
    s_tim->Instance->EGR = TIM_EGR_UG;

    if (HAL_TIM_PWM_Start(s_tim, s_channel) == HAL_OK)
    {
        HAL_Delay(duration_ms);
        HAL_TIM_PWM_Stop(s_tim, s_channel);
        __HAL_TIM_SET_COMPARE(s_tim, s_channel, 0u);
    }
}

void RosettaBuzzer_Click(void)
{
    RosettaBuzzer_Tone(1800u, 18u);
}

void RosettaBuzzer_BootChime(void)
{
    /* Short original three-note power-on chime. */
    RosettaBuzzer_Tone(659u, 70u);
    HAL_Delay(25u);
    RosettaBuzzer_Tone(784u, 70u);
    HAL_Delay(25u);
    RosettaBuzzer_Tone(988u, 110u);
}
