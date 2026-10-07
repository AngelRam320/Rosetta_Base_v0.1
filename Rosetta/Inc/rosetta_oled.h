#ifndef ROSETTA_OLED_H
#define ROSETTA_OLED_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define ROSETTA_OLED_WIDTH  128
#define ROSETTA_OLED_HEIGHT 64

bool RosettaOLED_Init(I2C_HandleTypeDef *hi2c);
uint8_t RosettaOLED_GetAddress7(void);
void RosettaOLED_Clear(void);
void RosettaOLED_Update(void);
void RosettaOLED_DrawPixel(uint8_t x, uint8_t y, bool on);
void RosettaOLED_DrawBitmap128x64(const uint8_t *bitmap);
void RosettaOLED_DrawChar(uint8_t x, uint8_t y, char c);
void RosettaOLED_DrawCharScale(uint8_t x, uint8_t y, char c, uint8_t scale);
void RosettaOLED_DrawString(uint8_t x, uint8_t y, const char *s);
void RosettaOLED_DrawStringScale(uint8_t x, uint8_t y, const char *s, uint8_t scale);

#endif
