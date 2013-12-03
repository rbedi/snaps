#include "global.h"
#include "util.h"
#include "stm32f4/core.h"
#include "usb_controller.h"
#include "ff.h"
#include "stm32f4/radio.h"
#include "stm32f4/adc.h"

// ############################
// ##### GLOBAL VARIABLES #####
// ############################

volatile uint64_t time_ms = 0;

// ##########################
// ##### FREERTOS HOOKS #####
// ##########################

// Hook to increment the system timer
void vApplicationTickHook(void){
    time_ms++;
    if(!(time_ms % 10))
      disk_timerproc();
}

// Hook to execute whenever entering the idle task
// This is a good place for power-saving code.
void vApplicationIdleHook(void) { __WFI(); }

// Hook to handle stack overflow
void vApplicationStackOverflowHook(xTaskHandle xTask,
                                   signed portCHAR *pcTaskName){}

// ################
// ##### MAIN #####
// ################

#pragma location = "SRAMSection" // Place the next variable in SRAM
uint8_t mem[128]; // This is placed in SRAM
uint8_t k; // This is placed in normal memory because it isn't preceded with the #pragma location directive

static char strTmp[256];

void rs232_task(void* pv) {
  char *data = "hello\0";
  while(1)
  {    
    Radio_SendData(data, strlen(data));
    vTaskDelay(100);
  }
}

void adc_task(void* pv) {
  float reading = 0;
  while(1)
  {    
    reading = adc_GetChannelVoltage(ADC_SNS_3V0);
    vTaskDelay(100);
  }
}

void image_process_task(void* pv) {
  
  FATFS fs;
  f_mount(0, &fs);
  
  image_main();
  
  //static uint8_t buffer[20] = "aabbccdd12aabbccdd\r\n";
  //FIL outfile;
  //f_open(&outfile, "rgbs123.txt",FA_WRITE | FA_OPEN_ALWAYS);
 // for(int i = 0; i < 200; i++)
 // {
  //  uint32_t bytes_written;
  //  f_write(&outfile, buffer, 20, &bytes_written);
  //  f_sync(&outfile);
 // }
 // f_close(&outfile);
  
  vTaskSuspend(NULL);
  
}

void camera_task(void *pv) {
  Camera_Record_Snippet(20000);
  
  vTaskSuspend(NULL);
}

int main()
{
  // Start the CPU cycle counter (read with CYCCNT)
  DWT_CTRL |= DWT_CTRL_CYCEN;
  
  // Populate unique id
  populateUniqueID();
  
  // Initialize modules
  Core_Init();
  // USB_Init();
  
  
  //xTaskCreate( rs232_task, ( signed char * ) "rs232 test", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  //xTaskCreate( adc_task, ( signed char * ) "adc test", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  //xTaskCreate( camera_task, ( signed char * ) "camera test", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( image_process_task, ( signed char * ) "process", 15 * 1024, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );

  // Start the scheduler
  vTaskStartScheduler();

  // Should never get here
  while(1);
}
