#pragma once

#ifdef STM32F4XX
	#include <stm32f4xx.h>
	#include <stm32f4xx_can.h>
    #include <stm32f4xx_crc.h>
#elif STM32F2XX
    #include <stm32f2xx_crc.h>
	#include <stm32f2xx_can.h>
	#include <stm32f2xx.h>
#endif

#include "FreeRTOS.h"
#include <queue.h>
#include <list.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Catalog version, mostly for FLASH save/rehydrate
#define CAT_VERSION         0x0

/* For purposes of avoiding several switch/case statements, we divide into
   sets of getter and setter opIDs. */
// Max operation ID for a get request--they start at 0x0 and increase
#define GETTER_MAX          0x7
// Min operation ID for a set request--they start at 0xF and decrease
#define SETTER_MIN          0xE

// Field or operation values
#define CAT_OP_TYPEID       0x0
#define CAT_OP_LEN          0x1
#define CAT_OP_FLAGS        0x2
#define CAT_OP_NAMELEN      0x3
#define CAT_OP_NAME         0x4
#define CAT_OP_VALUE        0x5
#define CAT_OP_HASH         0x6
#define CAT_OP_GET_ANNOUNCE     0x7

#define CAT_OP_SETANNOUNCE  0xE
#define CAT_OP_SETVALUE     0xF

// Type ID values

#define CAT_TID_UINT8           0
#define CAT_TID_INT8            1
#define CAT_TID_UINT16          2
#define CAT_TID_INT16           3
#define CAT_TID_UINT32          4
#define CAT_TID_INT32           5
#define CAT_TID_CHAR          	6
#define CAT_TID_FLOAT         	7
#define CAT_TID_DOUBLE        	8
#define CAT_TID_BOOL          	9
#define CAT_TID_ENUM            0xA
#define CAT_TID_BITFIELD       	0xB
#define CAT_TID_INT64			0xC
#define CAT_TID_UINT64			0xD

// Required entries
#define VID_CAT_VERSION     0x0
#define VID_CAT_HASH        0x1
#define VID_VID_ARRAY       0x2
#define VID_BOARD_NAME      0x3
#define VID_SW_HASH         0x4

#define VID_BOARD_HW        0xe
#define VID_BOARD_SN        0xf
#define VID_SYS_TIME        0x10
#define VID_REBOOT_FLAG     0x11
#define VID_REBOOT_STR      0x12
#define VID_SAVE_PARAMS     0x13

// The announce value that signals that an entry will only be sent if queried.
#define NO_ANNOUNCE   		0x0

#define NO_FLAGS			0x0
#define FLAG_WRITABLE       0x1
#define FLAG_NONVOLATILE    0x2
#define FLAG_TRANSIENT      0x4
#define FLAG_ANNOUNCED		0x8

// Error on a get: Index out of range
#define CAT_MAX_STRLEN			2048


typedef union
{
    char* asChar;
    uint8_t* asUByte;
    int8_t* asByte;
    uint32_t* asUInt;
    int32_t* asInt;
    uint16_t* asUShort;
    int16_t* asShort;
    float* asFloat;
    double* asDouble;
    uint64_t* asULong;
    int64_t* asLong;
} DlcData;

// Forward declarations
typedef struct CatalogEntryStruct CatalogEntry;
typedef struct AnnounceEntryStruct AnnounceEntry;

// CatalogEntry
struct CatalogEntryStruct
{
    uint32_t typeID;
    uint32_t length;
    uint32_t varID;
	uint32_t flags;
    char*   name;
    void*   varptr;
    AnnounceEntry *announceEntry;
    CatalogEntry *next;
};

// AnnounceEntry
struct AnnounceEntryStruct
{
	uint32_t interval;
	uint64_t nextTime;
	CatalogEntry *catalogEntry;
    AnnounceEntry *next;
};

// Catalog, with expectation to use malloc/realloc
typedef struct
{
	uint8_t			elemCount;
    uint8_t         devID;
    
    CatalogEntry* 	catalogEntriesHead;
    CatalogEntry*   catalogEntriesTail;
	
	AnnounceEntry*	announceEntriesHead;
	AnnounceEntry*	announceEntriesTail;
} Catalog;

CatalogEntry *catInitEntry(Catalog *catalog, uint8_t typeID, uint8_t varID, \
    uint8_t flags, uint32_t announce, char *name, uint8_t length,        \
    void *varptr);


// Initialize the catalog data structure
void catInitializeCatalog(Catalog* catalog, uint8_t devID);

void catFinishInit(Catalog *catalog, char *board_name);

void catAddRequiredEntries(Catalog *catalog, char *board_name);

// Thread that processes catalog requests, pvParameters = Catalog*
void catRequestProcessor(void* pvParameters);

// Enqueue a catalog request
uint32_t catEnqueueRequest(Catalog* catalog, CanRxMsg* msg);

void CatalogReceiveTask(void* pvParameters);
void CatalogAnnounceTask(void* pvParameters);

/* Immediately push the value of the specified variable to the outgoing
 * message queue.  Does not modify normal announcements in any way. */
void catAnnounceValue(Catalog *catalog, CatalogEntry *curEntry, uint8_t index);

// Do these matter?
void catUpdateEnum(Catalog *cat, CatalogEntry *entry, uint32_t newVal);
void catUpdateBitField(Catalog *cat, CatalogEntry *entry, uint32_t newVal);

// Used in interrupts
bool isCat(uint32_t extID);
uint8_t getDevID(uint32_t extID);

/* RR TODOs
 * Initialization should configure the can peripheral, 
 * allowing the user to choose which CAN peripheral they
 * want.  Also either hardcode cleanly or remove the ability
 * to set the enqueue function and the receive queue.
 */

