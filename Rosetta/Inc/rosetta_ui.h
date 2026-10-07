#ifndef ROSETTA_UI_H
#define ROSETTA_UI_H

#include "rosetta_status.h"

void RosettaUI_ShowSplash(void);
void RosettaUI_ShowTesting(const char *item);
void RosettaUI_ShowStatus(const RosettaBoardStatus *status);
void RosettaUI_ShowDetails(const RosettaBoardStatus *status);

#endif
