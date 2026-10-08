/*
Generic timed-text OLED demo.
Use text you wrote yourself or text you are authorized to reproduce.
The acceptance firmware does not compile this file by default.
*/
#include "rosetta_oled.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
    uint32_t at_ms;
    const char *line1;
    const char *line2;
} TimedTextCue;

static const TimedTextCue cues[] =
{
    {    0u, "YOUR TEXT",     "STARTS HERE" },
    { 2500u, "SECOND",        "TIMED LINE"  },
    { 5000u, "THIRD",         "TIMED LINE"  },
};

void TimedTextDemo_Run(void)
{
    uint32_t start = HAL_GetTick();
    uint32_t index = 0u;
    const uint32_t count = (uint32_t)(sizeof(cues) / sizeof(cues[0]));

    while (index < count)
    {
        uint32_t elapsed = HAL_GetTick() - start;
        if (elapsed >= cues[index].at_ms)
        {
            RosettaOLED_Clear();
            RosettaOLED_DrawString(0, 18, cues[index].line1);
            RosettaOLED_DrawString(0, 34, cues[index].line2);
            RosettaOLED_Update();
            index++;
        }
    }
}
