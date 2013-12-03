#pragma once
#include "global.h"

#define ADC_NUM_CHANNELS        4

#define ADC_SNS_3V0         0
#define ADC_SNS_5V0         1
#define ADC_SNS_VBATT       2
#define ADC_SNS_VBOOST      3
#define ADC_SNS_VBUS        4

void adc_Init();
void adc_Cmd(FunctionalState);
float adc_GetChannelVoltage(uint8_t channelNum);