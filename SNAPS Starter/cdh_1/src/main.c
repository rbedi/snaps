#include "global.h"
#include "util.h"
#include "stm32f4/core.h"
#include "usb_controller.h"
#include "diskio.h"
#include "ff.h"
#include "stm32f4/radio.h"
#include "stm32f4/adc.h"
#include "stm32f4/camera.h"

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
void vApplicationIdleHook(void) { } // __WFI(); }

// Hook to handle stack overflow
void vApplicationStackOverflowHook(xTaskHandle xTask,
                                   signed portCHAR *pcTaskName){}

// ################
// ##### MAIN #####
// ################

#pragma location = "SRAMSection" // Place the next variable in SRAM
uint8_t mem[128]; // This is placed in SRAM
uint8_t k; // This is placed in normal memory because it isn't preceded with the #pragma location directive

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

void camera_task(void *pv) {
  
  vTaskDelay(100);
  /*
  Camera_Cmd(ENABLE);
  Camera_Record_Snippet(4000);
  vTaskDelay(1000);
  Camera_Cmd(DISABLE);
  */
  vTaskDelay(100);
  
  vTaskDelay(100);
  
  {
    FATFS fs;
    FIL file;
    
    if(f_mount(0, &fs) != FR_OK)
      while(1) ;
    
    if(f_open(&file, "temp1.txt", FA_WRITE | FA_OPEN_ALWAYS) != FR_OK)
      while(1) ;
    
    uint32_t bytes_written = 0;
    f_write(&file, "test\r\n", 6, &bytes_written);
    
    f_sync(&file);
    f_close(&file);
    
    f_mount(0, NULL);
    
    uint8_t cmd = 0;
    disk_ioctl(0, CTRL_POWER, &cmd);
    
    if(f_mount(0, &fs) != FR_OK)
      while(1) ;
    
    if(f_open(&file, "temp2.txt", FA_WRITE | FA_OPEN_ALWAYS) != FR_OK)
      while(1) ;
    
    bytes_written = 0;
    f_write(&file, "test\r\n", 6, &bytes_written);
    
    f_sync(&file);
    f_close(&file);
    
    f_mount(0, NULL);
        
    
  }
  
  
  vTaskDelay(100);
  
  
  vTaskDelay(100);
  /*
  Camera_Cmd(ENABLE);
  Camera_Record_Snippet(4000);
  vTaskDelay(1000);
  Camera_Cmd(DISABLE);
  
  vTaskDelay(100);
  */
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
  
  //xTaskCreate( task, ( signed char * ) "task", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  //xTaskCreate( rs232_task, ( signed char * ) "rs232 test", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  //xTaskCreate( adc_task, ( signed char * ) "adc test", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
  xTaskCreate( camera_task, ( signed char * ) "camera test", 4000, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );

  // Start the scheduler
  vTaskStartScheduler();

  // Should never get here
  while(1);
}
