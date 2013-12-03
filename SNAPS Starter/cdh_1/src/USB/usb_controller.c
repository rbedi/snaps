#include "global.h"
#include "usb_controller.h"

#include  "usbd_composite_core.h"
#include  "usbd_usr.h"
#include  "usbd_desc.h"
#include  "usb_dcd_int.h"

static USB_OTG_CORE_HANDLE  USB_OTG_dev;
static void usbThread(void* pv);

void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

static void usbThread(void* pv)
{
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc,  &USBD_CDC_cb, &USR_cb);
  
  vTaskSuspend(NULL);
}

void USB_Init(void)
{
  xTaskCreate( usbThread, ( signed char * ) "USB thread", configMINIMAL_STACK_SIZE + 200, ( void * ) NULL, tskIDLE_PRIORITY + 1, NULL );
}
