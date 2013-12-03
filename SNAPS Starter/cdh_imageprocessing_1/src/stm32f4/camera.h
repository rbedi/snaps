#pragma once
#include "global.h"

void Camera_Cmd(FunctionalState);
void Camera_Record_Snippet(uint32_t timeToRecord);
bool Camera_Is_Recording(void);
bool Camera_Power_Good(void);