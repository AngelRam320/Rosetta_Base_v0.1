/*
Rosetta Acceptance Test v0.3 integration

Required CubeMX labels:
PA15 = USER_LED
PB15 = USER_KEY
PB5  = FLASH_CS

Required handles:
hi2c1, hspi3, hrtc, htim12

TIM12:
PB14 = TIM12_CH1 / PWM Generation CH1
Initial pulse may be 0; Rosetta changes frequency dynamically.
*/

/* USER CODE BEGIN Includes */
#include "rosetta.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Init */
/* Must execute before SystemClock_Config().
   In CubeMX also set RCC -> LSE Drive Capability = High. */
Rosetta_EarlyClockAssist();
/* USER CODE END Init */

/* CubeMX generated calls remain in their normal order:
   SystemClock_Config();
   MX_GPIO_Init();
   MX_I2C1_Init();
   MX_SPI3_Init();
   MX_RTC_Init();
   MX_TIM12_Init();
*/

/* USER CODE BEGIN 2 */
RosettaHandles rosetta_hw =
{
    .i2c_internal  = &hi2c1,
    .spi_flash     = &hspi3,
    .rtc           = &hrtc,
    .buzzer_tim    = &htim12,
    .buzzer_channel = TIM_CHANNEL_1
};

Rosetta_Init(&rosetta_hw);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    Rosetta_Update();
/* USER CODE END WHILE */
