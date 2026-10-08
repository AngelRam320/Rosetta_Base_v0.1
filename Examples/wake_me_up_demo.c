/*
Rosetta OLED timed-text demo
Text supplied by the project owner for this demo.

This example is intentionally NOT compiled by the Acceptance Test target.
Put it on the separate Git branch `demo-wake-me-up`.

The timing values below are STARTING VALUES ONLY. Use the song version you will
actually play and tune `at_ms` until the OLED changes exactly on each phrase.
KEY can be used as a start trigger: capture HAL_GetTick() when the first supplied
line begins, then call WakeMeUpDemo_Update(now - start_ms).
*/

#include "rosetta_oled.h"
#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint32_t at_ms;
    const char *line1;
    const char *line2;
} TimedCard;

static const TimedCard k_cards[] =
{
    {    0u, "Are you real",      "or an illusion?" },
    { 3200u, "'Cause I fear",      "your love's my delusion" },
    { 6500u, "Wake me up,",        "come find me" },
    { 9000u, "It feels like",      "I'm dying (ooh-whoa)" },
    {12000u, "Wake me up,",        "these demons" },
    {15000u, "Keep creeping,",     "don't fear 'em" },
    {18100u, "Wake me up,",        "'cause I'm tryin'" },
    {21200u, "This sleep is",      "paralyzing (oh)" },
    {24400u, "Wake me up,",        "'cause I'm dreaming" },
    {27700u, "It feels so damn",   "real" },
    {30600u, "Wake me up",         "" }
};

#define CARD_COUNT ((uint32_t)(sizeof(k_cards) / sizeof(k_cards[0])))

static uint32_t s_last_card = 0xFFFFFFFFu;

void WakeMeUpDemo_Reset(void)
{
    s_last_card = 0xFFFFFFFFu;
    RosettaOLED_Clear();
    RosettaOLED_DrawStringScale(0, 4, "WAKE ME UP", 2);
    RosettaOLED_DrawString(0, 34, "PRESS KEY AT");
    RosettaOLED_DrawString(0, 46, "FIRST LINE");
    RosettaOLED_Update();
}

void WakeMeUpDemo_Update(uint32_t elapsed_ms)
{
    uint32_t card = 0u;

    for (uint32_t i = 0u; i < CARD_COUNT; ++i)
    {
        if (elapsed_ms >= k_cards[i].at_ms)
            card = i;
        else
            break;
    }

    if (card == s_last_card)
        return;

    s_last_card = card;
    RosettaOLED_Clear();
    RosettaOLED_DrawStringScale(0, 10, k_cards[card].line1, 1);
    RosettaOLED_DrawStringScale(0, 30, k_cards[card].line2, 1);
    RosettaOLED_Update();
}
