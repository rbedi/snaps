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

// ############################
// ##### GLOBAL VARIABLES #####
// ############################

extern volatile uint64_t time;