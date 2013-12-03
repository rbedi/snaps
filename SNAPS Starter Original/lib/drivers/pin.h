#pragma once

#include <stm32f4xx.h>

typedef struct
{
   GPIO_TypeDef*    port; 
   uint16_t         pin;
   uint16_t			pinsource;
   uint32_t         clock;
} Pin;

// Atomic set pin high
void Pin_SetHigh(Pin* pin);

// Atomic set pin low
void Pin_SetLow(Pin* pin);

// Non-atomic toggle pin
void Pin_Toggle(Pin* pin);