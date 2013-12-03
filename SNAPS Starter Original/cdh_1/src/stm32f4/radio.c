#include "global.h"
#include "radio.h"

static bool _initialized = false;

static void Radio_SendByte(char);

static void Radio_Init()
{
  // TODO: sharing boost converter with camera
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
  // set up TX pin for USART. No need for RX.
  
  GPIO_InitTypeDef GPIO_InitStructure;  
  
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  // connect TX pin to alternate function
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  
  // set up USART data structure
  
  USART_InitTypeDef USART_InitStructure;
  
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);
  
  USART_Cmd(USART3, ENABLE);
  
  
  // set up the \FORCEOFF pin
  
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  _initialized = true;
}

void Radio_Cmd(FunctionalState cmd) {
  if(!_initialized) Radio_Init();
  
  // TODO: shut off boost converter when disabling radio?
  
  // set the FORCEOFF pin
  GPIO_WriteBit(GPIOD, GPIO_Pin_10, (cmd == ENABLE) ? Bit_SET : Bit_RESET);
}

static void Radio_SendByte(char data) {
  USART_SendData(USART3, (uint8_t) data);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
  {}
}

/*
Sends data from the buffer one byte at a time until finished.
*/
void Radio_SendData(char *data, uint32_t length) {
  if(length <= 0) return;
  
  uint32_t i;
  for(i = 0; i < length; i++) {
    Radio_SendByte(data[i]);
  }
}