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

#ifndef CABAC_H_
#define CABAC_H_

#include "intmath.h"
#include "decode.h"
#include "cabac_data.h"
#include "bitstream.h"


//#define CABAC_DEBUG(a...) printf(a)
#define CABAC_DEBUG(a...)

void cabac_init_data();

static should_inline uint8_t cabac_ctx_vars(int preCtxState)
{
  if (preCtxState <= 63)
    return 2 * (63 - preCtxState) + 0;
  else
    return 2 * (preCtxState - 64) + 1;
}

static inline void init_cabac_context_variables(CABACContext* cabac_ctx, int SliceQPY, unsigned int cabac_init_idc, slice_type_t slice_type_modulo5)
{
  int i;
  int preCtxState;

  SliceQPY = im_clip(0, 51, SliceQPY);


  // TODO: remove once debugged
  memset(cabac_ctx->cabac_ctx_vars, -1, 460);

  switch (slice_type_modulo5)
  {
    case SI_SLICE:
      for (i = 0; i <= 10; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_I[i][0] * SliceQPY) >> 4 ) + cabac_context_init_I[i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      for (i = 60; i <= 398; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_I[i][0] * SliceQPY) >> 4 ) + cabac_context_init_I[i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      break;

    case I_SLICE:
      for (i = 3; i <= 10; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_I[i][0] * SliceQPY) >> 4 ) + cabac_context_init_I[i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      for (i = 60; i <= 459; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_I[i][0] * SliceQPY) >> 4 ) + cabac_context_init_I[i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      break;

    case P_SLICE:
    case SP_SLICE:
      for (i = 11; i <= 23; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_PB[cabac_init_idc][i][0] * SliceQPY) >> 4 ) + cabac_context_init_PB[cabac_init_idc][i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      for (i = 40; i <= 459; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_PB[cabac_init_idc][i][0] * SliceQPY) >> 4 ) + cabac_context_init_PB[cabac_init_idc][i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      break;

    case B_SLICE:
      for (i = 24; i <= 459; i++)
      {
        preCtxState = im_clip( 1, 126, ((cabac_context_init_PB[cabac_init_idc][i][0] * SliceQPY) >> 4 ) + cabac_context_init_PB[cabac_init_idc][i][1]);
        cabac_ctx->cabac_ctx_vars[i] = cabac_ctx_vars(preCtxState);
      }
      break;

    default:
      LUD_DEBUG_ASSERT(0); // Should not happen !
      break;
  }
}

static should_inline void init_cabac_decoding_engine(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  cabac_ctx->codIRange = 0x01FE;
  cabac_ctx->codIOffset = bs_read_un(bs, 9);
  LUD_DEBUG_ASSERT(cabac_ctx->codIOffset < cabac_ctx->codIRange);

  // cubu debug
  CABAC_DEBUG("\ninitialize:[");
  int i;
  for (i = 8; i >= 0; i--)
    CABAC_DEBUG("%d", (cabac_ctx->codIOffset>>i) & 1);
}

static should_inline void renormalization_process(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  while (cabac_ctx->codIRange < 0x0100)
  {
    cabac_ctx->codIRange = cabac_ctx->codIRange << 1;
    cabac_ctx->codIOffset = cabac_ctx->codIOffset << 1 | bs_read_u1(bs);
    // cubu debug
    CABAC_DEBUG("%d", cabac_ctx->codIOffset & 1);
  }
}

static should_inline unsigned int decode_cabac_1bit(CABACContext* cabac_ctx, BitStreamContext* bs, uint8_t* state_vars)
{
  unsigned int pStateIdx = (*state_vars) >> 1;
  unsigned int valMPS = (*state_vars) & 1;
  unsigned int binVal;
  unsigned int qCodIRangeIdx = (cabac_ctx->codIRange >> 6) & 3;
  unsigned int codIRangeLPS = rangeTabLPS[pStateIdx][qCodIRangeIdx];
  cabac_ctx->codIRange -= codIRangeLPS;

  if (cabac_ctx->codIOffset >= cabac_ctx->codIRange)
  {
    binVal = !valMPS;
    cabac_ctx->codIOffset -= cabac_ctx->codIRange;
    cabac_ctx->codIRange = codIRangeLPS;
    *state_vars = transIdxLPS_mod[*state_vars];
  }
  else
  {
    binVal = valMPS;
    *state_vars = transIdxMPS_mod[*state_vars];
  }

  renormalization_process(cabac_ctx, bs);

  return binVal;
}

static should_inline unsigned int decode_cabac_bypass_bit(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  cabac_ctx->codIOffset = (cabac_ctx->codIOffset << 1) | bs_read_u1(bs);

  // cubu debug
  CABAC_DEBUG("%d", cabac_ctx->codIOffset & 1);

  unsigned int test = (cabac_ctx->codIOffset >= cabac_ctx->codIRange);
  cabac_ctx->codIOffset -= test * cabac_ctx->codIRange;
  return test;
}


static should_inline unsigned int decode_cabac_termination_bit(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  cabac_ctx->codIRange -= 2;

  if (cabac_ctx->codIOffset >= cabac_ctx->codIRange)
    return 1;

  renormalization_process(cabac_ctx, bs);
  return 0;
}


//
// TODO: optimization: add 2 row of dummy MbAttr at the beginning of the buffer. Those dummy MbAttr would have slice_num=0xFFFF(~unreachable)
//    gain => do not have to test mb_col and mb_row anymore !! (Actually, it can be done in a lot of locations in the code !)
//

// Utils functions

// mbA and mnB neighbors are needed twice during cabac decoding. This is the reason why this function is defined here
// The process is the one from section 6.4.8.1
static should_inline void cabac_derive_mb_neighbors(CABACContext* cabac_ctx, MbAttrib* curr_mb_attr,
    unsigned int curr_is_bot, unsigned int mb_field_decoding_flag, unsigned int PicWidthInMbs,
    unsigned int MbaffFrameFlag)
{

  if (MbaffFrameFlag)
  {
    cabac_ctx->mbB = curr_mb_attr->up_mb_is_available ?
        get_up_mbaff_mb(curr_mb_attr, PicWidthInMbs, mb_field_decoding_flag, curr_mb_attr->up_mb_is_field, curr_is_bot) :
        &undef_mb;

    if (!curr_mb_attr->left_mb_is_available)
    {
      cabac_ctx->mbA = &undef_mb;
      cabac_ctx->cbpA = -1;
    }
    else
    {
      unsigned int left_mb_is_field = curr_mb_attr->left_mb_is_field;
      cabac_ctx->mbA = get_left_mbaff_mb(curr_mb_attr, PicWidthInMbs, mb_field_decoding_flag, left_mb_is_field, curr_is_bot, 0);
      if (mb_field_decoding_flag ^ left_mb_is_field)
      {
        MbAttrib* mbA = curr_mb_attr -1 - curr_is_bot*PicWidthInMbs;
        if (mb_field_decoding_flag)
          cabac_ctx->cbpA = (mbA->CodedBlockPatternLuma & 2) | (((mbA+PicWidthInMbs)->CodedBlockPatternLuma & 2)<<2);
        else
        {
          if (curr_is_bot)
            cabac_ctx->cbpA = (mbA->CodedBlockPatternLuma & 8) | ((mbA->CodedBlockPatternLuma & 8)>>2);
          else
            cabac_ctx->cbpA = (mbA->CodedBlockPatternLuma & 2) | ((mbA->CodedBlockPatternLuma & 2)<<2);
        }
      }
      else
        cabac_ctx->cbpA = cabac_ctx->mbA->CodedBlockPatternLuma;
    }
  }
  else
  {
    cabac_ctx->mbA = curr_mb_attr->left_mb_is_available ? curr_mb_attr-1 : &undef_mb;
    cabac_ctx->mbB = curr_mb_attr->up_mb_is_available ? curr_mb_attr-PicWidthInMbs : &undef_mb;
    cabac_ctx->cbpA = cabac_ctx->mbA->CodedBlockPatternLuma;
  }

}

// Syntax element decode functions


static should_inline unsigned int cabac_decode_mb_skip_flag(CABACContext* cabac_ctx, BitStreamContext* bs,
    MbAttrib* curr_mb_attr, slice_type_t slice_type_modulo5)
{
  //cubu debug
  CABAC_DEBUG("]\nmb_skip_flag:[");

  int ctxIdx = slice_type_modulo5 == B_SLICE ? 24 : 11;

  MbAttrib* mbA = cabac_ctx->mbA;
  MbAttrib* mbB = cabac_ctx->mbB;

  ctxIdx += curr_mb_attr->left_mb_is_available && !mbA->mb_skip_flag;
  ctxIdx += curr_mb_attr->up_mb_is_available && !mbB->mb_skip_flag;

  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}


static should_inline unsigned int cabac_decode_end_of_slice_flag(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  //cubu debug
  CABAC_DEBUG("]\nend_of_slice_flag:[");

  return decode_cabac_termination_bit(cabac_ctx, bs);
}


static should_inline unsigned int cabac_decode_mb_field_decoding_flag(CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr_mb_attr,
    unsigned int mb_row, unsigned int PicWidthInMbs, unsigned int slice_num)
{
  //cubu debug
  CABAC_DEBUG("]\nmb_field_decoding_flag:[");

  MbAttrib* mbB = curr_mb_attr - (PicWidthInMbs << (mb_row&1));
  unsigned int ctxIdx = 70;

  ctxIdx += curr_mb_attr->left_mb_is_available && curr_mb_attr->left_mb_is_field;
  ctxIdx += (mb_row>1) && (mbB->slice_num == slice_num) && mbB->mb_field_decoding_flag; // cannot use up_mb_is_XXX flags because here the upper pair is needed

  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}

static should_inline MB_TYPE cabac_decode_I_mb_type(CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr,
    unsigned int ctxIdxOffset, unsigned int is_intra_slice)
{
  MbAttrib* mbA = cabac_ctx->mbA;
  MbAttrib* mbB = cabac_ctx->mbB;

  unsigned int condTermFlagA;
  unsigned int condTermFlagB;
  MB_TYPE mb_type;
  uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[ctxIdxOffset];

  if (is_intra_slice)
  {
    unsigned int ctxIdxInc;
    condTermFlagA = ! (!curr->left_mb_is_available
                    || (ctxIdxOffset==0 && mbA->mb_type == SI)
                    || (ctxIdxOffset==3 && mbA->mb_type == I_NxN)
                    );
    condTermFlagB = ! (!curr->up_mb_is_available
                    || (ctxIdxOffset==0 && mbB->mb_type == SI)
                    || (ctxIdxOffset==3 && mbB->mb_type == I_NxN)
                    );
    ctxIdxInc = condTermFlagA + condTermFlagB;

    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
      return I_NxN;
    cabac_ctx_vars += 2;
  }
  else // is inter slice
  {
    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[0]))
      return I_NxN;
  }

  if (decode_cabac_termination_bit(cabac_ctx, bs))
    return I_PCM;

  mb_type = 1;
  mb_type += 12 * decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[1]); // b2
  if (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2])) // b3
    mb_type += 4 + 4 * decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2+is_intra_slice]); // b4
  mb_type += 2 * decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3+is_intra_slice]); // b4 or b5
  mb_type += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3+2*is_intra_slice]); // b5 or b6

  return mb_type;
}

static should_inline MB_TYPE cabac_decode_mb_type(CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr, slice_type_t slice_type_modulo5)
{
  //cubu debug
  CABAC_DEBUG("\nmb_type:[");


  if (slice_type_modulo5 == B_SLICE)
  {
    MbAttrib* mbA = cabac_ctx->mbA;
    MbAttrib* mbB = cabac_ctx->mbB;
    unsigned int condTermFlagA;
    unsigned int condTermFlagB;
    uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[27];
    condTermFlagA = ! (!curr->left_mb_is_available
                    || (mbA->mb_type == B_Skip || mbA->mb_type == B_Direct_16x16)
                    );
    condTermFlagB = ! (!curr->up_mb_is_available
                    || (mbB->mb_type == B_Skip || mbB->mb_type == B_Direct_16x16)
                    );
    cabac_ctx_vars += condTermFlagA + condTermFlagB;

    if (!decode_cabac_1bit(cabac_ctx, bs, cabac_ctx_vars)) // b0
      return B_Direct_16x16;

    cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[27];
    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3])) // b1
      return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]) + B_L0_16x16; // b2 [5]

    unsigned int bins;
    bins =  decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[4]) << 3;// b2 [4]
    bins |= decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]) << 2;// b3 [5]
    bins |= decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]) << 1;// b4 [5]
    bins |= decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]) << 0;// b5 [5]

    if (bins < 8)
      return B_Bi_16x16 + bins;
    else if (bins < 13)
      return (bins<<1) - 16 + B_L0_Bi_16x8 + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]);
    else if (bins == 13)
      return cabac_decode_I_mb_type(cabac_ctx, bs, curr, 32, 0);
    else // bins == 14 or bins == 15
      return bins == 14 ? B_L1_L0_8x16 : B_8x8;
  }
  else if (slice_type_modulo5 == P_SLICE || slice_type_modulo5 == SP_SLICE )
  {
    unsigned int b1, b2;
    uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[14];

    if (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[0]))// b0 [0]
      return cabac_decode_I_mb_type(cabac_ctx, bs, curr, 17, 0);

    b1 = decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[1]);// b1 [1]
    b2 = decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2 + b1]);// b2 [2, 3]
    static const MB_TYPE c[2][2] = {{P_L0_16x16, P_8x8}, {P_L0_L0_8x16, P_L0_L0_16x8}};
    return c[b1][b2];
  }
  else if (slice_type_modulo5 == I_SLICE)
  {
    return cabac_decode_I_mb_type(cabac_ctx, bs, curr, 3, 1);
  }
  else
  {
    LUD_DEBUG_ASSERT(slice_type_modulo5 == SI_SLICE);
    MbAttrib* mbA = cabac_ctx->mbA;
    MbAttrib* mbB = cabac_ctx->mbB;
    unsigned int condTermFlagA;
    unsigned int condTermFlagB;
    uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[0];
    condTermFlagA = ! (!curr->left_mb_is_available
                    || (mbA->mb_type == SI)
                    );
    condTermFlagB = ! (!curr->up_mb_is_available
                    || (mbB->mb_type == SI)
                    );

    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[0+condTermFlagA+condTermFlagB]))// b0 [0]
      return SI;
    else
      return cabac_decode_I_mb_type(cabac_ctx, bs, curr, 3, 1);

  }
}



static should_inline SUB_MB_TYPE cabac_decode_sub_mb_type(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int is_B_mb)
{
  //cubu debug
  CABAC_DEBUG("]\nsub_mb_type:[");


  if (is_B_mb)
  {
    uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[36];

    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[0]))
      return B_Direct_8x8;

    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[1]))
      return B_L0_8x8 + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]);

    SUB_MB_TYPE type = B_Bi_8x8;
    if (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2]))
    {
      if (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]))
        return B_L1_4x4 + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]);

      type += 4;
    }

    return type + (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3])<<1) + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]);
  }
  else // P / SP Slices
  {
    uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[21];

    if (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[0]))
      return P_L0_8x8;

    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[1]))
      return P_L0_8x4;

    return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2]) ? P_L0_4x8 : P_L0_4x4;
  }
}

static should_inline unsigned int cabac_decode_transform_size_8x8_flag(CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr_mb_attr)
{
  //cubu debug
  CABAC_DEBUG("]\ntransform_size_8x8_flag:[");

  //ctxIdxOffset 399
  int ctxIdx = 399;

  MbAttrib* mbA = cabac_ctx->mbA;
  MbAttrib* mbB = cabac_ctx->mbB;

  ctxIdx += curr_mb_attr->left_mb_is_available && mbA->transform_size_8x8_flag;
  ctxIdx += curr_mb_attr->up_mb_is_available && mbB->transform_size_8x8_flag;

  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}

static should_inline void cabac_decode_coded_block_pattern(unsigned int* CodedBlockPatternLuma, unsigned int* CodedBlockPatternChroma,
    CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr, unsigned int cfidc)
{
  //cubu debug
  CABAC_DEBUG("]\ncoded_block_pattern:[");

  // prefix ctxIdxOffset 73
  // suffix ctxIdxOffset 77
  unsigned int cbp, cbpA, cbpB;
  MbAttrib* mbA = cabac_ctx->mbA;
  MbAttrib* mbB = cabac_ctx->mbB;
  unsigned int ctxIdxInc;
  uint8_t *cabac_ctx_vars;


  // The spec also state that mbA must not be I_PCM, B_Skip or P_Skip (cf 9.3.3.1.1.4), but the test is not
  // needed here because CodedBlockPatternLuma value has been set to behave accordingly (-1 for I_PCM and 0 for Skip)
  // => save extra tests
  cbpA = cabac_ctx->cbpA;
  cbpB = mbB->CodedBlockPatternLuma;
  cbp = 0;
  //condTermFlagN = !(cbpN & (1<<luma8x8BlkIdxN))

  cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[73];
  ctxIdxInc = !(cbpA&2) + 2 * !(cbpB&4);
  cbp  = decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]);
  ctxIdxInc = !(cbp &1) + 2 * !(cbpB&8);
  cbp += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]) << 1;
  ctxIdxInc = !(cbpA&8) + 2 * !(cbp &1);
  cbp += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]) << 2;
  ctxIdxInc = !(cbp &4) + 2 * !(cbp &2);
  cbp += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]) << 3;

  *CodedBlockPatternLuma = cbp;

  if (cfidc > 0)
  {
    cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[77];
    cbpA = mbA->CodedBlockPatternChroma;
    cbpB = mbB->CodedBlockPatternChroma;

    ctxIdxInc = (cbpA != 0) + 2 * (cbpB != 0);
    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
      *CodedBlockPatternChroma = 0;
    else
    {
      ctxIdxInc = 4 + (cbpA == 2) + 2 * (cbpB == 2);
      *CodedBlockPatternChroma = 1 + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]);
    }
  }
}

static should_inline int cabac_decode_mb_qp_delta(CABACContext* cabac_ctx, BitStreamContext* bs, int prevMbAddr_mb_qp_delta)
{
  //cubu debug
  CABAC_DEBUG("]\nmb_qp_delta:[");

  // ctxIdxOffset:60, section 9.3.3.7
  unsigned int ctxIdxInc;
  uint8_t *cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[60];
  int v;

  ctxIdxInc = prevMbAddr_mb_qp_delta != 0; // slice_num test is not needed since we are decoding the same slice !

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
    return 0;

  if(!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[2]))
    return 1;

  v = 2+1;
  while (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]))
  {
    v++;
    LUD_DEBUG_ASSERT(v < 200);
  }

  return v&1 ? -(v>>1) : (v>>1);
  // return (v>>1) - (v&1)*v; // better if mult is faster than a test...
}

static should_inline unsigned int cabac_decode_prev_intra_pred_mode_flag(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  //cubu debug
  CABAC_DEBUG("]\nintra_pred_mode_flag:[");

  // ctxIdxOffset: 68
  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[68]);
}

static should_inline unsigned int cabac_decode_rem_intra_pred_mode(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  //cubu debug
  CABAC_DEBUG("]\nrem_intra_pred_mode:[");

  // ctxIdxOffset: 69
  unsigned int bins;

  bins = decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[69]);
  bins += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[69]) << 1;
  bins += decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[69]) << 2;

  return bins;
}

static should_inline unsigned int cabac_decode_intra_chroma_pred_mode(CABACContext* cabac_ctx, BitStreamContext* bs, MbAttrib* curr)
{
  //cubu debug
  CABAC_DEBUG("]\nintra_chroma_pred_mode:[");

  // ctxIdxOffset: 64
  unsigned int ctxIdxInc;
  MbAttrib* mbA = cabac_ctx->mbA;
  MbAttrib* mbB = cabac_ctx->mbB;
  uint8_t* cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[64];

  // TODO: OPTIMIZE: for all functions that test slice_num, check if mbA could be set to undef_mb when not in the same slice_num, and set the undef_mb
  // with the good values for all tests. For example here intra_chroma_pred_mode would be set to 0.
  // => this would save one test and the use of curr variable !

  ctxIdxInc  = curr->left_mb_is_available && (IS_INTRA(mbA)) && (mbA->pred.intra.intra_chroma_pred_mode != 0);
  ctxIdxInc += curr->up_mb_is_available && (IS_INTRA(mbB)) && (mbB->pred.intra.intra_chroma_pred_mode != 0);

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
    return 0;

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]))
    return 1;

  return 2 + decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[3]);
}

static should_inline int8_t cabac_decode_ref_idx_lX(CABACContext* cabac_ctx, BitStreamContext* bs, int8_t refIdx_cache[25],
    int r5x5idx, int xoff, int yoff)
{
  //cubu debug
  CABAC_DEBUG("]\nref_idx_lX:[");

  // U, ctxIdxOffset: 54
  uint8_t* cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[54];
  unsigned int ctxIdxInc = (refIdx_cache[r5x5idx-xoff]>0) + ((refIdx_cache[r5x5idx-yoff]>0) << 1);

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
    return 0;

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[4]))
    return 1;

  int val = 2;
  while (decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[5]))
    val++;

  return val;
}

static should_inline int16_t cabac_decode_mvd_lX(CABACContext* cabac_ctx, BitStreamContext* bs, int16_t mv_cache[25][2],
    int r5x5idx, unsigned int compIdx, int xoff, int yoff)
{
  //cubu debug
  CABAC_DEBUG("]\nmvd_lX:[");

  // x comp, UEG3, ctxIdxOffset: 40
  // y comp, UEG3,  ctxIdxOffset: 47
  static const unsigned int t[2][2] = {{1, 2}, {0, N_A}}; //[absMvdComp < 3][absMvdComp > 32]
  static const unsigned int i[2] = {40, 47};

  unsigned int ctxIdxOffset = i[compIdx];
  uint8_t* cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[ctxIdxOffset];
  unsigned int absMvdComp = im_abs(mv_cache[r5x5idx-xoff][compIdx]) + im_abs(mv_cache[r5x5idx-yoff][compIdx]);
  unsigned int ctxIdxInc = t[absMvdComp < 3][absMvdComp > 32];

  if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
    return 0;


  int mvd = 1;
  ctxIdxInc = 3;
  while (mvd<9 && decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
  {
    mvd++;
    if (ctxIdxInc < 6)
      ctxIdxInc++;
  }

  unsigned int k = 3;
  if (mvd == 9)
  {
    while(decode_cabac_bypass_bit(cabac_ctx, bs))
    {
      mvd += 1<<k;
      k++;
    }
    while(k--)
      mvd += decode_cabac_bypass_bit(cabac_ctx, bs) << k;
  }

  int s[2] = {mvd, -mvd};
  return s[decode_cabac_bypass_bit(cabac_ctx, bs)];
}

static should_inline unsigned int cabac_decode_coded_block_flag(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int ctxBlockCat, uint8_t* nC_cache, unsigned int cfidc, unsigned int lc4x4blockIdx)
{
  //cubu debug
  CABAC_DEBUG("]\ncoded_block_flag:[");

  // FL:1, ctxIdxOffset: 85

  unsigned int condTermFlagA, condTermFlagB;

  if (ctxBlockCat == 0 || ctxBlockCat == 3)
  {
    condTermFlagA = nC_cache[26] != 0;
    condTermFlagB = nC_cache[27] != 0;
  }
  else if (ctxBlockCat == 1 || ctxBlockCat == 2)
  {
    unsigned int idx = scan4x4toraster5x5[lc4x4blockIdx];
    condTermFlagA = nC_cache[idx-1] != 0;
    condTermFlagB = nC_cache[idx-5] != 0;
  }
  else // ctxBlockCat == 4
  {
    unsigned int idx = cfidc < 3 ? raster2x4toraster5x5[lc4x4blockIdx] : scan4x4toraster5x5[lc4x4blockIdx];
    condTermFlagA = nC_cache[idx-1] != 0;
    condTermFlagB = nC_cache[idx-5] != 0;
  }

  unsigned int ctxIdx = 85 + condTermFlagA + condTermFlagB*2 + coded_block_flag_ctxIdxBlockCatOffset[ctxBlockCat];
  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}


static should_inline unsigned int cabac_decode_significant_coeff_flag(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int ctxBlockCat, unsigned int levelListIdx, unsigned int mb_field_decoding_flag, unsigned int cfidc)
{
  //cubu debug
  CABAC_DEBUG("]\nsignificant_coeff_flag:[");

// FL:1
  // field, ctxBlockCat <5, ctxIdxOffset: 277
  // field, ctxBlockCat==5, ctxIdxOffset: 436
  // frame, ctxBlockCat <5, ctxIdxOffset: 105
  // frame, ctxBlockCat==5, ctxIdxOffset: 402

  // [ctxBlockCat <5][mb_field_decoding_flag]
  static const unsigned int ctxIdxOffset[2][2] = {{402, 436}, {105, 277}};

  unsigned int ctxIdxInc;

  if (ctxBlockCat < 5 && ctxBlockCat != 3)
    ctxIdxInc = levelListIdx;
  else if (ctxBlockCat == 3)
    ctxIdxInc = im_min(levelListIdx>>(MbNumC8x8[cfidc]-1), 2);
  else // ctxBlockCat == 5
    ctxIdxInc = significant_coeff_flag_ctxIdxInc[mb_field_decoding_flag][levelListIdx];

  unsigned int ctxIdx = ctxIdxOffset[ctxBlockCat<5][mb_field_decoding_flag] + ctxIdxInc + significant_coeff_flag_ctxIdxBlockCatOffset[ctxBlockCat];
  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}

static should_inline unsigned int cabac_decode_last_significant_coeff_flag(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int ctxBlockCat, unsigned int levelListIdx, unsigned int mb_field_decoding_flag, unsigned int cfidc)
{
  //cubu debug
  CABAC_DEBUG("]\nlast_significant_coeff_flag:[");

  // FL:1
  // field, ctxBlockCat <5, ctxIdxOffset: 338
  // field, ctxBlockCat==5, ctxIdxOffset: 451
  // frame, ctxBlockCat <5, ctxIdxOffset: 166
  // frame, ctxBlockCat==5, ctxIdxOffset: 417

  // [ctxBlockCat <5][mb_field_decoding_flag]
  static const unsigned int ctxIdxOffset[2][2] = {{417, 451}, {166, 338}};

  unsigned int ctxIdxInc;

  if (ctxBlockCat < 5 && ctxBlockCat != 3)
    ctxIdxInc = levelListIdx;
  else if (ctxBlockCat == 3)
    ctxIdxInc = im_min(levelListIdx>>(MbNumC8x8[cfidc]-1), 2);
  else // ctxBlockCat == 5
    ctxIdxInc = last_significant_coeff_flag_ctxIdxInc[levelListIdx];

  unsigned int ctxIdx = ctxIdxOffset[ctxBlockCat<5][mb_field_decoding_flag] + ctxIdxInc + last_significant_coeff_flag_ctxIdxBlockCatOffset[ctxBlockCat];
  return decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx->cabac_ctx_vars[ctxIdx]);
}


static should_inline unsigned int cabac_decode_coeff_abs_level_minus1(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int numDecodAbsLevelEq1, unsigned int numDecodAbsLevelGt1, unsigned int ctxBlockCat)
{
  //cubu debug
  CABAC_DEBUG("]\ncoeff_abs_level_minus1:[");

  // ctxBlockCat <5
  // prefix UEG0, signedValFlag=0, uCoff=14, ctxIdxOffset: 227
  // suffix UEG0, signedValFlag=0, uCoff=14, Bypass
  // ctxBlockCat==5
  // prefix UEG0, signedValFlag=0, uCoff=14, ctxIdxOffset: 426
  // suffix UEG0, signedValFlag=0, uCoff=14, Bypass

  unsigned int ctxIdxOffset = (ctxBlockCat<5 ? 227 : 426) + coeff_abs_level_minus1_ctxIdxBlockCatOffset[ctxBlockCat];
  uint8_t* cabac_ctx_vars = &cabac_ctx->cabac_ctx_vars[ctxIdxOffset];
  unsigned int ctxIdxInc = ((numDecodAbsLevelGt1 != 0) ? 0 : im_min(4, 1+numDecodAbsLevelEq1));
  unsigned int coeff = 0;

  for (coeff = 0; coeff < 14; ++coeff)
  {
    if (!decode_cabac_1bit(cabac_ctx, bs, &cabac_ctx_vars[ctxIdxInc]))
      return coeff;
    if (!coeff)
      ctxIdxInc = 5 + im_min(4-(ctxBlockCat==3), numDecodAbsLevelGt1);
  }


  unsigned int k = 0;
  while(decode_cabac_bypass_bit(cabac_ctx, bs))
  {
    coeff += 1<<k;
    k++;
  }
  while(k--)
    coeff += decode_cabac_bypass_bit(cabac_ctx, bs) << k;

  return coeff;
}


static should_inline unsigned int cabac_decode_coeff_sign_flag(CABACContext* cabac_ctx, BitStreamContext* bs)
{
  //cubu debug
  CABAC_DEBUG("]\ncoeff_sign_flag:[");

  // FL:1, Bypass
  return decode_cabac_bypass_bit(cabac_ctx, bs);
}

#endif /* CABAC_H_ */
