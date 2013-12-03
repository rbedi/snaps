/*
    STM32F4 starter code
	IAR toolchain

    72MHz sysclock w/8MHz crystal
*/

#include "global.h"
#include "config.h"
#include "spi.h"

volatile uint64_t time;

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


static void SPITask(void *);

int main()
{
	// Start the CPU cycle counter (read with CYCCNT)
    DWT_CTRL |= DWT_CTRL_CYCEN;
	// Disable unused pins
    GPIO_SetAllAnalog();
        
    xTaskCreate(SPITask, (const signed char*)("SPITask"), 200, NULL, 2, NULL);

    // Start the scheduler
    vTaskStartScheduler();

    // Should never get here
    while(1);
}

static void SPITask(void *blah) {
  SPI_Wrapper_Init();
      
  const uint8_t dataout[] = { 0xaa, 0xbb, 0x12, 0x34, 0xff };
  
  while(1) {
    SPI_Wrapper_SendBuffer(dataout, 5, NULL);
    vTaskDelay(100);
  }
}
