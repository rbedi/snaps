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

static uint8_t _initialized;
static void csk_sd_init(void);

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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  _initialized = 1;
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

**                                                                           **
****                                                                       ****
******************************************************************************/
void csk_sd_pwr_on(void) {
  
  if(!_initialized)
    csk_sd_init();
  
  GPIO_ResetBits(GPIOC, GPIO_Pin_6); // 3.3V to SD card
  
} /* csk_sd_pwr_on() */


/******************************************************************************
****                                                                       ****
**                                                                           **
csk_sd_pwr_off()

Power off the SD Card.

**                                                                           **
****                                                                       ****
******************************************************************************/
void csk_sd_pwr_off(void) {
  
  if(!_initialized)
    csk_sd_init();

  GPIO_SetBits(GPIOC, GPIO_Pin_6); // Disable 3.3V to SD card
  
} /* csk_sd_pwr_off() */
