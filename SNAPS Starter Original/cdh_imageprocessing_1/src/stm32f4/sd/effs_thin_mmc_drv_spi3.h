/******************************************************************************
(C) Copyright Pumpkin, Inc. All Rights Reserved.

This file may be distributed under the terms of the License
Agreement provided with this software.

THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.

$Source: C:\\RCS\\D\\Pumpkin\\CubeSatKit\\PIC24\\Inc\\effs_thin_mmc_drv_spi1.h,v $
$Author: aek $
$Revision: 3.2 $
$Date: 2011-09-17 17:13:00-07 $

Derived from HCC Embedded's mmc_drv.h (c)2003 for the MSP430F16x. Used with 
permission.

EFFS-THIN MMC/SD Card driver for CubeSat Kit /PIC24, using SPI1.

******************************************************************************/
#ifndef __effs_thin_mmc_drv_spi1_h
#define __effs_thin_mmc_drv_spi1_h


#ifdef __cplusplus
extern "C" {
#endif


#include "global.h"


// EFFS-THIN speed settings
#define CSK_SD_OSC          8000000     // Hz, CSK's external oscillator (no PLL active)
#define CSK_SD_INIT_SPEED    100000     // Hz, EFFS-THIN default (conservative)
#define CSK_SD_MAX_SPEED    1000000     // Hz, for most CSK hardware


// MMC driver function declarations.
         int  spi_init(void);    
         int  spi_delete(void);                
         void spi_set_baudrate(unsigned long);  
         void spi_set_max_baudrate(unsigned long);  
unsigned long spi_get_baudrate(void);  
unsigned long spi_get_max_baudrate(void);  
         void spi_tx1(unsigned char);
         void spi_tx512(unsigned char *);
unsigned char spi_rx1(void);  
         void spi_rx512(unsigned char *);
         void spi_cs_lo(void);
         void spi_cs_hi(void);
         

#define DMA_STREAM_RX DMA1_Stream0
#define DMA_STREAM_TX DMA1_Stream5
#define DMA_FLAG_RX_TC DMA_FLAG_TCIF0
#define DMA_FLAG_TX_TC DMA_FLAG_TCIF5
#define DMA_IT_RX_TC DMA_IT_TCIF0
#define DMA_IT_TX_TC DMA_IT_TCIF5
#define DMA_IT_RX_IRQn DMA1_Stream0_IRQn
#define DMA_IT_RX_IRQHandler DMA1_Stream0_IRQHandler
         
#define CloseSPI3()  (SPI_I2S_DeInit(SPI3))

#define get_cd()            1
#define get_wp()            0


#ifdef __cplusplus
}
#endif


#endif /* __effs_thin_mmc_drv_spi1_h */

