#include "global.h"
#include "fram.h"

static bool _initialized = false;

static void FRAM_CS_High(void);
static void FRAM_CS_Low(void);

void FRAM_Init() {
  GPIO_InitTypeDef GPIO_InitStructure;

  // enable RCC clocks
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

  // configure SCK, MOSI, MISO for alternate function of SPI1
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  // configure SCK pin
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // configure MISO pin
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // configure MOSI pin
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // configure CS pin on PG15
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  // set up SPI1 peripheral
  
  SPI_InitTypeDef  SPI_InitStructure;

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
  
  _initialized = true;
}

void FRAM_Cmd(FunctionalState cmd) {
  if(!_initialized) FRAM_Init();
  
  // set the CS line appropriately
  (cmd == ENABLE) ? FRAM_CS_Low() : FRAM_CS_High();
}

static void FRAM_CS_Low(void) {
  GPIO_WriteBit(GPIOG, GPIO_Pin_15, Bit_RESET);
}

static void FRAM_CS_High(void) {
  GPIO_WriteBit(GPIOG, GPIO_Pin_15, Bit_SET);
}

uint8_t FRAM_ReadByte(void){
  // TODO: strip out these CS changes. Make a TX and RX buffer function
  FRAM_CS_Low();
  
  uint8_t rxByte = FRAM_SendByte(0xFF);
  
  FRAM_CS_High();
  
  return rxByte;
}

uint8_t FRAM_SendByte(uint8_t byte) {
  
  // TODO: strip out these CS changes. Make a TX and RX buffer function
  
  // loop until the DR register is empty
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  
  FRAM_CS_Low();
  
  SPI_I2S_SendData(SPI1, byte);
  
  // wait to receive a byte
  //while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  
  //wait for a bit
  uint32_t i;
  for(i = 0; i < 1000; i++)
    asm("nop");
  
  FRAM_CS_High();
  
  // return the received byte
  return SPI_I2S_ReceiveData(SPI1);
  
}
