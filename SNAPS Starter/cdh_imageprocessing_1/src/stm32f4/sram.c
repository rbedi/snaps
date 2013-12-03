#include "global.h"
#include "sram.h"

static bool _initialized = false;

static void SRAM_Init() {
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  _initialized = true;
}

void SRAM_Cmd(FunctionalState cmd)
{
  if(!_initialized) SRAM_Init();
  
  GPIO_WriteBit(GPIOE, GPIO_Pin_11, (cmd == ENABLE) ? Bit_SET : Bit_RESET);
}