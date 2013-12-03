#pragma once

// ###################
// ##### M4 CORE #####
// ###################

#define CYCCNT          (*(volatile unsigned long*)0xE0001004)
#define DWT_CTRL        (*(volatile unsigned long*)0xE0001000)
#define DWT_CTRL_CYCEN  0x00000001

// ##########################
// ##### GLOBAL DEFINES #####
// ##########################

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define BOUND(a, b, c)	(MIN(b, MAX(a, c)))

// ####################
// ##### INCLUDES #####
// ####################

// STM32 peripheral library files
#include <stm32f4xx.h>

// ANSI C library files
#include <stdio.h>
#include <string.h>

// Scheduler header files
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Local definitions
#include "lis331.h"
#include "l3g4200.h"
#include "ms5803.h"
#include "config.h"
#include "catalog.h"

// ############################
// ##### GLOBAL VARIABLES #####
// ############################

extern volatile uint64_t time;
// Outgoing can messages
extern xQueueHandle CanTxQueue;
// All incoming messages
extern xQueueHandle CanRxQueue;

// #############################
// ##### CATALOG VARIABLES #####
// #############################

// Incoming catalog messages should go here.
extern xQueueHandle CatalogRxQueue;

extern Catalog cat;

#define CAN_TXLEN		128
#define CAN_RXLEN		128

#define DEVICE_ID 0x14