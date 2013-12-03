/******************************************************************************
(C) Copyright Pumpkin, Inc. All Rights Reserved.

This file may be distributed under the terms of the License
Agreement provided with this software.

THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.

$Source: C:\\RCS\\D\\Pumpkin\\CubeSatKit\\PIC24\\Src\\csk_sd.c,v $
$Author: aek $
$Revision: 3.1 $
$Date: 2010-01-24 20:36:56-08 $

******************************************************************************/
#include "global.h"
#include "csk_sd.h"
//aek #include "csk_hw.h"
#include "../camera.h"

static uint8_t _initialized;
static uint8_t _pwr_on; 

static void csk_sd_init(void);
static void csk_sd_tristate(FunctionalState cmd);

uint8_t csk_sd_is_pwr_on()
{
  return _pwr_on;
}

static void csk_sd_init()
{
  // Configure SD enable
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  GPIO_SetBits(GPIOC, GPIO_Pin_6); // Disable SD card by default
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOC, GPIO_Pin_9); // Chip select high by default
  
  // Configure I/O pins
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
  
  csk_sd_tristate(ENABLE);
  
  _initialized = 1;
}

static void csk_sd_tristate(FunctionalState cmd)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = cmd ? GPIO_Mode_AIN : GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = cmd ? GPIO_Mode_AIN : GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/******************************************************************************
****                                                                       ****
**                                                                           **
csk_sd_open()

Open the SD Card for use. Just makes -CS_SD an output, leaves it inactive.

Note: Requires that SD Card be powered prior to use.


**                                                                           **
****                                                                       ****
******************************************************************************/
void csk_sd_open(void) {

  if(!_initialized)
    csk_sd_init();
  
  // Configure -CS_SD as an output, and make
  //  it inactive (i.e., HIGH)
  //aek PORTE |=  CS_SD_BAR;
  //aek TRISE &= ~CS_SD_BAR;

} /* csk_sd_open() */


/******************************************************************************
****                                                                       ****
**                                                                           **
csk_sd_close()

Close the SD Card.

**                                                                           **
****                                                                       ****
******************************************************************************/
void csk_sd_close(void) {
  
  if(!_initialized)
    csk_sd_init();
  

  // Restore -CS_SD to an input. It's therefore pulled HIGH by its pull-up 
  //  resistor.
  //aek PORTE |=  CS_SD_BAR;
  //aek TRISE |=  CS_SD_BAR;

} /* csk_sd_close() */


/******************************************************************************
****                                                                       ****
**                                                                           **
csk_sd_pwr_on()

Power on the SD Card.
Return false if the camera could not be powered off.

**                                                                           **
****                                                                       ****
******************************************************************************/
uint8_t csk_sd_pwr_on(void) {
  
  if(!_initialized)
    csk_sd_init();
  
  // return true if already on
  if(_pwr_on)
    return 1;
  
  // return false if the camera is enabled
  if(Camera_Is_Enabled())
    return 0;
  
  // Disable tristate on SPI pins
  csk_sd_tristate(DISABLE);
  // Power to SD card
  GPIO_ResetBits(GPIOC, GPIO_Pin_6);
  
  // pwr is on
  _pwr_on = 1;
  
  return 1;
  
} /* csk_sd_pwr_on() */


/******************************************************************************
****                                                                       ****
**                                                                           **
csk_sd_pwr_off()

Power off the SD Card.
Return false if we couldn't turn off the SD card 
  (because MCU pins were not yet tri-stated)

**                                                                           **
****                                                                       ****
******************************************************************************/
uint8_t csk_sd_pwr_off(void) {
  
  if(!_initialized)
    csk_sd_init();
  
  // return 1 if already off
  if(!_pwr_on)
    return 1;
  
  // enable tristate on SPI pins
  csk_sd_tristate(ENABLE);
  
  // disable 3.3V to SD card
  GPIO_SetBits(GPIOC, GPIO_Pin_6);
  
  _pwr_on = 0;
  
  return 1;
  
} /* csk_sd_pwr_off() */
