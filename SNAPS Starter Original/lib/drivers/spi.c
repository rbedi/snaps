#include <spi.h>

// Figure out the clock divider
uint32_t SPI_GetDivider(uint32_t coreClock, uint32_t maxSPIclock)
{
    uint32_t ratio = coreClock / maxSPIclock;

    // Iterate
    for(uint32_t a = 1; a <= 8; a++)
        if(ratio >> a == 0)
            return (a - 1) << 3;

    // Give up and return /256
    return 7 << 3;
}

// Wait for SPI "transmit empty"
static inline void SPI_WaitTX(SPI_TypeDef* spi)
{
    while(!(spi->SR & SPI_I2S_FLAG_TXE));
}

// Wait for SPI "receive not empty"
static inline void SPI_WaitRX(SPI_TypeDef* spi)
{
    while(!(spi->SR & SPI_I2S_FLAG_RXNE));
}

// Set the chip select line
void SPI_SetCS(Pin* pin)
{
    pin->port->BSRRH |= pin->pin;
}

// Reset the chip select line
void SPI_ResetCS(Pin* pin)
{
    pin->port->BSRRL |= pin->pin;
}

// Transfer data
inline uint16_t SPI_Transfer(SPI_TypeDef* spi, uint16_t value)
{
	spi->DR = value;
	SPI_WaitTX(spi);
	SPI_WaitRX(spi);
	return spi->DR;
}

// Read a bunch of data (8 bits at a time)
void SPI_ReadMulti( SPI_TypeDef* spi, uint32_t numReads, \
					uint32_t numReadsPerElem, void* target)
{
	uint32_t numElements = numReads / numReadsPerElem;

	// Clock in data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = 0; b < numReadsPerElem; b++)
			((uint8_t*)target)[a * numReadsPerElem + b] = SPI_Transfer(spi, 0);
}

// Write a bunch of data (8 bits at a time)
void SPI_WriteMulti(SPI_TypeDef* spi, uint32_t numWrites, \
					uint32_t numWritesPerElem, void* source)
{
	uint32_t numElements = numWrites / numWritesPerElem;

	// Clock out data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = 0; b < numWritesPerElem; b++)
			SPI_Transfer(spi, ((uint8_t*)source)[a * numWritesPerElem + b]);
}

// Transfer a bunch of data (8 bits at a time)
void SPI_TransferMulti(	SPI_TypeDef* spi, uint32_t numTransfers, \
						uint32_t numTransfersPerElem, void* source, \
						void* target)
{
	uint32_t numElements = numTransfers / numTransfersPerElem;

	// Clock in data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = 0; b < numTransfersPerElem; b++)
			((uint8_t*)target)[a * numTransfersPerElem + b] = SPI_Transfer( \
			spi, ((uint8_t*)source)[a * numTransfersPerElem + b]);
}

// Read a bunch of data (8 bits at a time)
// Big endian version
void SPI_ReadMulti_Big(SPI_TypeDef* spi, uint32_t numReads, \
					uint32_t numReadsPerElem, void* target)
{
	uint32_t numElements = numReads / numReadsPerElem;

	// Clock in data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = numReadsPerElem; b > 0; b--)
			((uint8_t*)target)[a * numReadsPerElem + b - 1] = \
				SPI_Transfer(spi, 0);
}

// Write a bunch of data (8 bits at a time)
// Big endian version
void SPI_WriteMulti_Big(SPI_TypeDef* spi, uint32_t numWrites, \
					uint32_t numWritesPerElem, void* source)
{
	uint32_t numElements = numWrites / numWritesPerElem;

	// Clock out data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = numWritesPerElem; b > 0; b--)
			SPI_Transfer(spi, ((uint8_t*)source)[a * numWritesPerElem + b - 1]);
}

// Transfer a bunch of data (8 bits at a time)
// Big endian version
void SPI_TransferMulti_Big(	SPI_TypeDef* spi, uint32_t numTransfers, \
						uint32_t numTransfersPerElem, void* source, \
						void* target)
{
	uint32_t numElements = numTransfers / numTransfersPerElem;

	// Clock in data
	for(uint32_t a = 0; a < numElements; a++)
		for(uint32_t b = numTransfersPerElem; b > 0; b--)
			((uint8_t*)target)[a * numTransfersPerElem + b - 1] = \
				SPI_Transfer \
				(spi, ((uint8_t*)source)[a * numTransfersPerElem + b - 1]);
}
