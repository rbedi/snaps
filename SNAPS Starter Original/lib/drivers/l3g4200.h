#pragma once

#include <stm32f4xx.h>
#include <pin.h>

typedef struct
{
    SPI_TypeDef*    spi;
    uint32_t        clock;
	void			(*rccfunc)(uint32_t peripheral, FunctionalState NewState);
    uint8_t         af;
    Pin             mosi;
    Pin             miso;
    Pin             sclk;
    Pin             cs;
	volatile uint64_t*  time;
	int16_t			x;
	int16_t			y;
	int16_t			z;
} L3G4200;

// Initialize the peripherals
void L3G4200_Init(L3G4200* gyro);

// Quick existence check to verify that a gyro can communicate
uint32_t L3G4200_WhoAmI(L3G4200* gyro);

// Enable the gyro
void L3G4200_TurnOn(L3G4200* gyro);

// Read new X, Y, Z angular velocities
void L3G4200_UpdateReadings(L3G4200* gyro);
