#include "rosetta_flash.h"
#include "main.h"

bool RosettaFlash_ReadJEDEC(SPI_HandleTypeDef *hspi, uint32_t *jedec_id)
{
    uint8_t tx[4] = {0x9Fu, 0xFFu, 0xFFu, 0xFFu};
    uint8_t rx[4] = {0};

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(hspi, tx, rx, sizeof(tx), 100);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

    if (st != HAL_OK)
        return false;

    *jedec_id = ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | rx[3];
    return true;
}

bool RosettaFlash_IsExpectedW25Q128(uint32_t jedec_id)
{
    return jedec_id == 0xEF4018u;
}
