#pragma once

#include "global.h"
#include <stdint.h>

#define SPI             SPI1
#define SPI_CLK         RCC_APB2Periph_SPI1

#define SPI_SCK_PIN               GPIO_Pin_5                  /* PA.05 */
#define SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
#define SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define SPI_SCK_SOURCE            GPIO_PinSource5
#define SPI_SCK_AF                GPIO_AF_SPI1

#define SPI_MISO_PIN              GPIO_Pin_6                  /* PA.6 */
#define SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */
#define SPI_MISO_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define SPI_MISO_SOURCE           GPIO_PinSource6
#define SPI_MISO_AF               GPIO_AF_SPI1

#define SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.7 */
#define SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define SPI_MOSI_SOURCE           GPIO_PinSource7
#define SPI_MOSI_AF               GPIO_AF_SPI1

#define SPI_CS_PIN                GPIO_Pin_3                  /* PE.03 */
#define SPI_CS_GPIO_PORT          GPIOE                       /* GPIOE */
#define SPI_CS_GPIO_CLK           RCC_AHB1Periph_GPIOE

#define CS_LOW()       GPIO_ResetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)
#define CS_HIGH()      GPIO_SetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)

void SPI_Wrapper_Init(void);
void SPI_Wrapper_SendBuffer(const void *send_buffer, size_t length, void * recv_buffer);