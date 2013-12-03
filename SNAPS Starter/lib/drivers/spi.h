#pragma once

#include <stm32f4xx.h>
#include <pin.h>

// Figure out the clock divider
uint32_t SPI_GetDivider(uint32_t coreClock, uint32_t maxSPIclock);

// Set the chip select line
void SPI_SetCS(Pin* pin);

// Reset the chip select line
void SPI_ResetCS(Pin* pin);

// Transfer data
uint16_t SPI_Transfer(SPI_TypeDef* spi, uint16_t value);

// Read a bunch of data
void SPI_ReadMulti( SPI_TypeDef* spi, uint32_t numReads, \
					uint32_t numReadsPerElem, void* target);

// Write a bunch of data
void SPI_WriteMulti(SPI_TypeDef* spi, uint32_t numWrites, \
					uint32_t numWritesPerElem, void* source);

// Transfer a bunch of data
void SPI_TransferMulti(	SPI_TypeDef* spi, uint32_t numTransfers, \
						uint32_t numTransfersPerElem, void* source, \
						void* target);

// Read a bunch of data
void SPI_ReadMulti_Big( SPI_TypeDef* spi, uint32_t numReads, \
					uint32_t numReadsPerElem, void* target);

// Write a bunch of data
void SPI_WriteMulti_Big(SPI_TypeDef* spi, uint32_t numWrites, \
					uint32_t numWritesPerElem, void* source);

// Transfer a bunch of data
void SPI_TransferMulti_Big(	SPI_TypeDef* spi, uint32_t numTransfers, \
						uint32_t numTransfersPerElem, void* source, \
						void* target);
