#include <stm32f4xx.h>
#include "config.h"

// Set all GPIO banks to analog in and disabled to save power
void GPIO_SetAllAnalog()
{
    // Data structure to represent GPIO configuration information
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable the peripheral clocks on all GPIO banks (needed during configuration)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
        RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |
        RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH |
        RCC_AHB1Periph_GPIOI, ENABLE);

    // Set all GPIO banks to analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    GPIO_Init(GPIOH, &GPIO_InitStructure);
    GPIO_Init(GPIOI, &GPIO_InitStructure);

    // Do these separately so we don't break jtag
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All & ~GPIO_Pin_13 & ~GPIO_Pin_14 & ~GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All & ~GPIO_Pin_3 & ~GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Disable the clocks to save power
    // Enable the peripheral clocks on all GPIO banks (needed during configuration)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
        RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |
        RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH |
        RCC_AHB1Periph_GPIOI, DISABLE);
}