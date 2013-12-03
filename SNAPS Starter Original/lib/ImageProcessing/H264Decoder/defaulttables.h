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



#ifndef __DEFAULTTABLES_H__
#define __DEFAULTTABLES_H__

#include <inttypes.h>
#include "syntax.h"

#define N_A   -1


extern const uint8_t ScalingList_Default_4x4[2][16];
extern const uint8_t ScalingList_Default_8x8[2][64];

// depends on chroma_format_idc
static const uint8_t SubWidthC[4] =
{
  0, 2, 2, 1
};

// depends on chroma_format_idc
static const uint8_t SubHeightC[4] =
{
  0, 2, 1, 1
};

// depends on chroma_format_idc
static const uint8_t MbWidthC[4] =
{
  0, 8, 8, 16
};
static const uint8_t MbWidthCdiv4[4] =
{
  0, 2, 2, 4
};

// depends on chroma_format_idc
static const uint8_t MbHeightC[4] =
{
  0, 8, 16, 16
};
static const uint8_t MbHeightCdiv4[4] =
{
  0, 2, 4, 4
};

// depends on chroma_format_idc
static const uint16_t MbSizeC[4] =
{
  0, 64, 128, 256
};
static const uint8_t MbSizeCdiv16[4] =
{
  0, 4, 8, 16
};
static const uint8_t MbNumC8x8[4] =
{
  0, 1, 2, 4
};


static const uint8_t pred_mode_conv[10] = {2, 0, 1, 2, 3, 4, 5, 6, 7, 8};

// MbPartPredMode( mb_type, mbPartIdx ) array (I, SI, P, SP, B Slices)
// Must not be used for mb_type = 0
static const uint8_t MbPartPredMode_array[2][NB_OF_MB_TYPE] =
{
  {
    // From I Slices
    N_A, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16,
    Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16,
    Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, Intra_16x16, N_A,
    // From SI Slices
    Intra_4x4,
    // From P, SP Slices
    Pred_L0, Pred_L0, Pred_L0, N_A, N_A, Pred_L0,
    // From B Slices
    Direct, Pred_L0, Pred_L1, BiPred, Pred_L0, Pred_L0, Pred_L1, Pred_L1, Pred_L0, Pred_L0, Pred_L1, Pred_L1,
    Pred_L0, Pred_L0, Pred_L1, Pred_L1, BiPred, BiPred, BiPred, BiPred, BiPred, BiPred, N_A, Direct
  },
  {
    // I and SI slice: NA
    N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A, N_A,
    N_A, N_A, N_A, N_A, N_A,
    // From P SP Slices
    N_A, Pred_L0, Pred_L0, N_A, N_A, N_A,
    // From B Slices
    N_A, N_A, N_A, N_A, Pred_L0, Pred_L0, Pred_L1, Pred_L1, Pred_L1, Pred_L1, Pred_L0, Pred_L0, BiPred, BiPred,
    BiPred, BiPred, Pred_L0, Pred_L0, Pred_L1, Pred_L1, BiPred, BiPred, N_A, N_A
  }
};



// CodedBlockPatternChroma (I and SI) MbPartWidth( mb_type ) (P SP B)
// Remove 27=P_L0_16x16_MB from mb_type in order to get the correct value
static const uint8_t CodedBlockPatternChroma_MbPartWidth_array[NB_OF_MB_TYPE] =
{
  // CodedBlockPatternChroma From I Slices
  N_A, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, N_A,
  // CodedBlockPatternChroma From SI Slice
  N_A,
  // MbPartWidth From P SP Slices
  4, 4, 2, 2, 2, 4,
  // MbPartWidth From B Slices
  2, 4, 4, 4, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 2, 2
};
// CodedBlockPatternLuma (I and SI) MbPartHeight( mb_type ) (P, SP and B)
// Remove 27=P_L0_16x16_MB from mb_type in order to get the correct value
static const uint8_t CodedBlockPatternLuma_MbPartHeight_array[NB_OF_MB_TYPE] =
{
  // CodedBlockPatternLuma From I Slices
  N_A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, N_A,
  // CodedBlockPatternLuma From SI Slice
  N_A,
  // MbPartHeight From P SP Slices
  4, 2, 4, 2, 2, 4,
  // MbPartHeight From B Slices
  2, 4, 4, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 2
};
// Intra16x16PredMode (for I and SI SLices) and NumMbPart( mb_type ) (for P SP B SLices)
// the first 27 values are Intra16x16PredMode, the following values are NumMbPart
static const uint8_t Intra16x16PredMode_NumMbPart_array[NB_OF_MB_TYPE] =
{
  // Intra16x16PredMode From I Slices
  N_A, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, N_A,
  // Intra16x16PredMode From SI SLice
  N_A,
  // NumMbPart From P SP Slices
  1, 2, 2, 4, 4, 1,
  // NumMbPart From B Slices
  N_A, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, N_A
};


// NumSubMbPart
static const uint8_t NumSubMbPart_array[NB_OF_SUB_MB_TYPE] =
{
  // From P macroblocks
  1, 2, 2, 4,
  // From B macroblocks
  4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 4, 4
};

// NumSubMbPart
static const uint8_t SubMbPredMode_array[NB_OF_SUB_MB_TYPE] =
{
  // From P macroblocks
  Pred_L0, Pred_L0, Pred_L0, Pred_L0,
  // From B macroblocks
  Direct, Pred_L0, Pred_L1, BiPred, Pred_L0, Pred_L0, Pred_L1, Pred_L1, BiPred, BiPred, Pred_L0, Pred_L1, BiPred
};

// SubMbPartWidth
static const uint8_t SubMbPartWidth_array[NB_OF_SUB_MB_TYPE] =
{
  // From P macroblocks
  2, 2, 1, 1,
  // From B macroblocks
  1, 2, 2, 2, 2, 1, 2, 1, 2, 1, 1, 1, 1
};

// SubMbPartHeight
static const uint8_t SubMbPartHeight_array[NB_OF_SUB_MB_TYPE] =
{
  // From P macroblocks
  2, 1, 2, 1,
  // From B macroblocks
  1, 2, 2, 2, 1, 2, 1, 2, 1, 2, 1, 1, 1
};

// convert from raster to scan or scan to raster (same table!)
static const uint8_t scan4x4[16] =
{
  0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};
static const uint8_t scan4x4_col[16] =
{
  0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3
};
static const uint8_t scan4x4_row[16] =
{
  0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};
static const uint8_t scan2x4[8] =
{
  0, 1, 2, 3, 4, 5, 6, 7
};
static const uint8_t scan2x4_col[8] =
{
  0, 1, 0, 1, 0, 1, 0, 1
};
static const uint8_t scan2x4_row[8] =
{
  0, 0, 1, 1, 2, 2, 3, 3
};
static const uint8_t col_scan4x4[4] =
{
  0, 2, 8, 10
};

static const uint8_t r4x4tor2x2[16] =
{
  0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};

// Conversion table from 4x4scan to 6x5raster. This table is used in mv pred
static const uint8_t scan4x4toraster6x5[16] =
{
  7, 8, 13, 14, 9, 10, 15, 16, 19, 20, 25, 26, 21, 22, 27, 28
};

// Conversion table from 4x4raster to 6x5raster. This table is used in mv pred
static const uint8_t raster4x4toraster6x5[16] =
{
  7, 8, 9, 10, 13, 14, 15, 16, 19, 20, 21, 22, 25, 26, 27, 28
};

static const uint8_t raster6x5toscan4x4[30] =
{
    N_A, N_A, N_A, N_A, N_A, N_A,
    N_A,   0,   1,   4,   5, N_A,
    N_A,   2,   3,   6,   7, N_A,
    N_A,   8,   9,  12,  13, N_A,
    N_A,  10,  11,  14,  15, N_A,
};

static const uint8_t raster6x5toraster4x4[30] =
{
    N_A, N_A, N_A, N_A, N_A, N_A,
    N_A,   0,   1,   2,   3, N_A,
    N_A,   4,   5,   6,   7, N_A,
    N_A,   8,   9,  10,  11, N_A,
    N_A,  12,  13,  14,  15, N_A,
};

// Conversion table from 4x4scan to 5x5raster. This table is used in order to calculate nC
static const uint8_t scan4x4toraster5x5[16] =
{
  6, 7, 11, 12, 8, 9, 13, 14, 16, 17, 21, 22, 18, 19, 23, 24
};

static const uint8_t raster4x4toraster5x5[16] =
{
  6, 7, 8, 9, 11, 12, 13, 14, 16, 17, 18, 19, 21, 22, 23, 24
};

static const uint8_t raster8x8toraster5x5[4] =
{
  6, 8, 16, 18
};


// Conversion table from 2x4scan to 3x5raster. This table is used in order to calculate nC
static const uint8_t raster2x4toraster3x5[8] =
{
  4, 5, 7, 8, 10, 11, 13, 14
};
static const uint8_t raster2x4toraster5x5[8] =
{
  6, 7, 11, 12, 16, 17, 21, 22
};


static const uint8_t inverse_4x4zigzag_scan[16] =
{
  0+0*4, 1+0*4, 0+1*4, 0+2*4,
  1+1*4, 2+0*4, 3+0*4, 2+1*4,
  1+2*4, 0+3*4, 1+3*4, 2+2*4,
  3+1*4, 3+2*4, 2+3*4, 3+3*4,
};

static const uint8_t inverse_4x4field_scan[16] =
{
  0+0*4, 0+1*4, 1+0*4, 0+2*4,
  0+3*4, 1+1*4, 1+2*4, 1+3*4,
  2+0*4, 2+1*4, 2+2*4, 2+3*4,
  3+0*4, 3+1*4, 3+2*4, 3+3*4,
};

static const uint8_t inverse_8x8zigzag_scan[64] =
{
  0+0*8, 1+0*8, 0+1*8, 0+2*8,
  1+1*8, 2+0*8, 3+0*8, 2+1*8,
  1+2*8, 0+3*8, 0+4*8, 1+3*8,
  2+2*8, 3+1*8, 4+0*8, 5+0*8,
  4+1*8, 3+2*8, 2+3*8, 1+4*8,
  0+5*8, 0+6*8, 1+5*8, 2+4*8,
  3+3*8, 4+2*8, 5+1*8, 6+0*8,
  7+0*8, 6+1*8, 5+2*8, 4+3*8,
  3+4*8, 2+5*8, 1+6*8, 0+7*8,
  1+7*8, 2+6*8, 3+5*8, 4+4*8,
  5+3*8, 6+2*8, 7+1*8, 7+2*8,
  6+3*8, 5+4*8, 4+5*8, 3+6*8,
  2+7*8, 3+7*8, 4+6*8, 5+5*8,
  6+4*8, 7+3*8, 7+4*8, 6+5*8,
  5+6*8, 4+7*8, 5+7*8, 6+6*8,
  7+5*8, 7+6*8, 6+7*8, 7+7*8,
};
static const uint8_t inverse_8x8field_scan[64] =
{
  0+0*8, 0+1*8, 0+2*8, 1+0*8,
  1+1*8, 0+3*8, 0+4*8, 1+2*8,
  2+0*8, 1+3*8, 0+5*8, 0+6*8,
  0+7*8, 1+4*8, 2+1*8, 3+0*8,
  2+2*8, 1+5*8, 1+6*8, 1+7*8,
  2+3*8, 3+1*8, 4+0*8, 3+2*8,
  2+4*8, 2+5*8, 2+6*8, 2+7*8,
  3+3*8, 4+1*8, 5+0*8, 4+2*8,
  3+4*8, 3+5*8, 3+6*8, 3+7*8,
  4+3*8, 5+1*8, 6+0*8, 5+2*8,
  4+4*8, 4+5*8, 4+6*8, 4+7*8,
  5+3*8, 6+1*8, 6+2*8, 5+4*8,
  5+5*8, 5+6*8, 5+7*8, 6+3*8,
  7+0*8, 7+1*8, 6+4*8, 6+5*8,
  6+6*8, 6+7*8, 7+2*8, 7+3*8,
  7+4*8, 7+5*8, 7+6*8, 7+7*8,
};


// the 2 xxx_cavlc tables perform inverse 8x8 scan on LumaLevel8x8[ i8x8 ][ 4 * i + i4x4 ] = LumaLevel[ i8x8 * 4 + i4x4 ][ i ]
// only used when decoding cavlc slices...
static const uint8_t inverse_8x8zigzag_scan_cavlc[64] =
{
  0+0*8, 1+1*8, 1+2*8, 2+2*8,
  4+1*8, 0+5*8, 3+3*8, 7+0*8,
  3+4*8, 1+7*8, 5+3*8, 6+3*8,
  2+7*8, 6+4*8, 5+6*8, 7+5*8,
  1+0*8, 2+0*8, 0+3*8, 3+1*8,
  3+2*8, 0+6*8, 4+2*8, 6+1*8,
  2+5*8, 2+6*8, 6+2*8, 5+4*8,
  3+7*8, 7+3*8, 4+7*8, 7+6*8,
  0+1*8, 3+0*8, 0+4*8, 4+0*8,
  2+3*8, 1+5*8, 5+1*8, 5+2*8,
  1+6*8, 3+5*8, 7+1*8, 4+5*8,
  4+6*8, 7+4*8, 5+7*8, 6+7*8,
  0+2*8, 2+1*8, 1+3*8, 5+0*8,
  1+4*8, 2+4*8, 6+0*8, 4+3*8,
  0+7*8, 4+4*8, 7+2*8, 3+6*8,
  5+5*8, 6+5*8, 6+6*8, 7+7*8,
};
static const uint8_t inverse_8x8field_scan_cavlc[64] =
{
  0+0*8, 1+1*8, 2+0*8, 0+7*8,
  2+2*8, 2+3*8, 2+4*8, 3+3*8,
  3+4*8, 4+3*8, 4+4*8, 5+3*8,
  5+5*8, 7+0*8, 6+6*8, 7+4*8,
  0+1*8, 0+3*8, 1+3*8, 1+4*8,
  1+5*8, 3+1*8, 2+5*8, 4+1*8,
  3+5*8, 5+1*8, 4+5*8, 6+1*8,
  5+6*8, 7+1*8, 6+7*8, 7+5*8,
  0+2*8, 0+4*8, 0+5*8, 2+1*8,
  1+6*8, 4+0*8, 2+6*8, 5+0*8,
  3+6*8, 6+0*8, 4+6*8, 6+2*8,
  5+7*8, 6+4*8, 7+2*8, 7+6*8,
  1+0*8, 1+2*8, 0+6*8, 3+0*8,
  1+7*8, 3+2*8, 2+7*8, 4+2*8,
  3+7*8, 5+2*8, 4+7*8, 5+4*8,
  6+3*8, 6+5*8, 7+3*8, 7+7*8,
};
static const uint8_t normAdjust4x4[6][3] =
{
  { 10, 13, 16 },
  { 11, 14, 18 },
  { 13, 16, 20 },
  { 14, 18, 23 },
  { 16, 20, 25 },
  { 18, 23, 29 }
};

static const uint8_t normAdjust8x8[6][16] =
{
//vm 0   3   4   3   3   1   5   1   4   5   2   5   3   1   5   1
  { 20, 19, 25, 19, 19, 18, 24, 18, 25, 24, 32, 24, 19, 18, 24, 18 },
  { 22, 21, 28, 21, 21, 19, 26, 19, 28, 26, 35, 26, 21, 19, 26, 19 },
  { 26, 24, 33, 24, 24, 23, 31, 23, 33, 31, 42, 31, 24, 23, 31, 23 },
  { 28, 26, 35, 26, 26, 25, 33, 25, 35, 33, 45, 33, 26, 25, 33, 25 },
  { 32, 30, 40, 30, 30, 28, 38, 28, 40, 38, 51, 38, 30, 28, 38, 28 },
  { 36, 34, 46, 34, 34, 32, 43, 32, 46, 43, 58, 43, 34, 32, 43, 32 }
};

static const uint8_t qPi_to_QPC[22] =
{
  29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39
};

static const uint8_t chroma_inverse_dc_scan1[4] =
{
  0, 1, 2, 3
};
static const uint8_t chroma_inverse_dc_scan2[8] =
{
  0, 2, 1, 4, 6, 3, 5, 7
};

// Relevant when MbAffFrameFlag == 1
// Give the left mb position according to [curr_mb_is_field][left_mb_is_field][curr_is_bot_mb][curr_mb_block_row]
// each entry is a mb top/bot (=0 => top mb)
// left_mb = curr_mb - 1 - (PicWidthInMbs * curr_is_bot) + PicWithInMbs*left_mb_pos[][][][]
//         = curr_mb - 1 + PicWidthInMbs - (PicWidthInMbs<<curr_is_bot) - PicWithInMbs + (PicWithInMbs<<left_mb_pos[][][][])
//         = curr_mb - 1 - (PicWidthInMbs<<curr_is_bot) + (PicWithInMbs<<left_mb_pos[][][][])
static const uint8_t left_mb_pos[2][2][2][4] =
{
    { {{0, 0, 0, 0}, {1, 1, 1, 1}}, {{0, 0, 0, 0}, {0, 0, 0, 0}} },
    { {{0, 0, 1, 1}, {0, 0, 1, 1}}, {{0, 0, 0, 0}, {1, 1, 1, 1}} }
};

// Give the 4x4 left block position according to [curr_mb_is_field][left_mb_is_field][curr_is_bot_mb][curr_mb_block_row]
// each entry is =pos (in raster4x4 order) of the 4x4 block in the left mb
static const uint8_t left_4x4block_pos[2][2][2][4] =
{
    { {{3, 7, 11, 15}, {3, 7, 11, 15}}, {{3, 3, 7, 7}, {11, 11, 15, 15}} },
    { {{3, 11, 3, 11}, {3, 11, 3, 11}}, {{3, 7, 11, 15}, {3, 7, 11, 15}} }
};



// Give the 4x4 upper block position according to [curr_mb_is_field][left_mb_is_field][curr_is_bot_mb]
// each entry is a bit field of 1 bit: bit0= row offset minus 1 to subtract from curr mb in order to get the top mb
// up_mb = curr_mb - (PicWidthInMbs << up_4x4block_pos[][][]);
// the 4x4 block is: block_pos = 12, 13, 14, 15 according to the required column (0, 1, 2 or 3) (in 4x4raster mode)
static const uint8_t up_4x4block_pos[2][2][2] =
{
    { {0, 0}, {0, 0} },
    { {0, 1}, {1, 1} }
};

// Give the 4x4 upper left block according to [curr_mb_is_field][left_mb_is_field][curr_is_bot_mb]
// each entry is a bit field of 4 bits: bit03=pos (4x4raster order) of the 4x4 block in the mb, bit47= row offset to subtract from the current mb in
//  order to get the upper left mb
// up_left_mb = curr_mb - 1 - PicWidthInMbs * (bit47);
// the 4x4 block is: block_pos = bit03; (4x4raster mode)
static const uint8_t upleft_4x4block_pos[2][2][2] =
{
    { {0x1F, 0x1F}, {0x1F, 0x07} },
    { {0x1F, 0x2F}, {0x2F, 0x2F} }
};

// Give the 4x4 upper right block according to [curr_mb_is_field][left_mb_is_field][curr_is_bot_mb]
// each entry is a bit field of 1 bit: bit0= row offset minus 1 to subtract from curr mb in order to get the up right mb
// up_right_mb = curr_mb + 1 - (PicWidthInMbs << bit0);
// the 4x4 block is always the block at pos 12 (4x4raster mode)
static const uint8_t upright_4x4block_pos[2][2][2] =
{
    { {0, 0/*NA*/}, {0, 0/*NA*/} },
    { {0, 1}, {1, 1} }
};

// This is an undefined mb with a slice_num that is different from a real slice_num
extern MbAttrib undef_mb;

// Defines the partIdx of the left/up mb given its struct type and the row / col that is needed
static const uint8_t partstruct_def[4][4] = //[mb_partstruct_type or mb_subpartstruct_type][row(0..1) and col(2..3)]
{
    {0, 0,   0, 0},
    {0, 1,   1, 1},
    {1, 1,   0, 1},
    {1, 3,   2, 3}
};

// Offset for the left or up block. This is for r5x5 table
static const uint8_t leftpartoffset[4][4] = //[mb_partstruct_type or mb_subpartstruct_type][partIdx]
{
    {1, N_A, N_A, N_A},
    {1,   1, N_A, N_A},
    {1, 2*1, N_A, N_A},
    {1, 2*1,   1, 2*1},
};
static const uint8_t uppartoffset[4][4] = //[mb_partstruct_type or mb_subpartstruct_type][partIdx]
{
    {5, N_A, N_A, N_A},
    {5, 2*5, N_A, N_A},
    {5,   5, N_A, N_A},
    {5,   5, 2*5, 2*5},
};
// gives the r5x5idx given a struct type and the mbPartIdx
static const uint8_t partr5x5idx[4][4] = //[mb_partstruct_type][partIdx]
{
    { 6, N_A, N_A, N_A},
    { 6,  16, N_A, N_A},
    { 6,   8, N_A, N_A},
    { 6,   8,  16,  18},
};
// gives the r5x5idx given a struct type and the subMbPartIdx
static const uint8_t partr5x5idxoffset[4][4] = //[submb_partstruct_type][partIdx]
{
    { 0, N_A, N_A, N_A},
    { 0,   5, N_A, N_A},
    { 0,   1, N_A, N_A},
    { 0,   1,   5,   6},
};

// Indicate the type of mb part struct (only for B/P mb !)
//  0 => 16x16
//  1 => 16x8
//  2 => 8x16
//  3 => 8x8 => the mb has sub partitions !
//  4 => not available
static const uint8_t mb_partstruct_type[57] =
{
  // From I Slices
  4,//  I_NxN          = 0,
  4,//  I_16x16_0_0_0,
  4,//  I_16x16_1_0_0,
  4,//  I_16x16_2_0_0,
  4,//  I_16x16_3_0_0,
  4,//  I_16x16_0_1_0,
  4,//  I_16x16_1_1_0,
  4,//  I_16x16_2_1_0,
  4,//  I_16x16_3_1_0,
  4,//  I_16x16_0_2_0,
  4,//  I_16x16_1_2_0,
  4,//  I_16x16_2_2_0,
  4,//  I_16x16_3_2_0,
  4,//  I_16x16_0_0_1,
  4,//  I_16x16_1_0_1,
  4,//  I_16x16_2_0_1,
  4,//  I_16x16_3_0_1,
  4,//  I_16x16_0_1_1,
  4,//  I_16x16_1_1_1,
  4,//  I_16x16_2_1_1,
  4,//  I_16x16_3_1_1,
  4,//  I_16x16_0_2_1,
  4,//  I_16x16_1_2_1,
  4,//  I_16x16_2_2_1,
  4,//  I_16x16_3_2_1,
  4,//  I_PCM,
  //
  //  // From SI Slices
  4,//  SI  = 26,
  //
  //  // From P and SP Slices
  0,//  P_L0_16x16  = 27,
  1,//  P_L0_L0_16x8,
  2,//  P_L0_L0_8x16,
  3,//  P_8x8,
  3,//  P_8x8ref0,
  4,//  P_Skip,
  //
  //  // From B Slices
  4,//  B_Direct_16x16= 33,
  0,//  B_L0_16x16,
  0,//  B_L1_16x16,
  0,//  B_Bi_16x16,
  1,//  B_L0_L0_16x8,
  2,//  B_L0_L0_8x16,
  1,//  B_L1_L1_16x8,
  2,//  B_L1_L1_8x16,
  1,//  B_L0_L1_16x8,
  2,//  B_L0_L1_8x16,
  1,//  B_L1_L0_16x8,
  2,//  B_L1_L0_8x16,
  1,//  B_L0_Bi_16x8,
  2,//  B_L0_Bi_8x16,
  1,//  B_L1_Bi_16x8,
  2,//  B_L1_Bi_8x16,
  1,//  B_Bi_L0_16x8,
  2,//  B_Bi_L0_8x16,
  1,//  B_Bi_L1_16x8,
  2,//  B_Bi_L1_8x16,
  1,//  B_Bi_Bi_16x8,
  2,//  B_Bi_Bi_8x16,
  3,//  B_8x8,
  4,//  B_Skip,
} ;

// Indicate the type of sub mb part struct (only for B/P mb !)
//  0 => 8x8
//  1 => 8x4
//  2 => 4x8
//  3 => 4x4
static const uint8_t submb_partstruct_type[17] =
{
  //  // From P Slices
  0,//  P_L0_8x8 = 0,
  1,//  P_L0_8x4,
  2,//  P_L0_4x8,
  3,//  P_L0_4x4,
  //
  //  // From B Slices
  3,//  B_Direct_8x8 = 4,
  0,//  B_L0_8x8,
  0,//  B_L1_8x8,
  0,//  B_Bi_8x8,
  1,//  B_L0_8x4,
  2,//  B_L0_4x8,
  1,//  B_L1_8x4,
  2,//  B_L1_4x8,
  1,//  B_Bi_8x4,
  2,//  B_Bi_4x8,
  3,//  B_L0_4x4,
  3,//  B_L1_4x4,
  3,//  B_Bi_4x4,
  //
  //  NB_OF_SUB_MB_TYPE // = 17
};

static const uint8_t coded_block_flag_ctxIdxBlockCatOffset[6] = {0, 4, 8, 12, 16, N_A};
static const uint8_t significant_coeff_flag_ctxIdxBlockCatOffset[6] = {0, 15, 29, 44, 47, 0};
//static const uint8_t last_significant_coeff_flag_ctxIdxBlockCatOffset[6] = {0, 15, 29, 44, 47, 0};
#define last_significant_coeff_flag_ctxIdxBlockCatOffset significant_coeff_flag_ctxIdxBlockCatOffset
static const uint8_t coeff_abs_level_minus1_ctxIdxBlockCatOffset[6] = {0, 10, 20, 30, 39, 0};


static const uint8_t significant_coeff_flag_ctxIdxInc[2][63] =
{
  {
    0, 1, 2, 3, 4, 5, 5, 4, 4, 3, 3, 4, 4, 4, 5, 5,
    4, 4, 4, 4, 3, 3, 6, 7, 7, 7, 8, 9,10, 9, 8, 7,
    7, 6,11,12,13,11, 6, 7, 8, 9,14,10, 9, 8, 6,11,
    12,13,11, 6, 9,14,10, 9,11,12,13,11,14,10,12
  },
  {
    0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 7, 7, 8, 4, 5,
    6, 9,10,10, 8,11,12,11, 9, 9,10,10, 8,11,12,11,
    9, 9,10,10, 8,11,12,11, 9, 9,10,10, 8,13,13, 9,
    9,10,10, 8,13,13, 9, 9,10,10,14,14,14,14,14
  }
};

static const uint8_t last_significant_coeff_flag_ctxIdxInc[63] =
{
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8
};

extern const uint8_t coeff_token_values[62];
extern const uint8_t coeff_token[3][62];
extern const uint8_t coeff_token_len[3][62];
extern const uint8_t coeff_token_fixed[64];
extern const uint8_t coeff_token_cdc0[14];
extern const uint8_t coeff_token_cdc0_len[14];
extern const uint8_t coeff_token_cdc1[30];
extern const uint8_t coeff_token_cdc1_len[30];
extern const uint8_t total_zeros_values[16];
extern const uint8_t total_zeros_for4x4blocks[15][16];
extern const uint8_t total_zeros_for4x4blocks_len[15][16];
extern const uint8_t total_zeros_for2x2blocks[3][4];
extern const uint8_t total_zeros_for2x2blocks_len[3][4];
extern const uint8_t total_zeros_for2x4blocks[7][8];
extern const uint8_t total_zeros_for2x4blocks_len[7][8];
extern const uint8_t run_before_vlc[7][15];
extern const uint8_t run_before_vlc_len[7][15];

#endif //__DEFAULTTABLES_H__
