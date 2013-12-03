#include "global.h"
#include "camera.h"

#define TURN_ON_TIME    5000

static bool _initialized = false;
static bool _recording = false;

static void Camera_Toggle_Record(void);

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
  
  _initialized = true;
}

void Camera_Cmd(FunctionalState cmd) {
  if(!_initialized) Camera_Init();
  
  // set the LDO enable pin
  GPIO_WriteBit(GPIOD, GPIO_Pin_6, (cmd == ENABLE) ? Bit_SET : Bit_RESET);
}

/*
Records video for the specified amount of time (in milliseconds)
*/
void Camera_Record_Snippet(uint32_t timeToRecord) {
  if(!_initialized) Camera_Init();
  
  // give camera time to start up
  vTaskDelay(6000);
  // start recording, delay, stop recording
  Camera_Toggle_Record();
  vTaskDelay(timeToRecord + TURN_ON_TIME);
  Camera_Toggle_Record();
}

static void Camera_Toggle_Record(void) {
  GPIO_WriteBit(GPIOG, GPIO_Pin_13, Bit_SET);
  vTaskDelay(500);
  GPIO_WriteBit(GPIOG, GPIO_Pin_13, Bit_RESET);
  
  _recording = !_recording;
}

bool Camera_Is_Recording(void) {
  return _recording;
}

bool Camera_Power_Good(void) {
  return GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_11);
}