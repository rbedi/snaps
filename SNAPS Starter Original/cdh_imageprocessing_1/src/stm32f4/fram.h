#pragma once
#include "global.h"

void FRAM_Init();
void FRAM_Cmd(FunctionalState);

uint8_t FRAM_SendByte(uint8_t byte);
uint8_t FRAM_RecvByte(void);