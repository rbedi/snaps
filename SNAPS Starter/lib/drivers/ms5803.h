#pragma once

#include <stm32f4xx.h>
#include <pin.h>

// MS5803 SPI commands
#define MS5803_RESET    0x1E        // Reset the pressure sensor
#define MS5803_SELD1    0x40        // Convert pressure channel
#define MS5803_SELD2    0x50        // Convert temperature channel 
#define MS5803_ADCREAD  0x00        // Read the ADC value 
#define MS5803_PROMREAD 0xA0        // Read the internal PROM

// MS5803 sample depths
#define MS5803_OSR256   0x00
#define MS5803_OSR512   0x02
#define MS5803_OSR1024  0x04
#define MS5803_OSR2048  0x06
#define MS5803_OSR4096  0x08

// Representation of our sensor to pass from higher level code
typedef struct
{
    SPI_TypeDef*    peripheral;
    uint32_t        clock;
    uint8_t         af;
    Pin             mosi;
    Pin             miso;
    Pin             sclk;
    Pin             cs;
    volatile uint64_t*  time;
    uint16_t  	  	prom[8];
	uint32_t		p_raw;
	uint32_t		t_raw;
	int64_t			dt;
	int64_t			temp;
	int64_t			off;
	int64_t			sens;
	int64_t			p;
    float           altitude;
} MS5803;

// Initialize the peripherals
void MS5803_Init(MS5803* sensor);

// Reset the pressure sensor
void MS5803_Reset(MS5803* sensor);

// Retrieve pressure data
void MS5803_UpdatePressure(MS5803* sensor);

// Retrieve temperature data
void MS5803_UpdateTemperature(MS5803* sensor);

// Retrieve factory calibration data
void MS5803_UpdateCalibration(MS5803* sensor);

// First order computations
void MS5803_ComputeFirstOrderCorrections(MS5803* sensor);

// Second order computations
void MS5803_ComputeSecondOrderCorrections(MS5803* sensor);

// Compute altitude
void MS5803_ComputeAltitude(MS5803* sensor);
