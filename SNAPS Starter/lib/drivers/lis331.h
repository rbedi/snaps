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
} LIS331;

// Initialize the peripherals
void LIS331_Init(LIS331* accel);

// Quick existence check to verify that an accelerometer can communicate
uint32_t LIS331_WhoAmI(LIS331* accel);

// Enable the accel
void LIS331_TurnOn(LIS331* accel);

// Read new X, Y, Z angular velocities
void LIS331_UpdateReadings(LIS331* accel);
