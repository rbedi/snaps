#include <string.h>
#include <freertos.h>
#include <queue.h>
#include <lea6t.h>

// Initialize the u-blox LEA-6T
void LEA6T_Init(LEA6T* gps)
{
    // Initialize structures
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Initialize queues
    gps->rxQueue = xQueueCreate(512, sizeof(unsigned char));
    gps->txQueue = xQueueCreate(512, sizeof(unsigned char));
    vQueueAddToRegistry(gps->rxQueue, "rxQueue");
    vQueueAddToRegistry(gps->txQueue, "txQueue");

    // Enable the peripheral clocks
    (*(gps->rccfunc))(gps->clock, ENABLE);
    RCC_AHB1PeriphClockCmd(gps->mcutx.clock | gps->mcurx.clock | \
                           gps->enable.clock | gps->pulse.clock, ENABLE);

	// Set enable line as push-pull output
	GPIO_InitStructure.GPIO_Pin = gps->enable.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(gps->enable.port, &GPIO_InitStructure);

    // Connect TX and RX pins to the UART peripheral
    GPIO_PinAFConfig(gps->mcutx.port, gps->mcutx.pinsource, gps->af);
    GPIO_PinAFConfig(gps->mcurx.port, gps->mcurx.pinsource, gps->af);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = gps->mcutx.pin;
    GPIO_Init(gps->mcutx.port, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = gps->mcurx.pin;
    GPIO_Init(gps->mcurx.port, &GPIO_InitStructure);

	// Configure USART interrupt
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = gps->irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Initial USART initialization
	USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(gps->usart, &USART_InitStructure);
    USART_Cmd(gps->usart, ENABLE);

    USART_ITConfig(gps->usart, USART_IT_TC, DISABLE);
    USART_ITConfig(gps->usart, USART_IT_TXE, DISABLE);
    USART_ITConfig(gps->usart, USART_IT_RXNE, ENABLE);
}

// Turn on and configure the u-blox LEA-6T
void LEA6T_TurnOn(LEA6T* gps)
{
	// Turn on power to the GPS
    gps->enable.port->BSRRL |= gps->enable.pin;
}

// ISR to handle serial reception
void LEA6T_UartHandler(LEA6T* gps)
{
    static unsigned char data = 0;

    if(gps->usart->SR & USART_FLAG_RXNE)
	{
        data = gps->usart->DR;
        //printf("%c", data);
        xQueueSendToBackFromISR(gps->rxQueue, &data, 0);
	}/*
    else if(gps->usart->SR & USART_FLAG_TC)
	{
        printf("\r\nTransmit complete?!?\r\n");
	}*/
}