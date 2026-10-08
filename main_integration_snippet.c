/*
Rosetta Acceptance Test v0.4 - microSD/SDIO integration

IMPORTANT:
This version intentionally keeps your CURRENT blocking LSE/RTC configuration.
Do NOT apply the asynchronous-LSE patch.

CubeMX additions:
- SDIO / SD 4-bit Wide bus
- FATFS / SD Card
- 48 MHz clock domain valid for SDIO

Rosetta A.0 SDIO pins:
PB2  = SDIO_CK
PD2  = SDIO_CMD
PC8  = SDIO_D0
PB0  = SDIO_D1
PB1  = SDIO_D2
PC11 = SDIO_D3

Critical boot behavior:
Rosetta A.0 has NO card-detect input. To allow boot with an empty socket,
do NOT automatically call MX_SDIO_SD_Init() from main().
Keep the function generated, but disable its generated function call in
CubeMX Project Manager / Advanced Settings if available.

MX_FATFS_Init() SHOULD be called once at boot. It only links the disk driver.
The SD card itself is initialized on demand when page 9 is opened.
*/

/* USER CODE BEGIN Includes */
#include "rosetta.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Init */
Rosetta_EarlyClockAssist();
/* USER CODE END Init */

/* Generated calls:
   SystemClock_Config();
   MX_GPIO_Init();
   MX_I2C1_Init();
   MX_RTC_Init();
   MX_SPI3_Init();
   MX_TIM12_Init();

   DO NOT AUTO-CALL:
   MX_SDIO_SD_Init();

   DO CALL:
   MX_FATFS_Init();
*/

/* USER CODE BEGIN 2 */
RosettaHandles rosetta_hw =
{
    .i2c_internal   = &hi2c1,
    .spi_flash      = &hspi3,
    .rtc            = &hrtc,
    .sd             = &hsd,
    .buzzer_tim     = &htim12,
    .buzzer_channel = TIM_CHANNEL_1
};

Rosetta_Init(&rosetta_hw);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    Rosetta_Update();
/* USER CODE END WHILE */
