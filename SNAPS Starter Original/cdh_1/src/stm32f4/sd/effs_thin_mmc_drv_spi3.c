/******************************************************************************
(C) Copyright Pumpkin, Inc. All Rights Reserved.

This file may be distributed under the terms of the License
Agreement provided with this software.

THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.

$Source: C:\\RCS\\D\\Pumpkin\\CubeSatKit\\PIC24\\Src\\effs_thin_mmc_drv_spi1.c,v $
$Author: aek $
$Revision: 3.5 $
$Date: 2012-03-13 11:18:02-08 $

Vaguely related to HCC Embedded's mmc_drv.c (c)2003 for the MSP430F16x. Used 
with  permission.

EFFS-THIN MMC/SD Card driver for CubeSat Kit /PIC24, using SPI1. Two baud rates
are supported, based on the default CSK PPM D1 crystal of 8.000MHz.

******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#include "effs_thin_mmc_drv_spi3.h"
#include "global.h"


// Pumpkin CubeSat Kit
#include "csk_sd.h"

// Semaphore handle
static xSemaphoreHandle _dmaSemaphore;

// Local data.
// The selected baudrate, in Hz
unsigned long baudrate;  


// For setting and reading SPI baud rates:
//  * symbols for init and run mode values
//  * init and running SPI baud rates
//  * valid indeces into the SPI prescalar array
//  * SPI baud rate prescalar array
//  * SPI baud rates array
#define SPI_INIT_MODE_PRESCALAR          	SPI_PRESCALAR_DIV_256 	// 164kHz @ 8MHz
#define SPI_RUN_MODE_PRESCALAR          	SPI_PRESCALAR_DIV_32		// 1MHz @ 8MHz
#define SPI_INIT_SPEED_PRESCALAR         	(prescalars[SPI_INIT_MODE_PRESCALAR])
#define SPI_RUN_SPEED_PRESCALAR          	(prescalars[SPI_RUN_MODE_PRESCALAR])
#define SPI_INIT_BAUD_RATE              	(baudrates[SPI_INIT_MODE_PRESCALAR])
#define SPI_RUN_BAUD_RATE                	(baudrates[SPI_RUN_MODE_PRESCALAR])

#define SPI_PRESCALAR_DIV_256            	0
#define SPI_PRESCALAR_DIV_128            	1
#define SPI_PRESCALAR_DIV_64            	2
#define SPI_PRESCALAR_DIV_32            	3
#define SPI_PRESCALAR_DIV_16            	4
#define SPI_PRESCALAR_DIV_8              	5
#define SPI_PRESCALAR_DIV_4              	6
#define SPI_PRESCALAR_DIV_2              	7

const unsigned int prescalars[SPI_PRESCALAR_DIV_2+1] = { 
  SPI_BaudRatePrescaler_256,    	// :256
  SPI_BaudRatePrescaler_128,    	// :128
  SPI_BaudRatePrescaler_64,    	// :64
  SPI_BaudRatePrescaler_32,    	// :32
  SPI_BaudRatePrescaler_16,    	// :16
  SPI_BaudRatePrescaler_8,    	// :8
  SPI_BaudRatePrescaler_4,    	// :4
  SPI_BaudRatePrescaler_2,    	// :2
  };

const unsigned long baudrates[SPI_PRESCALAR_DIV_2+1] = {
  CSK_SD_OSC / 2 /256,                        
  CSK_SD_OSC / 2 /128,                     
  CSK_SD_OSC / 2 / 64,  
  CSK_SD_OSC / 2 / 32, 
  CSK_SD_OSC / 2 / 16, 
  CSK_SD_OSC / 2 /  8, 
  CSK_SD_OSC / 2 /  4,
  CSK_SD_OSC / 2 /  2, 
  };
   

// User-configurable options.
// NOT CURRENTLY SUPPORTED 2010-04-24 -- change SPI_RUN_MODE_PRESCALAR instead
#define ENABLE_VARIABLE_MAX_BAUDRATE  0

#if ENABLE_VARIABLE_MAX_BAUDRATE
unsigned long spi_max_baudrate = 1000000; // in Hz
#endif


/******************************************************************************
****                                                                       ****
**                                                                           **
ConfigureSPI1()

INPUT: prescalar_bits - bit values for primary and secondary prescalars.

Configure the SPI1 peripheral. Uses Microchip library functions.

**                                                                           **
****                                                                       ****
******************************************************************************/
void ConfigureSPI1(unsigned int prescalar_bits) {
  SPI_InitTypeDef  SPI_InitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  SPI_I2S_DeInit(SPI3);
  
  SPI_StructInit(&SPI_InitStructure);
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = prescalar_bits;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI3, &SPI_InitStructure);
  
  // Enable DMA on TX and RX
  SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Rx, ENABLE);
  
  // Configure DMA channels
  DMA_DeInit(DMA_STREAM_RX);
  DMA_DeInit(DMA_STREAM_TX);
  
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SPI3->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = NULL;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA_STREAM_RX, &DMA_InitStructure);
  
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_Init(DMA_STREAM_TX, &DMA_InitStructure);
  
  // Enable SPI
  SPI_Cmd(SPI3, ENABLE);
  
  vSemaphoreCreateBinary(_dmaSemaphore);
  xSemaphoreTake(_dmaSemaphore, 0);
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA_IT_RX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
} /* ConfigureSPI1() */

void DMA_IT_RX_IRQHandler() {
  portBASE_TYPE needToYield = pdFALSE;
  
  if(DMA_GetITStatus(DMA_STREAM_RX, DMA_IT_RX_TC))
  {
    DMA_ClearITPendingBit(DMA_STREAM_RX, DMA_IT_RX_TC);
    xSemaphoreGiveFromISR(_dmaSemaphore, &needToYield);
  }
  
  portEND_SWITCHING_ISR(needToYield);
}


/******************************************************************************
****                                                                       ****
**                                                                           **
spi_tx1()
spi_tx512()

Transmit 1 byte.
Transmit 512 bytes.

Thanks to user <ohmite> on Microchip forums 20090911 for this method of 
testing for transfer done (by looking at the receiver instead of the IFG flag).

Microchip library functions are not used here, for speed and code size.

**                                                                           **
****                                                                       ****
******************************************************************************/
void spi_tx1(unsigned char data8) {
  const uint8_t dummy = 0x00;
  
  DMA_STREAM_TX->M0AR = (uint32_t)(&data8);
  DMA_STREAM_RX->M0AR = (uint32_t)(&dummy);
  DMA_STREAM_TX->NDTR = DMA_STREAM_RX->NDTR = 1;
  
  DMA_Cmd(DMA_STREAM_RX, ENABLE);
  DMA_Cmd(DMA_STREAM_TX, ENABLE);
  
  while(!DMA_GetFlagStatus(DMA_STREAM_RX, DMA_FLAG_RX_TC)) { }
  
  // Disable DMA channels
  DMA_Cmd(DMA_STREAM_RX, DISABLE);
  DMA_Cmd(DMA_STREAM_TX, DISABLE);
  
  // Clear DMA Stream Transfer Complete interrupt pending bit 
  DMA_ClearFlag(DMA_STREAM_RX, DMA_FLAG_RX_TC);
  DMA_ClearFlag(DMA_STREAM_TX, DMA_FLAG_TX_TC);
} /* spi_tx1() */


void spi_tx512(unsigned char *buf) { 
  const uint8_t dummy = 0x00;
  
  DMA_STREAM_TX->M0AR = (uint32_t)(buf);
  DMA_STREAM_RX->M0AR = (uint32_t)(&dummy);
  DMA_STREAM_TX->NDTR = DMA_STREAM_RX->NDTR = 512;
  
  DMA_STREAM_TX->CR |= DMA_MemoryInc_Enable;
  
  DMA_ITConfig(DMA_STREAM_RX, DMA_IT_TC, ENABLE);
  
  DMA_Cmd(DMA_STREAM_RX, ENABLE);
  DMA_Cmd(DMA_STREAM_TX, ENABLE);
  
  xSemaphoreTake(_dmaSemaphore, portMAX_DELAY);
  
  DMA_ITConfig(DMA_STREAM_RX, DMA_IT_TC, DISABLE);
  
  // Disable DMA channels
  DMA_Cmd(DMA_STREAM_RX, DISABLE);
  DMA_Cmd(DMA_STREAM_TX, DISABLE);
  
  // Clear DMA Stream Transfer Complete interrupt pending bit 
  DMA_ClearFlag(DMA_STREAM_RX, DMA_FLAG_RX_TC);
  DMA_ClearFlag(DMA_STREAM_TX, DMA_FLAG_TX_TC);
  
  DMA_STREAM_TX->CR &= ~DMA_MemoryInc_Enable;
} /* spi_tx512() */


/******************************************************************************
****                                                                       ****
**                                                                           **
spi_rx1()
spi_rx512()

Receive 1 byte.
Receive 512 bytes.

Receiving from an SD Card via SPI is the same as transmitting 0xFF on MOSI 
and reading what came back simultaneously on MISO.

**                                                                           **
****                                                                       ****
******************************************************************************/
unsigned char spi_rx1(void) {
  
  const uint8_t empty = 0xFF;
  const uint8_t byteToReturn = 0x00;
  
  DMA_STREAM_TX->M0AR = (uint32_t)(&empty);
  DMA_STREAM_RX->M0AR = (uint32_t)(&byteToReturn);
  DMA_STREAM_TX->NDTR = DMA_STREAM_RX->NDTR = 1;
  
  DMA_Cmd(DMA_STREAM_RX, ENABLE);
  DMA_Cmd(DMA_STREAM_TX, ENABLE);
  
  while(!DMA_GetFlagStatus(DMA_STREAM_RX, DMA_FLAG_RX_TC)) { }
  
  // Disable DMA channels
  DMA_Cmd(DMA_STREAM_RX, DISABLE);
  DMA_Cmd(DMA_STREAM_TX, DISABLE);
  
  // Clear DMA Stream Transfer Complete interrupt pending bit 
  DMA_ClearFlag(DMA_STREAM_RX, DMA_FLAG_RX_TC);
  DMA_ClearFlag(DMA_STREAM_TX, DMA_FLAG_TX_TC);
  
  // Return byte received
  return byteToReturn;
  
} /* spi_rx1() */


void spi_rx512(unsigned char *buf) {
  const uint8_t empty = 0xFF;
  
  DMA_STREAM_TX->M0AR = (uint32_t)(&empty);
  DMA_STREAM_RX->M0AR = (uint32_t)(buf);
  DMA_STREAM_TX->NDTR = DMA_STREAM_RX->NDTR = 512;
  
  DMA_STREAM_RX->CR |= DMA_MemoryInc_Enable;
  
  DMA_ITConfig(DMA_STREAM_RX, DMA_IT_TC, ENABLE);
  
  DMA_Cmd(DMA_STREAM_RX, ENABLE);
  DMA_Cmd(DMA_STREAM_TX, ENABLE);
  
  xSemaphoreTake(_dmaSemaphore, portMAX_DELAY);
  
  DMA_ITConfig(DMA_STREAM_RX, DMA_IT_TC, DISABLE);
  
  // Disable DMA channels
  DMA_Cmd(DMA_STREAM_RX, DISABLE);
  DMA_Cmd(DMA_STREAM_TX, DISABLE);
  
  // Clear DMA Stream Transfer Complete interrupt pending bit 
  DMA_ClearFlag(DMA_STREAM_RX, DMA_FLAG_RX_TC);
  DMA_ClearFlag(DMA_STREAM_TX, DMA_FLAG_TX_TC);
  
  DMA_STREAM_RX->CR &= ~DMA_MemoryInc_Enable;
} /* spi_rx512() */

void spi_cs_lo()
{
  GPIO_ResetBits(GPIOC, GPIO_Pin_9);
}

void spi_cs_hi()
{
  GPIO_SetBits(GPIOC, GPIO_Pin_9);
}

/******************************************************************************
****                                                                       ****
**                                                                           **
spi_init()

Init SPI peripheral to talk to an SD Card. requires general config of I/O
pins, as well as config of the SPI1 peripheral, and the related PPS pins.

**                                                                           **
****                                                                       ****
******************************************************************************/
int spi_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  
  // Enable -CS_SD (IO.0) control, keep it high / inactive.
  csk_sd_open();  
  spi_cs_hi();  
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  
  // Disable SPI3 module if previously enabled.
  CloseSPI3();    

  
  // 8 MHz (4MHz Fcy) oscillator with 32:1 prescalar yields 125MHz initial
  //  clock speed (must be <400kHz during card identification stage).
  ConfigureSPI1(SPI_INIT_SPEED_PRESCALAR);
  
  // Set baudrate placeholder
  baudrate = SPI_INIT_BAUD_RATE;
  
  // Done, leaving with SPI1 configured, and -CS_SD as an output and high/inactive.
  return 0;

} /* spi_init() */


/******************************************************************************
****                                                                       ****
**                                                                           **
spi_set_baudrate()

Set SPI baudrate.

INPUT: br - baudrate

PIC24 has only coarse-scale tuning of SPI rates -- it has a 64/16/4/1:1 primary 
prescalar and a 8/7/6/5/4/3/2/1:1 secondary prescalar. For simplicity, we
just manhandle the SPI baud rate for two possible operating speeds: 
CSK_SD_INIT_SPEED & CSK_SD_MAX_SPEED, by specifying the appropriate prescalars.

Note the need to CloseSPI1() before changing the baud rates.

**                                                                           **
****                                                                       ****
******************************************************************************/
void spi_set_baudrate(unsigned long br) {
  
  // Send out a sweet nothing and ensure that all xfers are complete.
  spi_tx1(0xFF);
  
  // Either run at <400kHz (startup) or at target-specific maximum SD Card
  //  interface speed.
  if (br != CSK_SD_INIT_SPEED) {
    
    #if ENABLE_VARIABLE_MAX_BAUDRATE
    // Change the prescalar here to something that works with your particular
    //  configuration ...
    //aek CloseSPI1();  
    ConfigureSPI1(prescalars[SPI_PRESCALAR_DIV_2]);   
    baudrate = baudrates[SPI_PRESCALAR_DIV_2];
    
    #else
    // Set baud rate to predefined max rate, based on prescalars.
    CloseSPI3();     
    ConfigureSPI1(SPI_RUN_SPEED_PRESCALAR);    
    baudrate = SPI_RUN_BAUD_RATE;
    #endif
    
  }
  else {
  
    // Set baud rate to identification-phase rate (< 400kHz).
    //aek CloseSPI1();    
    //aek ConfigureSPI1(SPI_INIT_SPEED_PRESCALAR); 
    baudrate = SPI_INIT_BAUD_RATE;
    
  } /* if */
} /* spi_set_baudrate() */


/******************************************************************************
****                                                                       ****
**                                                                           **
spi_get_baudrate()

Get SPI baudrate.

Return: baudrate

**                                                                           **
****                                                                       ****
******************************************************************************/
unsigned long spi_get_baudrate(void) {

  return baudrate;
  
} /* spi_get_baudrate() */



/******************************************************************************
****                                                                       ****
**                                                                           **
spi_set_max_baudrate()

Set maximum SPI baudrate. User can change the baud rate at which EFFS-THIN
interacts with the SD Card by calling this before calling f_initvolume().

Note: No bounds checking!

**                                                                           **
****                                                                       ****
******************************************************************************/
#if ENABLE_VARIABLE_MAX_BAUDRATE

void spi_set_max_baudrate(unsigned long br) {

  spi_max_baudrate = br;

} /* spi_set_max_baudrate() */

#endif /* ENABLE_VARIABLE_MAX_BAUDRATE */


/******************************************************************************
****                                                                       ****
**                                                                           **
spi_get_max_baudrate()

Get maximum SPI baudrate.

Return: baudrate

**                                                                           **
****                                                                       ****
******************************************************************************/
#if ENABLE_VARIABLE_MAX_BAUDRATE

unsigned long spi_get_max_baudrate(void) {

  return spi_max_baudrate;

} /* spi_get_max_baudrate() */

#endif /* ENABLE_VARIABLE_MAX_BAUDRATE */


#ifdef __cplusplus
}
#endif

