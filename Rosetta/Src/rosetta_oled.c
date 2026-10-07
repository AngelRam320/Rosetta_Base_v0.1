#include "rosetta_oled.h"
#include "rosetta_font5x7.h"
#include <string.h>

static I2C_HandleTypeDef *s_i2c = NULL;
static uint8_t s_addr7 = 0;
static uint8_t s_fb[ROSETTA_OLED_WIDTH * ROSETTA_OLED_HEIGHT / 8];

static bool tx_cmd(uint8_t cmd)
{
    uint8_t packet[2] = {0x00, cmd};
    return HAL_I2C_Master_Transmit(s_i2c, (uint16_t)(s_addr7 << 1), packet, 2, 50) == HAL_OK;
}

static bool tx_cmd2(uint8_t cmd, uint8_t arg)
{
    uint8_t packet[3] = {0x00, cmd, arg};
    return HAL_I2C_Master_Transmit(s_i2c, (uint16_t)(s_addr7 << 1), packet, 3, 50) == HAL_OK;
}

bool RosettaOLED_Init(I2C_HandleTypeDef *hi2c)
{
    s_i2c = hi2c;
    HAL_Delay(100);

    if (HAL_I2C_IsDeviceReady(s_i2c, (0x3Cu << 1), 2, 20) == HAL_OK)
        s_addr7 = 0x3C;
    else if (HAL_I2C_IsDeviceReady(s_i2c, (0x3Du << 1), 2, 20) == HAL_OK)
        s_addr7 = 0x3D;
    else
        return false;

    /* SSD1315, compatible command set with SSD1306 for this 128x64 module */
    if (!tx_cmd(0xAE)) return false;       /* display off */
    if (!tx_cmd2(0xD5, 0x80)) return false;
    if (!tx_cmd2(0xA8, 0x3F)) return false;
    if (!tx_cmd2(0xD3, 0x00)) return false;
    if (!tx_cmd(0x40)) return false;
    if (!tx_cmd2(0x8D, 0x14)) return false; /* charge pump */
    if (!tx_cmd2(0x20, 0x00)) return false; /* horizontal addressing */
    if (!tx_cmd(0xA1)) return false;
    if (!tx_cmd(0xC8)) return false;
    if (!tx_cmd2(0xDA, 0x12)) return false;
    if (!tx_cmd2(0x81, 0x7F)) return false;
    if (!tx_cmd2(0xD9, 0xF1)) return false;
    if (!tx_cmd2(0xDB, 0x40)) return false;
    if (!tx_cmd(0xA4)) return false;
    if (!tx_cmd(0xA6)) return false;
    if (!tx_cmd(0xAF)) return false;

    RosettaOLED_Clear();
    RosettaOLED_Update();
    return true;
}

uint8_t RosettaOLED_GetAddress7(void)
{
    return s_addr7;
}

void RosettaOLED_Clear(void)
{
    memset(s_fb, 0, sizeof(s_fb));
}

void RosettaOLED_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= ROSETTA_OLED_WIDTH || y >= ROSETTA_OLED_HEIGHT) return;
    uint16_t index = x + (uint16_t)(y / 8u) * ROSETTA_OLED_WIDTH;
    uint8_t mask = (uint8_t)(1u << (y & 7u));
    if (on) s_fb[index] |= mask;
    else    s_fb[index] &= (uint8_t)~mask;
}

void RosettaOLED_DrawBitmap128x64(const uint8_t *bitmap)
{
    memcpy(s_fb, bitmap, sizeof(s_fb));
}

void RosettaOLED_DrawCharScale(uint8_t x, uint8_t y, char c, uint8_t scale)
{
    if (scale == 0u) scale = 1u;
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c < ROSETTA_FONT_FIRST || c > ROSETTA_FONT_LAST) c = '?';

    uint8_t idx = (uint8_t)c - ROSETTA_FONT_FIRST;

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t bits = rosetta_font5x7[idx][col];
        for (uint8_t row = 0; row < 7; row++)
        {
            if (((bits >> row) & 1u) == 0u) continue;

            for (uint8_t dx = 0; dx < scale; dx++)
                for (uint8_t dy = 0; dy < scale; dy++)
                    RosettaOLED_DrawPixel(
                        (uint8_t)(x + col * scale + dx),
                        (uint8_t)(y + row * scale + dy),
                        true);
        }
    }
}

void RosettaOLED_DrawChar(uint8_t x, uint8_t y, char c)
{
    RosettaOLED_DrawCharScale(x, y, c, 1u);
}

void RosettaOLED_DrawStringScale(uint8_t x, uint8_t y, const char *s, uint8_t scale)
{
    if (scale == 0u) scale = 1u;
    uint8_t advance = (uint8_t)(6u * scale);

    while (*s && x <= (ROSETTA_OLED_WIDTH - 5u * scale))
    {
        RosettaOLED_DrawCharScale(x, y, *s++, scale);
        x = (uint8_t)(x + advance);
    }
}

void RosettaOLED_DrawString(uint8_t x, uint8_t y, const char *s)
{
    RosettaOLED_DrawStringScale(x, y, s, 1u);
}

void RosettaOLED_Update(void)
{
    if (s_i2c == NULL || s_addr7 == 0) return;

    uint8_t cmd_col[]  = {0x00, 0x21, 0x00, 0x7F};
    uint8_t cmd_page[] = {0x00, 0x22, 0x00, 0x07};

    if (HAL_I2C_Master_Transmit(s_i2c, (uint16_t)(s_addr7 << 1), cmd_col, sizeof(cmd_col), 50) != HAL_OK)
        return;
    if (HAL_I2C_Master_Transmit(s_i2c, (uint16_t)(s_addr7 << 1), cmd_page, sizeof(cmd_page), 50) != HAL_OK)
        return;

    /* Small chunks keep stack usage low and make timeouts bounded. */
    uint8_t packet[33];
    packet[0] = 0x40;

    for (uint16_t offset = 0; offset < sizeof(s_fb); offset += 32)
    {
        memcpy(&packet[1], &s_fb[offset], 32);
        if (HAL_I2C_Master_Transmit(s_i2c, (uint16_t)(s_addr7 << 1), packet, sizeof(packet), 50) != HAL_OK)
            return;
    }
}
