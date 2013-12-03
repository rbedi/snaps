#include "global.h"
#include "core.h"
#include "boost.h"
#include "sram.h"
#include "camera.h"
#include "radio.h"
#include "adc.h"
#include "fram.h"

void Core_Init()
{
  // Initialize the boost converter driver
  Boost_Cmd(ENABLE);
  // Initialize the camera driver
  Camera_Cmd(DISABLE);
  // Initialize the radio driver
  Radio_Cmd(ENABLE);
  // Initialize the adc driver
  adc_Cmd(ENABLE);
  // Initialize the fram driver
  // FRAM_Cmd(ENABLE);
  
  // uint8_t data = 0x9F;
  // uint8_t recv = FRAM_SendByte(data);
  
  // Enable the SRAM
  SRAM_Cmd(ENABLE);
}