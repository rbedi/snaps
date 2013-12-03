#include "global.h"
#include "camera.h"
#include "sd/csk_sd.h"

#define TURN_ON_TIME    5000

static bool _initialized = false;
static bool _recording = false; 
static uint8_t _enabled;

static void Camera_Toggle_Record(void);
static void Camera_Set_Dual_Port(FunctionalState);

static void Camera_Init()
{
  // set up the 5v LDO SHDN pin
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_StructInit(&GPIO_InitStructure);
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  // set up the record_EN pin (GP13)
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  // set up the PWR_GOOD pin of the 5V regulator

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  // set up the SD card dual-port enable pin
  
  GPIO_WriteBit(GPIOG, GPIO_Pin_12, Bit_SET);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  _initialized = true;
}

/*
Returns true if the command is successful. 

Might return false if we try to enable the camera and the MCU is still using the SD card
*/
bool Camera_Cmd(FunctionalState cmd) {
  if(!_initialized) Camera_Init();
  
  if(cmd == ENABLE) {
    // attempt to turn off csk_sd's access to the SD card
    // abort if the csk_sd interface says it isn't done with the SD card
    //if(csk_sd_is_pwr_on())
    //  return false;
    
    if(_enabled)
      return true;
    
    // enable the dual-port (i.e. give the camera access to the MCU
    Camera_Set_Dual_Port(ENABLE);
    
    // turn on power to the camera
    GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_SET);
    
    // physically turn on the SD card
    GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
    
    _enabled = 1;
  }
  else {
    // don't allow turn off if we are recording.
    if(_recording)
      return false;
    
    if(!_enabled)
      return true;
    
    // turn everything related to the camera off
    Camera_Set_Dual_Port(DISABLE);
    GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
    
    _enabled = 0;
  }
  
  // return true to indicate that the command succeeded
  return true;
}

/*
Records video for the specified amount of time (in milliseconds)
*/
void Camera_Record_Snippet(uint32_t timeToRecord) {
  if(!_initialized) Camera_Init();
  if(!_enabled) return;
  
  // TODO: ensure that this function is being called from a FreeRTOS thread?
  
  // make a note that we are recording. This prevents the csk_sd from enabling SPI pins
  _recording = true;
  
  // give camera time to start up
  vTaskDelay(6000);
  // start recording, delay, stop recording
  Camera_Toggle_Record();
  vTaskDelay(timeToRecord + TURN_ON_TIME);
  Camera_Toggle_Record();
  
  _recording = false;
}

static void Camera_Toggle_Record(void) {
  GPIO_WriteBit(GPIOG, GPIO_Pin_13, Bit_SET);
  vTaskDelay(500);
  GPIO_WriteBit(GPIOG, GPIO_Pin_13, Bit_RESET);
}

/*
Manipulates the enable pin of the dual-porting switch. 
Performs no checks if it is safe to do so.
*/
uint8_t Camera_Is_Enabled(void)
{
  return _enabled;
}
static void Camera_Set_Dual_Port(FunctionalState cmd) {
  GPIO_WriteBit(GPIOG, GPIO_Pin_12, (cmd == ENABLE) ? Bit_RESET : Bit_SET);
}

bool Camera_Power_Good(void) {
  return GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_11);
}
