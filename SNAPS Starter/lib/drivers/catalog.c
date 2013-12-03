#include "global.h"
#include "catalog.h"

// Identifier masks for easy decoding
#define CAT_IDMASK_ISCAT    0x10000000
#define CAT_IDMASK_DEVID    0x0FF00000
#define CAT_IDMASK_VARID    0x000FF000
#define CAT_IDMASK_INDEX    0x00000FF0
#define CAT_IDMASK_OPID 	0x0000000F
// Identifier shifts for easy reading
#define CAT_IDSHIFT_ISCAT   28
#define CAT_IDSHIFT_DEVID   20
#define CAT_IDSHIFT_VARID   12
#define CAT_IDSHIFT_INDEX   4
#define CAT_IDSHIFT_OPID 0

#define CAN_MAXPAYLOAD	8
#define CAT_OPERATION_ERROR 255

/* Utilities */
static uint32_t assembleID(uint32_t devID,
    uint32_t varID, uint32_t ind, uint32_t opID){
    return (CAT_IDMASK_ISCAT | (devID << CAT_IDSHIFT_DEVID)
        | (varID << CAT_IDSHIFT_VARID) | (ind << CAT_IDSHIFT_INDEX)
        | (opID << CAT_IDSHIFT_OPID));
}

/* Functions for pulling apart a message ID */
static inline bool isWritable(CatalogEntry *entry){
    return ((entry->flags & FLAG_WRITABLE) == FLAG_WRITABLE);
}

static inline uint8_t getOpID(uint32_t extID){
    return extID & CAT_IDMASK_OPID;
}

static inline uint8_t getIndex(uint32_t extID){
    return ((extID & CAT_IDMASK_INDEX) >> CAT_IDSHIFT_INDEX);
}

static inline uint8_t getVarID(uint32_t extID){
    return ((extID & CAT_IDMASK_VARID) >> CAT_IDSHIFT_VARID);
}

// These are not static because they are used in interrupts
uint8_t getDevID(uint32_t extID){
    return ((extID & CAT_IDMASK_DEVID) >> CAT_IDSHIFT_DEVID);
}

bool isCat(uint32_t extID){
    return ((extID & CAT_IDMASK_ISCAT) >> CAT_IDSHIFT_ISCAT);
}

uint8_t catEntryArrayLength(CatalogEntry *entry)
{
	if (entry->typeID == CAT_TID_CHAR){ 
		return entry->length / 8;
    }
	return entry->length;
}

// Compute a CRC32 for a catalog entry
/* CRC32 engine computes a running CRC32 (Ethernet standard polynomial).
   You use it by appending chunks of words. The CRC32 result register is always
   complete and correct and may be used at any time. To start a new CRC32, 
   you must reset the CRC32 data register. */
static uint32_t catGetEntryHash(CatalogEntry* entry, bool reset){
    if (reset) 
        CRC_ResetDR();    
    /* The relevant parts of the entry are the first few ints.
     * Five uint32_ts.  Then the name.*/
    uint32_t calc = CRC_CalcBlockCRC((uint32_t *)entry, 5);

    int name_space = strnlen(entry->name, CAT_MAX_STRLEN) + 1;
    int len = ((name_space + (((name_space % 4) != 0) ? 4 : 0)) / 4);
    calc = CRC_CalcBlockCRC((uint32_t *)entry->name, len);
    return calc;
}

// Compute a CRC32 for an entire catalog
static uint32_t catGetCatHash(Catalog* catalog){
    CRC_ResetDR();
    uint32_t calc = 0;
    CatalogEntry *cur = catalog->catalogEntriesHead;
    while (cur != NULL)
    {
        calc = catGetEntryHash(cur, false);
        cur = cur->next;
    }
    return calc;
}

/*// @TODO(Rachel, w/Sasha): Actually do this
static uint32_t catGetSWHash(Catalog *catalog){
    return 0xdecebeef;
}*/

// Save permanent entries to flash
// @TODO(Rachel, w/Sasha): Actually do this
void catSaveToFlash(Catalog* catalog, void* addr);

// Restore permanent entries from flash
// @TODO(Rachel, w/Sasha): Actually do this
void catGetFromFlash(Catalog* catalog, void* addr);

/* Check the status of CAN message mailboxes. If none are available, enqueue
   the message and turn on an interrupt so that we empty the queue as messages
   go out. If mailboxes are available, send immediately.*/
void CanEnqueue(CAN_TypeDef* CANx, CanTxMsg* TxMessage, xQueueHandle queue)
{
    //uint8_t usedMb = CAN_Transmit(CANx, TxMessage);
    //while(CAN_TransmitStatus(CAN1, usedMb) != CAN_TxStatus_Ok);
    if(CAN_Transmit(CANx, TxMessage) == CAN_NO_MB)
    {
		// Append to the queue, blocking if the queue is full.
		// Time out after 5 SysTicks (usually 1ms/SysTick).
        xQueueSendToBack(CanTxQueue, TxMessage, 5);
        CANx->IER |= CAN_IT_TME;
    }
	//printf("Msg: %i\n", *((int *)TxMessage->Data));
}

/* Traverse the list, searching for the catalog entry with the 
   specified variable ID. */
CatalogEntry *catGetCatalogEntry(Catalog *catalog, uint8_t varIDtoFind)
{
    CatalogEntry *cur = catalog->catalogEntriesHead;
    while (cur != NULL)
    {
        if (cur->varID == varIDtoFind)
            return cur;
        cur = cur->next;
    }
    // Failed to find the entry
    return NULL;
}

AnnounceEntry *catGetAnnounceEntry(Catalog *catalog, uint8_t varIDtoFind)
{
    CatalogEntry *catEntry = catGetCatalogEntry(catalog, varIDtoFind);
    if (catEntry == NULL){
        return NULL;
    }
    return catEntry->announceEntry;
}

/* Copies a specific CAN payload-sized chunk of a string in to a txData buffer.
   Will copy up to CAN_MAXPAYLOAD bytes if they exist in the src string. Returns
   number of bytes copied.*/
uint8_t catFillStringChunk(uint8_t *data, uint8_t chunkNum, char *src)
{
    char *begin = src + (CAN_MAXPAYLOAD * chunkNum);
    uint8_t len = strnlen(begin, CAT_MAX_STRLEN);
    len = (len > CAN_MAXPAYLOAD) ? CAN_MAXPAYLOAD : len;
    strncpy((char *)data, begin, len);
    return len;
}


/* Fills the data field of a CAN message with the value of the
 * variable given by the entry based on the entry's typeID.
 * Returns the size of the data.
 * @TODO(Rachel): Add uint64_t and int64_t handlers
 */
static uint8_t catFillValue(CatalogEntry *entry, uint8_t *data, uint32_t recID)
{
    uint8_t ind = getIndex(recID);
	if (ind > catEntryArrayLength(entry)) return CAT_OPERATION_ERROR;
        
    switch (entry->typeID){
        case CAT_TID_CHAR:
			return catFillStringChunk(data, ind, (char *)entry->varptr);
        case CAT_TID_UINT8:
            *data = ((uint8_t *)entry->varptr)[ind];
            return 1;
        case CAT_TID_INT8:
            *((int8_t *)data) = ((int8_t *)entry->varptr)[ind];
            return 1;
        case CAT_TID_UINT16:
            *((uint16_t *)data) = ((uint16_t *)entry->varptr)[ind];
            return 2;
        case CAT_TID_INT16:
            *((int16_t *)data) = ((int16_t *)entry->varptr)[ind];
            return 2;
		case CAT_TID_ENUM:		// Fall through
        case CAT_TID_UINT32:
            *((uint32_t *)data) = ((uint32_t *)entry->varptr)[ind];
            return 4;
        case CAT_TID_INT32:
            *((int32_t *)data) = ((int32_t *)entry->varptr)[ind];
            return 4;
		case CAT_TID_BITFIELD:
		case CAT_TID_UINT64:
            *((uint64_t *)data) = ((uint64_t *)entry->varptr)[ind];
            return 8;
		case CAT_TID_DOUBLE:
            *((double *)data) = ((double *)entry->varptr)[ind];
            return 8;
		case CAT_TID_INT64:
			*((int64_t *)data) = ((int64_t *)entry->varptr)[ind];
			return 8;
        case CAT_TID_FLOAT:
            *((float *)data) = ((float *)entry->varptr)[ind];
            return 4;
        case CAT_TID_BOOL:
            *((bool *)data) = ((bool *)entry->varptr)[ind];
            return 1;

		// Error case
        default:
            return CAT_OPERATION_ERROR;
    }
}

/* Fills the data field of a message according to the op code.
 * Either fills it directly or calls the appropriate function.
 * Returns the length of the data field.
 * TODO: Change the types of the casts when you change the types of the data
 * (for instance, typeID).
 */
static uint8_t catFillData(CatalogEntry *entry, uint8_t *data, uint32_t recID)
{
    uint8_t opID = getOpID(recID);
    switch (opID){
        case CAT_OP_TYPEID: 
            *((uint32_t *)data) = entry->typeID;
            return 4;
        case CAT_OP_LEN:
            *((uint32_t *)data) = entry->length;
            return 4;          
        case CAT_OP_FLAGS: 
            *((uint32_t *)data) = entry->flags;
            return 4;
        case CAT_OP_NAMELEN:
            *((uint32_t *)data) = strnlen(entry->name, CAT_MAX_STRLEN);
            return 4;
        case CAT_OP_NAME:
            return catFillStringChunk(data, getIndex(recID), entry->name);
        case CAT_OP_VALUE:
            return catFillValue(entry, data, recID);
        case CAT_OP_GET_ANNOUNCE:
            *((uint32_t *)data) = entry->announceEntry->interval;
            return 4;
        case CAT_OP_HASH:
            *((uint32_t *)data) = catGetEntryHash(entry, true);
            return 4;
        default:
            return CAT_OPERATION_ERROR; 
    }
}

/* Set up an empty CAN message and hand off data-filling to catFillData()
 * Used only in response to a "get" request.
 */
static void catRespond(Catalog *catalog, uint32_t recID)
{    
    uint8_t varID = getVarID(recID);
    CatalogEntry *entry = catGetCatalogEntry(catalog, varID);

    static CanTxMsg TxMessage;
    
    TxMessage.StdId = 0;
    TxMessage.ExtId = recID;
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;

	// Protect against uninitialized entries and failed data manipulation
    if (entry != NULL){
        TxMessage.DLC = catFillData(entry, TxMessage.Data, recID);
		if(TxMessage.DLC <= 8)
			CanEnqueue(CAN1, &TxMessage, CanTxQueue);
    }
}

/* Receives chunked string data from outside and rehydrates it.
 * @TODO(Rachel): Don't allow resizing of strings; truncate data past length - nullterminator
 */
void write_string(CatalogEntry *entry, uint8_t ind, uint8_t dlc, char *data)
{
    if (strnlen((char *)(entry->varptr), CAT_MAX_STRLEN) < 8 * ind + dlc) return;  //Error: tried to write past the string
    for (int i = 0; i < dlc; i++)
        ((char *)entry->varptr)[8 * ind + i] = data[i];
}

/* Assign a value from the network to a local variable
 * @TODO(Rachel): Stop passing in both CanRxMsg and DlcData - one is inside the other
 */
void setValue(Catalog *catalog, CanRxMsg *RxMessage)
{
    CatalogEntry *target = catGetCatalogEntry(catalog, getVarID(RxMessage->ExtId));
	// Warning: "index" is a reserved keyword due to -std=gnu99
    uint8_t ind = getIndex(RxMessage->ExtId);
	
	// Scrub out-of-bounds requests
	if (!isWritable(target) || (ind > catEntryArrayLength(target))) return;
    
	// Handle normal cases by typeID
    switch (target->typeID){
        case CAT_TID_CHAR:
            write_string(target, ind, RxMessage->DLC, (char *)RxMessage->Data);
            return;
        case CAT_TID_UINT8:
            ((uint8_t *)target->varptr)[ind] = *((uint8_t *)RxMessage->Data);
            return;
        case CAT_TID_INT8:
            ((int8_t *)target->varptr)[ind] = *((int8_t *)RxMessage->Data);
            return;
        case CAT_TID_UINT16:
            ((uint16_t *)target->varptr)[ind] = *((uint16_t *)RxMessage->Data);
            return;
        case CAT_TID_INT16:
            ((int16_t *)target->varptr)[ind] = *((int16_t *)RxMessage->Data);
            return;
		case CAT_TID_ENUM:	//Fallthrough
        case CAT_TID_UINT32:
            ((uint32_t *)target->varptr)[ind] = *((uint32_t *)RxMessage->Data);
            return;
        case CAT_TID_INT32:
            ((int32_t *)target->varptr)[ind] = *((int32_t *)RxMessage->Data);
            return;
		case CAT_TID_BITFIELD:  //Fallthrough
		case CAT_TID_UINT64:
            ((uint64_t *)target->varptr)[ind] = *((uint64_t *)RxMessage->Data);
            return;
		case CAT_TID_INT64:
            ((int64_t *)target->varptr)[ind] = *((int64_t *)RxMessage->Data);
            return;
        case CAT_TID_DOUBLE:
            ((double *)target->varptr)[ind] = *((double *)RxMessage->Data);
            return;
        case CAT_TID_FLOAT:
            ((float *)target->varptr)[ind] = *((float *)RxMessage->Data);
            return;
        case CAT_TID_BOOL:
            ((bool *)target->varptr)[ind] = *((bool *)RxMessage->Data);
            return;
        default:
            return;
    }
}

void setAnnounce(Catalog *catalog, CanRxMsg *RxMessage)
{
    AnnounceEntry *entry = catGetAnnounceEntry(catalog, getVarID(RxMessage->ExtId));
    if (RxMessage->DLC == 4)
        entry->interval = *(uint32_t *)(RxMessage->Data);
}

/* Assembles a packet with the value specified by the entry and the index,
 * and puts it into the outgoing message buffer.
 * Visible externally so that clients can explicitly broadcast after important
 * updates.  Requires the client to have a pointer to the relevant entry. */
void catAnnounceValue(Catalog *catalog, CatalogEntry *curEntry, uint8_t ind){
    CanTxMsg TxMessage;

    // We're always sending data, not requests
    TxMessage.RTR = CAN_RTR_DATA;
    // Use extended (29-bit) identifiers
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.StdId = 0;

    TxMessage.ExtId = assembleID(catalog->devID, curEntry->varID, ind, CAT_OP_VALUE);
    TxMessage.DLC = catFillValue(curEntry, TxMessage.Data, TxMessage.ExtId);
	// @TODO(Rachel): Errp. Use external enqueue funcptr from CAN driver (when one exists).
    CanEnqueue(CAN1, &TxMessage, CanTxQueue);
}

void catAnnounceEntry(Catalog *catalog, CatalogEntry *entry)
{
    uint8_t len = catEntryArrayLength(entry);
    for (int i = 0; i < len; i++){
        catAnnounceValue(catalog, entry, i);
    }
}

/***************Broadcast and receive tasks***************/
void CatalogReceiveTask(void* pvParameters)
{
    static CanRxMsg RxMessage;
    Catalog *catalog = (Catalog *)pvParameters;

    while(1)
    {
		/* This thread can wait in the BLOCKED state for up to 1000 SysTicks
		 * before automatically coming back in to the READY state. Incoming
		 * packets automatically unblock the thread.
		 */
        if(xQueueReceive(CatalogRxQueue, &RxMessage, 1000) == pdPASS)
        {
            uint8_t opID = getOpID(RxMessage.ExtId);
            if (opID <= GETTER_MAX)
                catRespond(catalog, RxMessage.ExtId);
            else if (opID == CAT_OP_SETANNOUNCE)
                setAnnounce(catalog, &RxMessage);
            else if (opID == CAT_OP_SETVALUE)
                setValue(catalog, &RxMessage);
        }
    }
}

// Broadcast CAN messages on a regular interval
// @TODO(Rachel): Move to announcer
void CatalogAnnounceTask(void* pvParameters)
{
    Catalog *catalog = (Catalog *)pvParameters;
	
    // Send packets forever
	while(1)
	{
        /* Set the next time to the max, then bring it down to the minimum
         * next time specified by the entries. */
        uint64_t minNextTime = ULONG_MAX;
        AnnounceEntry *cur = catalog->announceEntriesHead;
		/* Iterate over the whole announce list, sending out entries whose announce
		 * time has come. Keep tabs on the next deadline so that we may send
		 * at an appropriate time while avoiding unnecessary rechecks.
		 */
        while(cur != NULL)
        {
            if (time >= cur->nextTime)
            {
                cur->nextTime = cur->nextTime + cur->interval;
                catAnnounceEntry(catalog, cur->catalogEntry);
            }
            //We want this to be the minimum next send time
            if (cur->nextTime < minNextTime)
                minNextTime = cur->nextTime;
            cur = cur->next;
        }   
        vTaskDelay(minNextTime - time);
	}
}
/***************Updates***************/

/* Updates a value that is stored as a uint32_t and announces its
 * new value on the bus.
 */
void catUpdateU32(Catalog *catalog, CatalogEntry *curEntry, uint32_t newVal)
{
    *((uint32_t *)curEntry->varptr) = newVal;

    CanTxMsg TxMessage;

    // We're always sending data, not requests
    TxMessage.RTR = CAN_RTR_DATA;
    // Use extended (29-bit) identifiers
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.StdId = 0;

    TxMessage.ExtId = assembleID(catalog->devID, curEntry->varID, 0, CAT_OP_VALUE);
    TxMessage.DLC = catFillValue(curEntry, (TxMessage.Data), TxMessage.ExtId);
    CanEnqueue(CAN1, &TxMessage, CanTxQueue);
}

/* Wrapper functions so the user has different names--call these when updating
 * enums and bit fields to guarantee that their new values are broadcast 
 * immediately.  
 * Enums and bitfields are otherwise not broadcast--we only want to send out 
 * their values when they change.
 */
void catUpdateEnum(Catalog *catalog, CatalogEntry *entry, uint32_t newVal)
{
    catUpdateU32(catalog, entry, newVal);
}

void catUpdateBitField(Catalog *catalog, CatalogEntry *entry, uint32_t newVal)
{
    catUpdateU32(catalog, entry, newVal);
}

/***************Initialization code***************/
/* Enable CRC clock */
void CRC_Setup()
{ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
}

// Set up the CAN peripheral.
// PB8 (CANRX) as digital input with integrated pull-up
// PB9 (CANTX) as low speed digital push-pull output
/* This was taken from the MPPT initialization code on an F2. 
 * TODO: make sure this will work on an F2, F4 or F1. 
 * TODO: ifdefs have been added.  Test. 
void CAN_Setup()
{
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    // Data structure to represent GPIO configuration information
    GPIO_InitTypeDef  GPIO_InitStructure;

	// Initialize the correct pins for the F2 vs the F4
	#ifdef STM32F4XX
		// Enable GPIO bank A and AFIO clocks
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		// Enable the remap of CAN1 to PA11 and PA12
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1); 

		// Indicate that we want digital input with pull-up on PA11 (CANRX)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// Indicate that we want low speed digital push-pull output on PA12 (CANTX)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	#elif STM32F2XX
		// Enable GPIO bank B and AFIO clocks
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

		// Enable the remap of CAN1 to PB8 and PB9
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1); 

		// Indicate that we want digital input with pull-up on PB8 (CANRX)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// Indicate that we want low speed digital push-pull output on PB9 (CANTX)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);		
	#endif

    // Enable the CAN peripheral clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    // Put the CAN peripheral in configuration mode
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    // CAN peripheral initialization (125kbps) (1000kbps for Aurora)
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = ENABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStructure.CAN_SJW = CAN_SJW_4tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    
    CAN_InitStructure.CAN_Prescaler = 16;
    
    // Enable the peripheral
    CAN_Init(CAN1, &CAN_InitStructure);

    // CAN filter initialization
    CAN_FilterInitStructure.CAN_FilterNumber=0;
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow=0x00;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x00;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    // Interrupt on CAN receive and CAN transmit
    CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_TME, ENABLE);
	
	// Configure CAN interrupt priority
    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
*/

/* Add an entry to the end of the catalog's list of entries. */
void appendToCatalogList(Catalog *catalog, CatalogEntry *entry)
{
    catalog->elemCount++;
    if (catalog->catalogEntriesHead == NULL)
        catalog->catalogEntriesHead = catalog->catalogEntriesTail = entry;
    else
    {
        catalog->catalogEntriesTail->next = entry;
        catalog->catalogEntriesTail = entry;
    }
}

void appendToAnnounceList(Catalog *catalog, AnnounceEntry *entry)
{
    if (catalog->announceEntriesHead == NULL)
        catalog->announceEntriesHead = catalog->announceEntriesTail = entry;
    else
    {
        catalog->announceEntriesTail->next = entry;
        catalog->announceEntriesTail = entry;
    }
}

// Initialize the catalog data structure
void catInitializeCatalog(Catalog* catalog, uint8_t devID)
{
    // Initialize the CRC for calculating hashes
    CRC_Setup();
    //CAN_Setup();

	CatalogRxQueue = xQueueCreate(CAN_RXLEN, sizeof(CanTxMsg));
	
	catalog->elemCount = 0;
    catalog->devID = devID;

    catalog->catalogEntriesHead = NULL;
    catalog->catalogEntriesTail = NULL;
}

/* Mallocs space for a new CatalogEntry, fills in the data fields, and 
 * adds it to the catalog. */
 // @TODO(Rachel): Make sure to harmonize these types with what's actually stored (red flag: length)
CatalogEntry *catInitEntry(Catalog *catalog, uint8_t typeID, uint8_t varID, \
    uint8_t flags, uint32_t announce, char *name, uint8_t length, void *varptr)
{
    CatalogEntry *entry = malloc(sizeof(CatalogEntry));
    if (entry == NULL)
      return NULL;
	entry->typeID = typeID;
    entry->varID = varID;
    entry->flags = flags;
	entry->varptr = varptr;
    entry->length = length;
    entry->next = NULL;
    appendToCatalogList(catalog, entry);

    // Copy over in case the name string was declared locally in a function
	// @TODO(Rachel): strnlen (TRUST NO ONE)
    int name_space = strnlen(name, CAT_MAX_STRLEN - 1) + 1;
    /* The CRC works on 32-bit chunks, so we round up the amount of space 
	 * needed for the string and zero-fill the rest.  It's null-terminated
	 * so this isn't visible from anywhere else. @TODO(Rachel): this becomes
	 * deprecated with new CRC32. (hack concentration camps++) */
    int len = name_space + ((((name_space) % 4) != 0) ? 4 : 0);
	entry->name = malloc(sizeof(uint8_t) * len);
	if (entry->name == NULL) 
		return NULL;
	memset(entry->name, 0,len);
	strcpy(entry->name, name);
	
	if (announce != NO_ANNOUNCE)
	{
        AnnounceEntry *announceEntry = malloc(sizeof(AnnounceEntry));
        if (announceEntry == NULL)
            return NULL;
        announceEntry->next = NULL;
        announceEntry->interval = announce;
        announceEntry->nextTime = time + announce;
        announceEntry->catalogEntry = entry;
        appendToAnnounceList(catalog, announceEntry);
        entry->flags |= FLAG_ANNOUNCED;
	}
    else{
        entry->flags &= ~FLAG_ANNOUNCED;
    }
    return entry;
}

/* Adds required entries that do not have to be initialized at the end of
 * catalog intitialization.  This can be extended indefinitely without 
 * worrying about the order of events. */
void catAddRequiredEntries(Catalog *catalog, char *board_name)
{
    static uint32_t static_cat_version = 1;
    catInitEntry(catalog, CAT_TID_UINT32, VID_CAT_VERSION, 0, NO_ANNOUNCE,\
        "Catalog protocol version", 1, &static_cat_version);

	uint32_t len = strnlen(board_name, CAT_MAX_STRLEN - 1);
    char *name = malloc(len + 1);
	if (name == NULL)
		return;
    strncpy(name, board_name, len + 1);
	name[len] = '\0';
    catInitEntry(catalog, CAT_TID_CHAR, VID_BOARD_NAME, 0, NO_ANNOUNCE,\
        "Board name", len, name);
}

/* Create the array of used variable IDs.  This is the last step in creating
 * the catalog, because this array needs to include every entry in the whole
 * catalog. Before creating the array it also adds all of the other required
 * entries to the catalog. */
void catFinishInit(Catalog *catalog, char *board_name)
{
    catAddRequiredEntries(catalog, board_name);

    // Now that all entries are in the catalog, calculate the hash
    static uint32_t static_hash;
    static_hash = catGetCatHash(catalog);
    catInitEntry(catalog, CAT_TID_UINT32, VID_CAT_HASH, 0, NO_ANNOUNCE,\
        "Catalog hash", 1, &static_hash);

    // Make an array with space for all of the existing entries + this one.
    uint32_t *used_IDs = malloc(sizeof(uint32_t) * (catalog->elemCount + 1));
    char *name = "In-use varID array";
    catInitEntry(catalog, CAT_TID_UINT32, VID_VID_ARRAY, 0,
        NO_ANNOUNCE, name, catalog->elemCount + 1, used_IDs);
    
    // This entry is now in the list. Walk the list adding IDs to the array
    CatalogEntry *cur = catalog->catalogEntriesHead;
    int count = 0;
    while (cur != NULL)
    {
        used_IDs[count] = cur->varID;
        count++;
        cur = cur->next;
    }
}