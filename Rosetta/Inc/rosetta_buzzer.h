#ifndef ROSETTA_BUZZER_H
#define ROSETTA_BUZZER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

void RosettaBuzzer_Init(TIM_HandleTypeDef *htim, uint32_t channel);
bool RosettaBuzzer_Available(void);
void RosettaBuzzer_Tone(uint32_t frequency_hz, uint32_t duration_ms);
void RosettaBuzzer_Click(void);
void RosettaBuzzer_BootChime(void);

#endif
