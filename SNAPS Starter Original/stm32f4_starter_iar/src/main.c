/*
    STM32F4 starter code
	IAR toolchain

    72MHz sysclock w/8MHz crystal
*/

#include <global.h>

// ############################
// ##### GLOBAL VARIABLES #####
// ############################

volatile uint64_t time = 0;

xQueueHandle CanTxQueue;
xQueueHandle CanRxQueue;
// Incoming catalog messages
xQueueHandle CatalogRxQueue;

Catalog cat;

// ##########################
// ##### FREERTOS HOOKS #####
// ##########################

// Hook to increment the system timer
void vApplicationTickHook(void){
    time++;
}

// Hook to execute whenever entering the idle task
// This is a good place for power-saving code.
void vApplicationIdleHook(void){}

// Hook to handle stack overflow
void vApplicationStackOverflowHook(xTaskHandle xTask,
                                   signed portCHAR *pcTaskName){}

// ############################
// ##### HELPER FUNCTIONS #####
// ############################


// #################
// ##### TASKS #####
// #################

void YourCodeHere(void* pvParameters)
{
    vTaskDelay(500);
    while(1)
    {
        printf("%u\tDERP DERP DERP\r\n", (unsigned long)time);
        vTaskDelay(100);
    }
}

// ################
// ##### MAIN #####
// ################

int main()
{
	// Start the CPU cycle counter (read with CYCCNT)
    DWT_CTRL |= DWT_CTRL_CYCEN;
	// Disable unused pins
    GPIO_SetAllAnalog();
    
    // Create CAN message buffers
    CanTxQueue = xQueueCreate(CAN_TXLEN, sizeof(CanTxMsg));
    CanRxQueue = xQueueCreate(CAN_RXLEN, sizeof(CanTxMsg));
    
    // Get some clock information
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);

    printf("STM32F4 starter code\r\n");
    printf("SYSCLK at %u hertz.\r\n", RCC_ClocksStatus.SYSCLK_Frequency);
    printf("HCLK at %u hertz.\r\n", RCC_ClocksStatus.HCLK_Frequency);
    printf("PCLK1 at %u hertz.\r\n", RCC_ClocksStatus.PCLK1_Frequency);
    printf("PCLK2 at %u hertz.\r\n\r\n", RCC_ClocksStatus.PCLK2_Frequency);

    
    catInitializeCatalog(&cat, DEVICE_ID);
    // Add entries here
    catFinishInit(&cat, "Discovery board");
    // Set up tasks and start the scheduler
    xTaskCreate(CatalogReceiveTask, (const signed char*)("CatalogReceiveTask"), 4096, &cat, 1, NULL);
    xTaskCreate(CatalogAnnounceTask, (const signed char *)("CatalogAnnounceTask"), 2048, &cat, 1, NULL);
    // Start the scheduler
    vTaskStartScheduler();

    // Should never get here
    while(1);
}
