/**
  ******************************************************************************
  * @file    usbd_cdc_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                CDC Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class

  *           @note
  *             For the Abstract Control Model, this core allows only transmitting the requests to
  *             lower layer dispatcher (ie. usbd_cdc_vcp.c/.h) which should manage each request and
  *             perform relative actions.
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_composite_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup usbd_cdc 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup usbd_cdc_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup usbd_cdc_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup usbd_cdc_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup usbd_cdc_Private_FunctionPrototypes
  * @{
  */

/*********************************************
   CDC Device library callbacks
 *********************************************/
static uint8_t  usbd_composite_Init        (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_composite_DeInit      (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_composite_Setup       (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_cdc_Setup       (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_hid_Setup       (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_cdc_EP0_RxReady  (void *pdev);
static uint8_t  usbd_composite_DataIn      (void *pdev, uint8_t epnum);
static uint8_t  usbd_composite_DataOut     (void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_DataIn      (void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_DataOut     (void *pdev, uint8_t epnum);
static uint8_t  usbd_hid_DataIn      (void *pdev, uint8_t epnum);
static uint8_t  usbd_hid_DataOut     (void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_SOF         (void *pdev);

/*********************************************
   CDC specific management functions
 *********************************************/
static void Handle_USBAsynchXfer  (void *pdev);
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length);
/**
  * @}
  */ 

/** @defgroup usbd_cdc_Private_Variables
  * @{
  */ 
extern CDC_IF_Prop_TypeDef  APP_FOPS;

uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ];

static uint32_t usbd_cdc_AltSet = 0;

uint8_t USB_Rx_Buffer[CDC_DATA_MAX_PACKET_SIZE];
uint8_t APP_Rx_Buffer[APP_RX_DATA_SIZE]; 
uint8_t CmdBuff[CDC_CMD_PACKET_SZE];

uint32_t APP_Rx_ptr_in  = 0;
uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;

uint8_t  USB_Tx_State = 0;

static uint32_t cdcCmd = 0xFF;
static uint32_t cdcLen = 0;


// HID variables
static uint32_t  USBD_hid_Protocol = 0;
static uint32_t  USBD_hid_IdleState = 0;
static uint32_t  USBD_hid_AltSet = 0;
static uint8_t USB_hid_Rx_Buffer[48 + 2 + 1 + 1];
// HID receive fn pointer
static usbReceiveFn USBD_Receive = NULL;

/* CDC interface class callbacks structure */
USBD_Class_cb_TypeDef  USBD_CDC_cb = 
{
  usbd_composite_Init,
  usbd_composite_DeInit,
  usbd_composite_Setup,
  NULL,                 /* EP0_TxSent, */
  usbd_cdc_EP0_RxReady,
  usbd_composite_DataIn,
  usbd_composite_DataOut,
  usbd_cdc_SOF,
  NULL,
  NULL,     
  USBD_cdc_GetCfgDesc,
#ifdef USE_USB_OTG_HS   
  USBD_cdc_GetCfgDesc, 
#endif /* USE_USB_OTG_HS  */
};

/* USB CDC device Configuration Descriptor */
uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ] =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
  LOBYTE(USB_CDC_CONFIG_DESC_SIZ),                /* wTotalLength:no of returned bytes */
  HIBYTE(USB_CDC_CONFIG_DESC_SIZ),                /* wTotalLength:no of returned bytes */
  0x03,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: bus powered */
  0x32,   /* MaxPower 100 mA */
  
  /*---------------------------------------------------------------------------*/
  
  // Interface association descriptor
  0x08, // length
  0x0B, // descriptor type
  0x00, // first interface
  0x02, // interface count
  0x02, // function class (communication)
  0x02, // function subclass (abstract control model)
  0x01, // function protocol
  0x00, // iInterface
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SZE),
#ifdef USE_USB_OTG_HS
  0x10,                           /* bInterval: */
#else
  0xFF,                           /* bInterval: */
#endif /* USE_USB_OTG_HS */
  
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
  0x00,                               /* bInterval: ignore for Bulk transfer */
  
  /************** Descriptor of Telematics HID interface ****************/
  /* 0941-9 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
  0x02,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0x00, /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Telematics HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  LOBYTE(HID_TELEMATICS_REPORT_DESC_SIZE),/*wItemLength: Total length of Report descriptor*/
  HIBYTE(HID_TELEMATICS_REPORT_DESC_SIZE),
  /******************** Descriptor of Telematics HID endpoint ************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_IN_PACKET, /*wMaxPacketSize: 4 Byte max */
  0x00,
  0x0A,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  HID_OUT_EP,     /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_OUT_PACKET, /*wMaxPacketSize: 4 Byte max */
  0x00,
  0x0A,          /*bInterval: Polling Interval (10 ms)*/
  /* 41 */
} ;

static uint8_t USBD_HID_ReportDesc[HID_TELEMATICS_REPORT_DESC_SIZE] =
{
  0x06,   0x00,   0xFF,
  0x09,   0x01,
  /* 5 */
  0xA1,   0x01,
  /* 7 */
  0x85,   0x01,           // Report ID 0x01 (input to host)
  0x75,   0x10,           // 16 bit int - ID
  0x95,   0x01,
  0x15,   0x00,
  0x27,   0xFF,   0xFF,   0x00,   0x00,
  0x09,   0x01,
  0x81,   0x02,
  0x75,   0x08,           // 8 bit int - length
  0x95,   0x01,
  0x15,   0x00,
  0x26,   0xFF,   0x00,
  0x09,   0x01,
  0x81,   0x02,
  0x75,   0x08,           // 8 bit ints - data
  0x95,   0x30,
  0x15,   0x00,
  0x26,   0xFF,   0x00,
  0x09,   0x01,
  0x81,   0x02,
  /* 50 */
  0x85,   0x81,           // Report ID 0x81 (output from host)
  0x75,   0x10,           // 16 bit int - ID
  0x95,   0x01,
  0x15,   0x00,
  0x27,   0xFF,   0xFF,   0x00,   0x00,
  0x09,   0x01,
  0x91,   0x02,
  0x75,   0x08,           // 8 bit int - length
  0x95,   0x01,
  0x15,   0x00,
  0x26,   0xFF,   0x00,
  0x09,   0x01,
  0x91,   0x02,
  0x75,   0x08,           // 8 bit ints - data
  0x95,   0x30,
  0x15,   0x00,
  0x26,   0xFF,   0x00,
  0x09,   0x01,
  0x91,   0x02,
  /* 93 */
  0xC0
  /* 94 */
}; 

/**
  * @}
  */ 

/** @defgroup usbd_cdc_Private_Functions
  * @{
  */ 

/**
  * @brief  usbd_cdc_Init
  *         Initilaize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_composite_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  // CDC
  /* Open EP IN */
  DCD_EP_Open(pdev,
              CDC_IN_EP,
              CDC_DATA_IN_PACKET_SIZE,
              USB_OTG_EP_BULK);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              CDC_OUT_EP,
              CDC_DATA_OUT_PACKET_SIZE,
              USB_OTG_EP_BULK);
  
  /* Open Command IN EP */
  DCD_EP_Open(pdev,
              CDC_CMD_EP,
              CDC_CMD_PACKET_SZE,
              USB_OTG_EP_INT);
  
  /* Initialize the Interface physical components */
  APP_FOPS.pIf_Init();

  /* Prepare Out endpoint to receive next packet */
  DCD_EP_PrepareRx(pdev,
                   CDC_OUT_EP,
                   (uint8_t*)(USB_Rx_Buffer),
                   CDC_DATA_OUT_PACKET_SIZE);
  
  
  // HID
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              HID_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              HID_OUT_PACKET,
              USB_OTG_EP_INT);

  DCD_EP_PrepareRx(pdev,
                   HID_OUT_EP,
                   (uint8_t*)(USB_hid_Rx_Buffer),
                   HID_OUT_PACKET);
  
  return USBD_OK;
}

/**
  * @brief  usbd_cdc_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_composite_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Open EP IN */
  DCD_EP_Close(pdev,
              CDC_IN_EP);
  
  /* Open EP OUT */
  DCD_EP_Close(pdev,
              CDC_OUT_EP);
  
  /* Open Command IN EP */
  DCD_EP_Close(pdev,
              CDC_CMD_EP);

  /* Restore default state of the Interface physical components */
  APP_FOPS.pIf_DeInit();

  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  return USBD_OK;
}


uint8_t USBD_SendReport     (USB_OTG_CORE_HANDLE  *pdev, 
                             uint8_t *report,
                             uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, HID_IN_EP, report, len);
  }
  
  return USBD_OK;
}

void SetUSBReceiveCallback(usbReceiveFn fn)
{
  USBD_Receive = fn;
}

/**
  * @brief  usbd_cdc_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  usbd_composite_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  if((req->wIndex & 0xFF) == 2)
    return usbd_hid_Setup(pdev, req);
  else
    return usbd_cdc_Setup(pdev, req);
}

static uint8_t  usbd_cdc_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len=USB_CDC_DESC_SIZ;
  uint8_t  *pbuf=usbd_cdc_CfgDesc + 9;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* CDC Class Requests -------------------------------*/
  case USB_REQ_TYPE_CLASS :
      /* Check if the request is a data setup packet */
      if (req->wLength)
      {
        /* Check if the request is Device-to-Host */
        if (req->bmRequest & 0x80)
        {
          /* Get the data to be sent to Host from interface layer */
          APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);
          
          /* Send the data to the host */
          USBD_CtlSendData (pdev, 
                            CmdBuff,
                            req->wLength);          
        }
        else /* Host-to-Device requeset */
        {
          /* Set the value of the current command to be processed */
          cdcCmd = req->bRequest;
          cdcLen = req->wLength;
          
          /* Prepare the reception of the buffer over EP0
          Next step: the received data will be managed in usbd_cdc_EP0_TxSent() 
          function. */
          USBD_CtlPrepareRx (pdev,
                             CmdBuff,
                             req->wLength);          
        }
      }
      else /* No Data request */
      {
        /* Transfer the command to the interface layer */
        APP_FOPS.pIf_Ctrl(req->bRequest, NULL, 0);
      }
      
      return USBD_OK;
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    
      
      
    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( (req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
      {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = usbd_cdc_Desc;   
#else
        pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);
#endif 
        len = MIN(USB_CDC_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&usbd_cdc_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
      {
        usbd_cdc_AltSet = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}

static uint8_t  usbd_hid_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case HID_REQ_SET_PROTOCOL:
      USBD_hid_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_hid_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_hid_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_hid_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_TELEMATICS_REPORT_DESC_SIZE , req->wLength);
        pbuf = USBD_HID_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        len = MIN(USB_CDC_CONFIG_DESC_SIZ , req->wLength);
        pbuf = usbd_cdc_CfgDesc + 0x09;
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_hid_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_hid_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  usbd_cdc_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_cdc_EP0_RxReady (void  *pdev)
{ 
  if (cdcCmd != NO_CMD)
  {
    /* Process the data */
    APP_FOPS.pIf_Ctrl(cdcCmd, CmdBuff, cdcLen);
    
    /* Reset the command variable to default value */
    cdcCmd = NO_CMD;
  }
  
  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_composite_DataIn (void *pdev, uint8_t epnum)
{
  if(epnum == CDC_OUT_EP)
    return usbd_cdc_DataIn(pdev, epnum);
  else
    return usbd_hid_DataIn(pdev, epnum);
}

static uint8_t  usbd_cdc_DataIn (void *pdev, uint8_t epnum)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;

  if (USB_Tx_State == 1)
  {
    if (APP_Rx_length == 0) 
    {
      USB_Tx_State = 0;
    }
    else 
    {
      if (APP_Rx_length > CDC_DATA_IN_PACKET_SIZE){
        USB_Tx_ptr = APP_Rx_ptr_out;
        USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
        
        APP_Rx_ptr_out += CDC_DATA_IN_PACKET_SIZE;
        APP_Rx_length -= CDC_DATA_IN_PACKET_SIZE;    
      }
      else 
      {
        USB_Tx_ptr = APP_Rx_ptr_out;
        USB_Tx_length = APP_Rx_length;
        
        APP_Rx_ptr_out += APP_Rx_length;
        APP_Rx_length = 0;
      }
      
      /* Prepare the available data buffer to be sent on IN endpoint */
      DCD_EP_Tx (pdev,
                 CDC_IN_EP,
                 (uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr],
                 USB_Tx_length);
    }
  }  
  
  return USBD_OK;
}

static uint8_t  usbd_hid_DataIn (void *pdev, uint8_t epnum)
{
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

/**
  * @brief  usbd_cdc_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_composite_DataOut (void *pdev, uint8_t epnum)
{
  if(epnum == CDC_OUT_EP)
    return usbd_cdc_DataOut(pdev, epnum);
  else
    return usbd_hid_DataOut(pdev, epnum);
}

static uint8_t  usbd_cdc_DataOut (void *pdev, uint8_t epnum)
{      
  uint16_t USB_Rx_Cnt;
  
  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
  
  /* USB data will be immediately processed, this allow next USB traffic being 
     NAKed till the end of the application Xfer */
  APP_FOPS.pIf_DataRx(USB_Rx_Buffer, USB_Rx_Cnt);
  
  /* Prepare Out endpoint to receive next packet */
  DCD_EP_PrepareRx(pdev,
                   CDC_OUT_EP,
                   (uint8_t*)(USB_Rx_Buffer),
                   CDC_DATA_OUT_PACKET_SIZE);

  return USBD_OK;
}

static uint8_t  usbd_hid_DataOut (void *pdev, uint8_t epnum)
{
  uint16_t len = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
  
  if(USBD_Receive && (len > 0))
  {
    USBD_Receive(USB_hid_Rx_Buffer, len);
  }
  
  DCD_EP_PrepareRx(pdev,
                   HID_OUT_EP,
                   (uint8_t*)(USB_hid_Rx_Buffer),
                   HID_OUT_PACKET);
  
  return USBD_OK;
}

/**
  * @brief  usbd_audio_SOF
  *         Start Of Frame event management
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_SOF (void *pdev)
{      
  static uint32_t FrameCount = 0;
  
  if (FrameCount++ == CDC_IN_FRAME_INTERVAL)
  {
    /* Reset the frame counter */
    FrameCount = 0;
    
    /* Check the data to be sent through IN pipe */
    Handle_USBAsynchXfer(pdev);
  }
  
  return USBD_OK;
}

/**
  * @brief  Handle_USBAsynchXfer
  *         Send data to USB
  * @param  pdev: instance
  * @retval None
  */
static void Handle_USBAsynchXfer (void *pdev)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;
  
  if(USB_Tx_State != 1)
  {
    if (APP_Rx_ptr_out == APP_RX_DATA_SIZE)
    {
      APP_Rx_ptr_out = 0;
    }
    
    if(APP_Rx_ptr_out == APP_Rx_ptr_in) 
    {
      USB_Tx_State = 0; 
      return;
    }
    
    if(APP_Rx_ptr_out > APP_Rx_ptr_in) /* rollback */
    { 
      APP_Rx_length = APP_RX_DATA_SIZE - APP_Rx_ptr_out;
    
    }
    else 
    {
      APP_Rx_length = APP_Rx_ptr_in - APP_Rx_ptr_out;
     
    }
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
     APP_Rx_length &= ~0x03;
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
    
    if (APP_Rx_length > CDC_DATA_IN_PACKET_SIZE)
    {
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
      
      APP_Rx_ptr_out += CDC_DATA_IN_PACKET_SIZE;	
      APP_Rx_length -= CDC_DATA_IN_PACKET_SIZE;
    }
    else
    {
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = APP_Rx_length;
      
      APP_Rx_ptr_out += APP_Rx_length;
      APP_Rx_length = 0;
    }
    USB_Tx_State = 1; 

    DCD_EP_Tx (pdev,
               CDC_IN_EP,
               (uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr],
               USB_Tx_length);
  }  
  
}

/**
  * @brief  USBD_cdc_GetCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_cdc_CfgDesc);
  return usbd_cdc_CfgDesc;
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
