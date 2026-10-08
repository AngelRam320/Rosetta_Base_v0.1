#ifndef ROSETTA_UI_H
#define ROSETTA_UI_H

#include "rosetta_status.h"
#include <stdint.h>

#define ROSETTA_UI_PAGE_COUNT 9u

void RosettaUI_ShowSplash(void);
void RosettaUI_ShowTesting(const char *item);
void RosettaUI_ShowPage(uint8_t page, const RosettaBoardStatus *status);

#endif
