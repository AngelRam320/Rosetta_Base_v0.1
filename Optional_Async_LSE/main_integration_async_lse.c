/*
OPTIONAL: non-blocking LSE/RTC startup for Rosetta.

CubeMX for this variant:
- Keep HSE + PLL as usual (180 MHz).
- DO NOT let SystemClock_Config enable LSE.
- Remove/disable the CubeMX RTC clock-source configuration and MX_RTC_Init().
- Keep PC14/PC15 free; the LSE oscillator will own them when enabled.
- GPIO, I2C1, SPI3 and TIM12 remain normal.

Because CubeMX no longer generates hrtc for this variant, if necessary declare it
inside USER CODE BEGIN PV:

    RTC_HandleTypeDef hrtc;

The Rosetta async helper selects LSE as RTC source only after LSERDY is set.
*/

/* USER CODE BEGIN Includes */
#include "rosetta.h"
#include "rosetta_lse_async.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
RTC_HandleTypeDef hrtc;
/* USER CODE END PV */

/* Normal generated startup: */
/* HAL_Init();              */
/* SystemClock_Config();    */
/* MX_GPIO_Init();          */
/* MX_I2C1_Init();          */
/* MX_SPI3_Init();          */
/* MX_TIM12_Init();         */
/* NO MX_RTC_Init() here.   */

/* USER CODE BEGIN 2 */
RosettaHandles rosetta_hw =
{
    .i2c_internal   = &hi2c1,
    .spi_flash      = &hspi3,
    .rtc            = &hrtc,
    .buzzer_tim     = &htim12,
    .buzzer_channel = TIM_CHANNEL_1
};

/* Start the UI/test code immediately. In this optional variant Rosetta_Init()
   should not treat RTC/LSE as fatal during the first few hundred ms. */
Rosetta_Init(&rosetta_hw);
RosettaLSEAsync_Begin(3000u);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    RosettaLSEAsync_Update(&hrtc);
    Rosetta_Update();
/* USER CODE END WHILE */
