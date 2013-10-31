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


#ifndef __DECODE_SLICE_H__
#define __DECODE_SLICE_H__

#include "decode.h"
#include "bitstream_types.h"
#include <inttypes.h>

RetCode decode_slice_data(PictureDecoderData* pdd, nal_unit_t* nalu, slice_header_t* sh,
    seq_parameter_set_rbsp_t* sps, pic_parameter_set_rbsp_t* pps, unsigned int entropy_coding_mode_flag);



static should_inline unsigned int NextMbAddress(uint8_t* MbToSliceGroupMap, unsigned int n, unsigned int PicSizeInMbs, unsigned int num_slice_groups_minus1)
{
  unsigned int i = n+1;

  if (!num_slice_groups_minus1) // easy optimisation...
    return i;

  while( i < PicSizeInMbs && MbToSliceGroupMap[ i ] != MbToSliceGroupMap[ n ] )
    i++;

  return i;
}


// can_be_I_mb must be set to a constant value when calling this function => enable simplification when we know we are dealing with P/B mb only
static should_inline PREDICTION_MODE MbPartPredMode( MB_TYPE mb_type, unsigned int mbPartIdx, unsigned int transform_size_8x8_flag, unsigned int can_be_I_mb)
{
  LUD_DEBUG_ASSERT(mb_type < NB_OF_MB_TYPE && mbPartIdx < 2);
  if (!can_be_I_mb || mb_type > 0)
  {
    LUD_DEBUG_ASSERT(mbPartIdx == 0 || mb_type >= P_L0_16x16 );
    return MbPartPredMode_array[mbPartIdx][mb_type];
  }
  else
  {
    LUD_DEBUG_ASSERT(mbPartIdx == 0);
    if (transform_size_8x8_flag)
      return Intra_8x8;
    else
      return Intra_4x4;
  }
}

static should_inline unsigned int NumMbPart(MB_TYPE mb_type)
{
  LUD_DEBUG_ASSERT(mb_type >= P_L0_16x16 && mb_type < NB_OF_MB_TYPE);
  return Intra16x16PredMode_NumMbPart_array[mb_type];
}

static should_inline PREDICTION_MODE SubMbPredMode(SUB_MB_TYPE sub_mb_type )
{
  LUD_DEBUG_ASSERT(sub_mb_type < NB_OF_SUB_MB_TYPE);
  return SubMbPredMode_array[sub_mb_type];
}

static should_inline unsigned int NumSubMbPart(SUB_MB_TYPE sub_mb_type)
{
  LUD_DEBUG_ASSERT(sub_mb_type < NB_OF_SUB_MB_TYPE);
  return NumSubMbPart_array[sub_mb_type];
}

static should_inline unsigned int MbPartWidth(MB_TYPE mb_type)
{
  LUD_DEBUG_ASSERT(mb_type >= P_L0_16x16 && mb_type < NB_OF_MB_TYPE);
  return CodedBlockPatternChroma_MbPartWidth_array[mb_type];
}

static should_inline unsigned int MbPartHeight(MB_TYPE mb_type)
{
  LUD_DEBUG_ASSERT(mb_type >= P_L0_16x16 && mb_type < NB_OF_MB_TYPE);
  return CodedBlockPatternLuma_MbPartHeight_array[mb_type];
}

static should_inline unsigned int SubMbPartWidth(SUB_MB_TYPE sub_mb_type)
{
  LUD_DEBUG_ASSERT(sub_mb_type < NB_OF_SUB_MB_TYPE);
  return SubMbPartWidth_array[sub_mb_type];
}

static should_inline unsigned int SubMbPartHeight(SUB_MB_TYPE sub_mb_type)
{
  LUD_DEBUG_ASSERT(sub_mb_type < NB_OF_SUB_MB_TYPE);
  return SubMbPartHeight_array[sub_mb_type];
}

static should_inline unsigned int is_IntraMb(MB_TYPE mb_type)
{
  return mb_type<=SI;
}
#define IS_INTRA(mb) (is_IntraMb((mb)->mb_type))

static should_inline unsigned int is_InterMb(MB_TYPE mb_type)
{
  return mb_type>=P_L0_16x16;
}
#define IS_INTER(mb) (is_InterMb((mb)->mb_type))

// 4x4 or 8x8 pred type. SI macroblocks are 4x4 pred type
static should_inline unsigned int is_NxNMb(MB_TYPE mb_type)
{
  return (mb_type == I_NxN || mb_type == SI);
}
#define IS_NxN(mb) (is_NxNMb((mb)->mb_type))

static should_inline unsigned int is_data_partitioning_slice(nal_unit_type_t nal_unit_type)
{
  return nal_unit_type>=NALU_TYPE_DPA && nal_unit_type<=NALU_TYPE_DPC;
}

static should_inline MbAttrib* get_left_mbaff_mb(MbAttrib* curr_mb, unsigned int PicWidthInMbs,
    unsigned int curr_is_field, unsigned int left_is_field, unsigned int curr_is_bot, unsigned int block_row)
{
  unsigned int left_mb_topbot = left_mb_pos[curr_is_field][left_is_field][curr_is_bot][block_row];
  return curr_mb -1 -(PicWidthInMbs << curr_is_bot) + (PicWidthInMbs << left_mb_topbot);
}
static should_inline unsigned int get_left_mbaff_4x4block(unsigned int curr_is_field, unsigned int left_is_field,
    unsigned int curr_is_bot, unsigned int block_row)
{
  return left_4x4block_pos[curr_is_field][left_is_field][curr_is_bot][block_row];
}

static should_inline MbAttrib* get_up_mbaff_mb(MbAttrib* curr_mb, unsigned int PicWidthInMbs,
    unsigned int curr_is_field, unsigned int up_is_field, unsigned int curr_is_bot)
{
  return curr_mb - (PicWidthInMbs << up_4x4block_pos[curr_is_field][up_is_field][curr_is_bot]);
}

static should_inline MbAttrib* get_upleft_mbaff_mb(MbAttrib* curr_mb, unsigned int PicWidthInMbs,
    unsigned int curr_is_field, unsigned int upleft_is_field, unsigned int curr_is_bot)
{
  unsigned int bit47 = upleft_4x4block_pos[curr_is_field][upleft_is_field][curr_is_bot] >> 4;
  return curr_mb - 1 - PicWidthInMbs * bit47;
}
static should_inline unsigned int get_upleft_mbaff_4x4block(unsigned int curr_is_field, unsigned int upleft_is_field,
    unsigned int curr_is_bot)
{
  unsigned int bit03 = upleft_4x4block_pos[curr_is_field][upleft_is_field][curr_is_bot] & 0xF;
  return bit03;
}

static should_inline MbAttrib* get_upright_mbaff_mb(MbAttrib* curr_mb, unsigned int PicWidthInMbs,
    unsigned int curr_is_field, unsigned int upright_is_field, unsigned int curr_is_bot)
{
  return curr_mb + 1 - (PicWidthInMbs << upright_4x4block_pos[curr_is_field][upright_is_field][curr_is_bot]);
}


#endif //__DECODE_SLICE_H__
