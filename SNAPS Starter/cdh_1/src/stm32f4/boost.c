#include "global.h"
#include "boost.h"

static bool _initialized = false;

static void Boost_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  // set up PGOOD pin
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  
  // set up SHDN pin of the protection circuit that follows the boost
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
  
  _initialized = true;
}

void Boost_Cmd(FunctionalState cmd) {
  if(!_initialized) Boost_Init();
  
  // set the SHDN pin of the protection circuit
  GPIO_WriteBit(GPIOC, GPIO_Pin_0, (cmd == ENABLE) ? Bit_RESET : Bit_SET);
}

bool Boost_PowerGood(void)
{
  return !GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_7);
}