#include <ms5803.h>
#include <math.h>

#define TIMEOUT 10

#ifndef INC_FREERTOS_H
    #define Wait(_a, _b)    {uint64_t _c = *(_a->time) + _b; \
                                while(_c > *(_a->time));}
#else
    #define Wait(_a, _b)    {vTaskDelay(_b)};
#endif

// Wait for SPI "busy"
static void MS5803_WaitBSY(MS5803* sensor, uint32_t timeout);

// Wait for SPI "transmit empty"
static void MS5803_WaitTX(MS5803* sensor, uint32_t timeout);

// Wait for SPI "receive not empty"
static void MS5803_WaitRX(MS5803* sensor, uint32_t timeout);

// Set the chip select line
static void MS5803_SetCS(MS5803* sensor);

// Reset the chip select line
static void MS5803_ResetCS(MS5803* sensor);

// Sample an ADC
static uint32_t MS5803_ReadADC(MS5803* sensor, uint32_t channel);

// Read some data
static void MS5803_ReadData(MS5803* sensor, uint32_t numBytes, \
                            uint32_t elemSize, uint32_t elems, \
                            void* target);

// Wait for SPI "busy"
static void MS5803_WaitBSY(MS5803* sensor, uint32_t timeout)
{
    uint64_t waitUntil = *(sensor->time) + timeout;
    SPI_TypeDef* spi = sensor->peripheral;

    while(  SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) == SET && \
            *(sensor->time) < waitUntil );
}

// Wait for SPI "transmit empty"
static void MS5803_WaitTX(MS5803* sensor, uint32_t timeout)
{
    uint64_t waitUntil = *(sensor->time) + timeout;
    SPI_TypeDef* spi = sensor->peripheral;

    while(  SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET && \
            *(sensor->time) < waitUntil );
}

// Wait for SPI "receive not empty"
static void MS5803_WaitRX(MS5803* sensor, uint32_t timeout)
{
    uint64_t waitUntil = *(sensor->time) + timeout;
    SPI_TypeDef* spi = sensor->peripheral;

    while(  SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET && \
            *(sensor->time) < waitUntil );
}

// Set the chip select line
static void MS5803_SetCS(MS5803* sensor)
{
    sensor->cs.port->BSRRH |= sensor->cs.pin;
}

// Reset the chip select line
static void MS5803_ResetCS(MS5803* sensor)
{
    sensor->cs.port->BSRRL |= sensor->cs.pin;
}

// Sample an ADC
static uint32_t MS5803_ReadADC(MS5803* sensor, uint32_t channel)
{
    uint32_t toReturn;
	
	// Select and convert a channel
	MS5803_SetCS(sensor);
	SPI_I2S_SendData(sensor->peripheral, channel);
	// Wait for the conversion
    Wait(sensor, 10);
	MS5803_ResetCS(sensor);
	
	// Instruct the sensor to read the converted value
	MS5803_SetCS(sensor);
	SPI_I2S_SendData(sensor->peripheral, MS5803_ADCREAD);
	MS5803_WaitTX(sensor, TIMEOUT);
	
	// Read back the data
	MS5803_ReadData(sensor, 3, sizeof(uint32_t), 1, &toReturn);
    MS5803_ResetCS(sensor);

    return toReturn;
}

// Read some data
// Expect integer number of bytes to receive per element with no remainder
// MSB is first
static void MS5803_ReadData(MS5803* sensor, uint32_t numBytes, \
                            uint32_t elemSize, uint32_t elems, \
                            void* target)
{
    // Clear the receive buffer so we don't get false data
    MS5803_WaitRX(sensor, TIMEOUT);
    SPI_I2S_ReceiveData(sensor->peripheral);
    
    // Iterate over the provided array and populate it
    uint32_t bytesPerElem = numBytes / elems;
    for(uint32_t a = 0; a < elems; a++)
    {
        for(uint32_t b = bytesPerElem ; b > 0; b--)
        {
            // Bang out the clocks and receive the data
            SPI_I2S_SendData(sensor->peripheral, 0);
            MS5803_WaitTX(sensor, TIMEOUT);
            MS5803_WaitRX(sensor, TIMEOUT);
            
            // Little-endian-ify the data
			uint32_t index = (a*elemSize) + (b - 1);
            uint8_t val = SPI_I2S_ReceiveData(sensor->peripheral);
            ((uint8_t*)target)[index] = val;
        }
    }
}

// Initialize the peripherals
void MS5803_Init(MS5803* sensor) 
{
    // Initialization structures
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // Enable the peripheral clocks
    RCC_APB1PeriphClockCmd( sensor->clock, ENABLE);
    RCC_AHB1PeriphClockCmd( sensor->mosi.clock | sensor->miso.clock | \
                            sensor->cs.clock, ENABLE);

    // Connect the the pins to the SPI peripheral
    GPIO_PinAFConfig(sensor->mosi.port, sensor->mosi.pinsource, sensor->af);
    GPIO_PinAFConfig(sensor->miso.port, sensor->miso.pinsource, sensor->af); 
    GPIO_PinAFConfig(sensor->sclk.port, sensor->sclk.pinsource, sensor->af);

    // Set up GPIO for CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = sensor->cs.pin;
    GPIO_Init(sensor->cs.port, &GPIO_InitStructure);

    // Set up GPIO for SCLK and MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Pin = sensor->sclk.pin;
    GPIO_Init(sensor->sclk.port, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = sensor->mosi.pin;
    GPIO_Init(sensor->mosi.port, &GPIO_InitStructure);

    // Set up GPIO for MISO
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = sensor->miso.pin;
    GPIO_Init(sensor->miso.port, &GPIO_InitStructure);

    // Configure SPI peripheral
    SPI_I2S_DeInit(sensor->peripheral);
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_Init(sensor->peripheral, &SPI_InitStructure);
    SPI_Cmd(sensor->peripheral, ENABLE);
}

// Reset the pressure sensor
void MS5803_Reset(MS5803* sensor)
{
    // Transmit the reset instruction
    MS5803_SetCS(sensor);
    SPI_I2S_SendData(sensor->peripheral, MS5803_RESET);
    // We have to wait at least 2.8ms while the part resets
    Wait(sensor, 3);
    // End the sequence
    MS5803_ResetCS(sensor);
}

// Retrieve pressure data
void MS5803_UpdatePressure(MS5803* sensor)
{
    uint32_t addr = MS5803_SELD1 | MS5803_OSR4096;
	sensor->p_raw = (int32_t)MS5803_ReadADC(sensor, addr);
}

// Retrieve temperature data
void MS5803_UpdateTemperature(MS5803* sensor)
{
    uint32_t addr = MS5803_SELD2 | MS5803_OSR4096;
	sensor->t_raw = MS5803_ReadADC(sensor, addr);
}

// Retrieve factory calibration data
void MS5803_UpdateCalibration(MS5803* sensor)
{
	for(uint32_t a = 0; a < 8; a++)
	{
		MS5803_SetCS(sensor);
		SPI_I2S_SendData(sensor->peripheral, MS5803_PROMREAD | (a << 1));
		MS5803_WaitTX(sensor, TIMEOUT);
        MS5803_ReadData(sensor, 2, sizeof(sensor->prom[a]), 1, \
						&(sensor->prom[a]));
		MS5803_ResetCS(sensor);
	}
}

// First order computations
void MS5803_ComputeFirstOrderCorrections(MS5803* sensor)
{
    // Compute temperature
  	sensor->dt = sensor->t_raw - ((int64_t)sensor->prom[5] << 8);
	sensor->temp = (int64_t)2000 + ((sensor->dt * (int64_t)sensor->prom[6]) \
					>> 23);

    // Compute pressure
    sensor->off = 	((int64_t)sensor->prom[2] << 16) + \
					(((int64_t)sensor->prom[4] * sensor->dt) >> 7);
	sensor->sens = 	((int64_t)sensor->prom[1] << 15) + \
					(((int64_t)sensor->prom[3] * sensor->dt) >> 8);
	sensor->p = 	((((int64_t)sensor->p_raw * sensor->sens) >> 21) \
					- sensor->off) >> 15;
}

// Second order computations
void MS5803_ComputeSecondOrderCorrections(MS5803* sensor)
{
    int64_t t2 = 0, off2 = 0, sens2 = 0;

	// This follows the logic tree for second order temperature corrections
    if(sensor->temp > 4500)
    {
        int64_t newtemp = sensor->temp - 4500;
        sens2 = sens2 - ((newtemp * newtemp) >> 3);
    }
    else if(sensor->temp >= 2000)
        return;
    else if(sensor->temp < 2000)
    {
        int64_t sqtemp = (sensor->temp - 2000) * (sensor->temp - 2000);

        t2 = (sensor->dt * sensor->dt) >> 31;
        off2 = 3 * sqtemp;
        sens2 = 7 * sqtemp >> 3;
        
        if(sensor->temp < -1500)
            sens2 += (2 * (sensor->temp + 1500) * (sensor->temp + 1500));
    }

    // Integrate our newly computed values in to previously acquired values
    sensor->temp -= t2;
    sensor->off -= off2;
    sensor->sens -= sens2;

    // Recompute pressure
   	sensor->p = 	((((int64_t)sensor->p_raw * sensor->sens) >> 21) \
					- sensor->off) >> 15;
}

// Compute altitude
void MS5803_ComputeAltitude(MS5803* sensor)
{
    // This is an implementation of the hypsometric formula
    float a = powf((1013.25f / (sensor->p / 100.0f)), (1.0f / 5.257f)) - 1;
    float b = (sensor->temp / 100.0f) + 273.15;
    
    sensor->altitude = a * b / .0065f;

}
