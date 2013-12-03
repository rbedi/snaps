#pragma once

#include <stm32f4xx.h>
#include <pin.h>

typedef struct
{
	USART_TypeDef* usart;
	uint32_t clock;
	void (*rccfunc)(uint32_t peripheral, FunctionalState NewState);
	uint8_t af;
	int32_t irq;
	Pin mcutx;
	Pin mcurx;
	Pin enable;
	Pin pulse;
	volatile uint64_t*  time;
	xQueueHandle rxQueue;
	xQueueHandle txQueue;
} LEA6T;

// Initialize the u-blox LEA-6T
void LEA6T_Init(LEA6T* gps);

// Turn on and configure the u-blox LEA-6T
void LEA6T_TurnOn(LEA6T* gps);

// ISR to handle serial reception
void LEA6T_UartHandler(LEA6T* gps);