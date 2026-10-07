/*
Paste only the indicated lines into the CubeMX-generated Core/Src/main.c
inside USER CODE sections.

Required CubeMX labels:
PA15 = USER_LED
PB15 = USER_KEY
PB5  = FLASH_CS

Required handles:
hi2c1, hspi3, hrtc
*/

/* USER CODE BEGIN Includes */
#include "rosetta.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
RosettaHandles rosetta_hw =
{
    .i2c_internal = &hi2c1,
    .spi_flash    = &hspi3,
    .rtc          = &hrtc
};

Rosetta_Init(&rosetta_hw);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    Rosetta_Update();
/* USER CODE END WHILE */
