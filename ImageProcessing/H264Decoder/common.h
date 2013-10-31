/***************************************************************************
 *                                                                         *
 *     Copyright (C) 2008  ludrao.net                                      *
 *     ludh264@ludrao.net                                                  *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define   TRACE_LEVEL TRACE_ALL



// Common enum and types
typedef enum
{
  RET_SUCCESS,
  ERR_NO_START_CODE,
  ERR_NO_ENDING_START_CODE,
  ERR_WORKING_BUFFER_TOO_SMALL,
  ERR_MEMORY_ALLOC_FAILED,
  ERR_NALU_EMPTY,
  ERR_PARSED_VALUE_OUT_OF_RANGE,
  ERR_REFERING_NON_EXISTING_SPS,
  ERR_REFERING_NON_EXISTING_PPS,
} RetCode;

typedef enum
{
  TRACE_NOTHING,
  TRACE_ERROR,
  TRACE_INFO,
  TRACE_DETAIL,
  TRACE_ALL,
} TraceLevel;


// type of a pixel. Needed because it could be eather 8 bits or 16bits if bit_depth_luma_minus8/chroma is not null...
#if 1
typedef uint8_t pixel_t;
#define PIXEL_T_IS_8_BITS
#else
typedef uint16_t pixel_t;
#define PIXEL_T_IS_16_BITS
#endif

// #define aligned_pixels(nb) __attribute__ ((aligned(sizeof(pixel_t)*(nb))))


// Object header: add necessary fields in the object that need to be used/released
#define OBJECT_HEADER_DECL                \
  uint32_t ref_count

#define OBJECT_HEADER_INST(o)             \
  (o)->ref_count = 0;


// For picture, use specific functions instead
#define use_object(o)                     \
  do {                                    \
    (o)->ref_count++;                     \
  } while(0)


#define LUD_TRACE(lvl, f, args...)   \
do                                   \
{                                    \
  if (lvl < TRACE_LEVEL)             \
    printf(f, ## args);              \
} while(0)


// Define a compilation conditional block. The cond must be determined at compilation time !
#define CONDITIONAL_BLOCK(cond, block)   \
do                                \
{                                 \
  if (cond)                       \
  {                               \
    block;                        \
  }                               \
 } while(0)


#define unused_param __attribute__((unused))
#define declare_aligned(n)  __attribute__ ((aligned (n)))
#define force_inline  __attribute__ ((always_inline)) inline
#define should_inline inline
//#define force_inline
//#define should_inline

#define LUD_ASSERT(a) assert(a)

#ifndef NDEBUG

#define LUD_DEBUG_ASSERT(a) assert(a)
#define DEBUG_PRINT_VAR_DEC(a) printf(#a" %d\n", (a))
#define DEBUG_PRINT_VAR_HEX(a) printf(#a" %x\n", (a))

#else //NDEBUG

#define LUD_DEBUG_ASSERT(a)
#define DEBUG_PRINT_VAR_DEC(a)
#define DEBUG_PRINT_VAR_HEX(a)

#endif


#endif //__COMMON_H__

