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


#include <string.h>
#include "syntax.h"
#include "bitstream.h"
#include "defaulttables.h"
#include "intmath.h"
#include "inverse_transforms.h"
#include "decode_slice.h"
#include "cabac.h"
#include <math.h>

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif// Place the next variable in SRAM
pixel_t topy[30720];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
pixel_t topcb[7680];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
pixel_t topcr[7680];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
pixel_t ysamp[1536];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
pixel_t cbsamp[384];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
pixel_t crsamp[384];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif // Place the next variable in SRAM
MbAttrib test_array[240];


static const int8_t dc_val[2] = {2, -1};


static should_inline void resetarray()
{
    int k;
    MbAttrib* Point;
    for(k=0;k<120;k++)
    {
        Point = test_array + 120 + k;
        test_array[k] = test_array[120+k];
        memset(Point,0,sizeof(test_array[120+k]));
    }
}


static should_inline unsigned int decode_intra_chroma_pred_mode(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag,
    MbAttrib* curr)
{
  //if (!entropy_coding_mode_flag)
  //  return bs_read_ue(bs);
  //else
    return cabac_decode_intra_chroma_pred_mode(cabac_ctx, bs, curr);
}


static should_inline void mb_pred(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag, MB_TYPE mb_type,
                           PREDICTION_MODE MbPartPredMode0, unsigned int transform_size_8x8_flag,
                           unsigned int cfidc, unsigned int MbaffFrameFlag, unsigned int mb_field_decoding_flag, unsigned int constrained_intra_pred_flag,
                           uint8_t num_ref_idx_active[2], MbAttrib* curr_mb_attr, unsigned int curr_is_bot, int PicWidthInMbs
                           )
{
  if( MbPartPredMode0 <= Intra_16x16 ) // Intra_4x4 Intra_8x8 or Intra_16x16
  {
    if( cfidc != 0 )
      curr_mb_attr->pred.intra.intra_chroma_pred_mode = decode_intra_chroma_pred_mode(cabac_ctx, bs, entropy_coding_mode_flag, curr_mb_attr); // 2 ue(v) | ae(v)
  }
}


// nC_cache is a 5x5 or 5x3 array that contains nC values for neighboring blocks
// table_flag==0 specify that we are in 5x5 neighboring or else that we have 5x3 neighboring


static should_inline void reset_one_nC_cache_line(uint8_t luma_nC_cache[28], uint8_t chroma_nC_cache[2][28], unsigned int is_horiz,
    unsigned int v, unsigned int cfidc)
{
  unsigned int i, iCbCr;
  unsigned int stride = is_horiz ? 1 : 5;

  for (i = 0; i < 4; ++i)
    luma_nC_cache[stride+i*stride] = v;

  // used for luma DC coeffs: 26: mbA, 27:mbB
  luma_nC_cache[26+is_horiz] = v;

  if (cfidc > 0)
  {
    unsigned int nb = (is_horiz ? MbWidthCdiv4 : MbHeightCdiv4)[cfidc];
    for (iCbCr = 0; iCbCr < 2; ++iCbCr)
    {
      for (i = 0; i < nb; ++i)
        chroma_nC_cache[iCbCr][stride+i*stride] = v;

      // used for chroma DC coeffs: 26: mbA, 27:mbB
      chroma_nC_cache[iCbCr][26+is_horiz] = v;
    }
  }
}

static should_inline void set_one_nC_cache_line(uint8_t luma_nC_cache[28], uint8_t chroma_nC_cache[2][28], unsigned int test, unsigned int is_horiz,
    MbAttrib* mb_attr, unsigned int cfidc)
{

  if (test)
  {
    reset_one_nC_cache_line(luma_nC_cache, chroma_nC_cache, is_horiz, 0, cfidc);
    return;
  }

  unsigned int i, iCbCr;
  unsigned int stride = is_horiz ? 1 : 5;
  unsigned int srcstride = is_horiz ? 1 : 4;
  unsigned int srcoffset = is_horiz ? 12 : 3;

  for (i = 0; i < 4; ++i)
    luma_nC_cache[stride+i*stride] = mb_attr->luma_total_coeff[i*srcstride+srcoffset];

  // used for luma DC coeffs: 26: mbA, 27:mbB
  luma_nC_cache[26+is_horiz] = mb_attr->luma_dc_coeffs_non_null;

  if (cfidc > 0)
  {
    unsigned int nb = (is_horiz ? MbWidthCdiv4 : MbHeightCdiv4)[cfidc];
    srcstride = is_horiz ? 1 : MbWidthCdiv4[cfidc];
    srcoffset = is_horiz ? MbSizeCdiv16[cfidc]-MbWidthCdiv4[cfidc] : MbWidthCdiv4[cfidc]-1;
    for (iCbCr = 0; iCbCr < 2; ++iCbCr)
    {
      for (i = 0; i < nb; ++i)
        chroma_nC_cache[iCbCr][stride+i*stride] = mb_attr->chroma_total_coeff[iCbCr][i*srcstride+srcoffset];

      // used for chroma DC coeffs: 26: mbA, 27:mbB
      chroma_nC_cache[iCbCr][26+is_horiz] = mb_attr->chroma_dc_coeffs_non_null & (1<<iCbCr);
    }
  }
}



static should_inline void fills_in_nC_cache_cabac(MbAttrib* curr_mb_attr, unsigned int curr_is_bot, unsigned int curr_is_field,
    int PicWidthInMbs, uint8_t luma_nC_cache[28], uint8_t chroma_nC_cache[2][28], unsigned int cfidc, MB_TYPE mb_type,
    unsigned int constrained_intra_pred_flag, unsigned int data_partitioning_slice_flag, unsigned int MbaffFrameFlag)
{

  unsigned int common = constrained_intra_pred_flag && is_IntraMb(mb_type) && data_partitioning_slice_flag;

  if (!MbaffFrameFlag)
  {
    // upper row
    if (!curr_mb_attr->up_mb_is_available)
      reset_one_nC_cache_line(luma_nC_cache, chroma_nC_cache, 1, is_IntraMb(mb_type), cfidc);
    else
    {
      MbAttrib* up = curr_mb_attr-PicWidthInMbs;
      set_one_nC_cache_line(luma_nC_cache, chroma_nC_cache, common && IS_INTER(up), 1, up, cfidc);
    }

    // left column
    if (!curr_mb_attr->left_mb_is_available)
      reset_one_nC_cache_line(luma_nC_cache, chroma_nC_cache, 0, is_IntraMb(mb_type), cfidc);
    else
    {
      MbAttrib* left = curr_mb_attr-1;
      set_one_nC_cache_line(luma_nC_cache, chroma_nC_cache, common && IS_INTER(left), 0, left, cfidc);
    }
  }

}


static should_inline void writes_back_nC_cache(MbAttrib* mb_attr,
                                        uint8_t luma_nC_cache[28], uint8_t chroma_nC_cache[2][28],
                                        unsigned int cfidc)
{
  unsigned int iCbCr;
  int i;

  // LUMA
  for( i=0; i<16; i++)
    mb_attr->luma_total_coeff[i] = luma_nC_cache[raster4x4toraster5x5[i]];

  // CHROMA
  if (cfidc == 1)
  {
    for (iCbCr = 0; iCbCr < 2; iCbCr++)
      for( i=0; i<4; i++)
        mb_attr->chroma_total_coeff[iCbCr][i] = chroma_nC_cache[iCbCr][raster2x4toraster5x5[i]];
  }
  else if (cfidc == 2)
  {
    for (iCbCr = 0; iCbCr < 2; iCbCr++)
      for( i=0; i<8; i++)
        mb_attr->chroma_total_coeff[iCbCr][i] = chroma_nC_cache[iCbCr][raster2x4toraster5x5[i]];
  }
  else if (cfidc == 3)
  {
    for (iCbCr = 0; iCbCr < 2; iCbCr++)
      for( i=0; i<16; i++)
        mb_attr->chroma_total_coeff[iCbCr][i] = chroma_nC_cache[iCbCr][raster4x4toraster5x5[i]];
  }
}



static should_inline void scale_coeff(int16_t* coeffLevel, const uint8_t* scan_table, int coeffNum, int level,
                               const uint16_t* LevelScale, int two_pow_qp_div6, RESIDUAL_BLOCK_TYPE block_type, int bypass)
{
  int i = scan_table[coeffNum];

  if (bypass)
  {
    coeffLevel[i] = (int16_t) level;
  }
  else if (block_type == CHROMA_DC_LEVEL)
  {
    // cfidc == 1 = >coeffLevel[i] = (int16_t) ((level * (int)LevelScale[0] * two_pow_qp_div6)>>5); // the multiplication must be done AFTER the hadamart transform
    //else => coeffLevel[i] = (int16_t) ((level * (int)LevelScale[0] * two_pow_qp_div6 + (1<<5))>>6); // the multiplication must be done AFTER the hadamart transform
    coeffLevel[i] = (int16_t) level;
  }
  else if (block_type == LUMA_DC_LEVEL)
  {
    // coeffLevel[i] = (int16_t) ((level * (int)LevelScale[0] * two_pow_qp_div6 + (1<<5))>>6);  // the multiplication must be done AFTER the hadamart transform
    coeffLevel[i] = (int16_t) level;
  }
  else // (block_type == INTRA_AC_LEVEL or LUMA_LEVEL or CHROMA_AC_LEVEL)
  {
    coeffLevel[i] = (int16_t) ((level * (int)LevelScale[i] * two_pow_qp_div6 + (1<<3))>>4);
  }
}

// decode residual with CAVLC method. It means that this function should be called only when entropy_coding_mode_flag==0



static should_inline unsigned int residual_block_cabac(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int lc4x4blockIdx, int16_t* coeffLevel, unsigned int maxNumCoeff, RESIDUAL_BLOCK_TYPE block_type,
    uint8_t* nC_cache, unsigned int cfidc, const uint8_t* scan_table, const uint16_t* LevelScale, int two_pow_qp_div6,
    int bypass, unsigned int mb_field_decoding_flag)
{
  unsigned int ctxBlockCat = block_type;
  unsigned int numCoeff = 0;
  uint8_t significant_coeff_flag[64];
  int i;
  unsigned int coded_block_flag;
  unsigned int numDecodAbsLevelGt1 = 0;
  unsigned int numDecodAbsLevelEq1 = 0;

  if( maxNumCoeff == 64 )
    coded_block_flag = 1;
  else
    coded_block_flag = cabac_decode_coded_block_flag(cabac_ctx, bs, ctxBlockCat, nC_cache, cfidc, lc4x4blockIdx);

  if( coded_block_flag )
  {
    int coeff_abs_level_minus1;
    unsigned int coeff_sign_flag;
    numCoeff = maxNumCoeff;
    i=0;
    do
    {
      significant_coeff_flag[i] = cabac_decode_significant_coeff_flag(cabac_ctx, bs, ctxBlockCat, i, mb_field_decoding_flag, cfidc);
      if( significant_coeff_flag[i] )
      {
        unsigned int last_significant_coeff_flag = cabac_decode_last_significant_coeff_flag(cabac_ctx, bs,
            ctxBlockCat, i, mb_field_decoding_flag, cfidc);
        if( last_significant_coeff_flag )
          numCoeff = i + 1; // not necessary to 0 all the remaining coeffs since they have been initialized to 0 already
      }
      i++;
    } while( i < numCoeff - 1 );

    int level;
    unsigned int count = 1;
    coeff_abs_level_minus1 = cabac_decode_coeff_abs_level_minus1(cabac_ctx, bs, numDecodAbsLevelEq1, numDecodAbsLevelGt1, ctxBlockCat);
    numDecodAbsLevelGt1 += coeff_abs_level_minus1 > 0;
    numDecodAbsLevelEq1 += coeff_abs_level_minus1 == 0;
    coeff_sign_flag = cabac_decode_coeff_sign_flag(cabac_ctx, bs);
    level = ( coeff_abs_level_minus1 + 1 ) *  ( 1 - 2 * coeff_sign_flag );
    scale_coeff(coeffLevel, scan_table, numCoeff - 1, level, LevelScale, two_pow_qp_div6, block_type, bypass);


    for( i = numCoeff - 2; i >= 0; i-- )
      if( significant_coeff_flag[i] )
      {
        coeff_abs_level_minus1 = cabac_decode_coeff_abs_level_minus1(cabac_ctx, bs, numDecodAbsLevelEq1, numDecodAbsLevelGt1, ctxBlockCat);
        numDecodAbsLevelGt1 += coeff_abs_level_minus1 > 0;
        numDecodAbsLevelEq1 += coeff_abs_level_minus1 == 0;
        coeff_sign_flag = cabac_decode_coeff_sign_flag(cabac_ctx, bs);
        //coeffLevel[ i ] = ( coeff_abs_level_minus1 + 1 ) * ( 1 - 2 * coeff_sign_flag );
        level = ( coeff_abs_level_minus1 + 1 ) *  ( 1 - 2 * coeff_sign_flag );
        scale_coeff(coeffLevel, scan_table, i, level, LevelScale, two_pow_qp_div6, block_type, bypass);
        count++;
      }
    numCoeff = count;
  }

  if (block_type != CHROMA_DC_LEVEL && block_type != LUMA_DC_LEVEL)
  {
    if (block_type == CHROMA_AC_LEVEL && cfidc<3)
      nC_cache[raster2x4toraster5x5[lc4x4blockIdx]] = numCoeff != 0;
    else if (block_type != LUMA8x8_LEVEL)
      nC_cache[scan4x4toraster5x5[lc4x4blockIdx]] = numCoeff;
    else
    {
      // TODO: Optimize this hack: numCoeff should be the number of coeff per 4x4 block. However when CABAC is used it can decode 8x8 block at a time
      //       so numCoeff is actually the number of coeff for the whole 8x8 block. Moreover, for CABAC decoding nC_cache is just used as a flag in
      //       order to derive coded_block_flag. => need to repeat the numCoeff value on each nC entries of the 8x8 block.
      //       However, the nC value is also used in intra/inter decoding in order to optimize the residual transform.
      //       The conclusion is: if numCoeff(8x8) = 1 => 1 is filled in the 4 nC, => residual decoding re-add the nC => give 4 instead of the original 1
      //          => suboptimize the residual transform.
      unsigned int idx = scan4x4toraster5x5[lc4x4blockIdx];
      numCoeff = (numCoeff+3) / 4; // this hacky divide preserve the non nulity of numCoeff, but keep it in the range of 0-16
      nC_cache[idx] = numCoeff;
      nC_cache[idx+1] = numCoeff;
      nC_cache[idx+5] = numCoeff;
      nC_cache[idx+6] = numCoeff;
    }
  }

  return numCoeff;
}



static should_inline unsigned int residual_block(CABACContext* cabac_ctx, BitStreamContext* bs,
    unsigned int lc4x4blockIdx, int16_t* coeffLevel, unsigned int maxNumCoeff, RESIDUAL_BLOCK_TYPE block_type,
    uint8_t* nC_cache, unsigned int cfidc, const uint8_t* scan_table, const uint16_t* LevelScale, int two_pow_qp_div6,
    int bypass, unsigned int mb_field_decoding_flag, unsigned int entropy_coding_mode_flag)
{

  //if (entropy_coding_mode_flag)
    return residual_block_cabac(cabac_ctx, bs, lc4x4blockIdx, coeffLevel, maxNumCoeff, block_type, nC_cache, cfidc,
        scan_table, LevelScale, two_pow_qp_div6, bypass, mb_field_decoding_flag);


}
// LumaLevel stands for either Intra16x16ACLevel, LumaLevel or LumaLevel8x8 according to context (transform_size_8x8_flag, MbPartPredMode)...
// Intra16x16DCLevel can be set to NULL if not processing an Intra_16x16 macroblock
static should_inline void residual(CABACContext* cabac_ctx, BitStreamContext* bs, PREDICTION_MODE MbPartPredMode0, MbAttrib* curr_mb_attr, MB_TYPE mb_type,
                            unsigned int CodedBlockPatternLuma, unsigned int CodedBlockPatternChroma, unsigned int curr_is_bot,
                            unsigned int entropy_coding_mode_flag, unsigned int transform_size_8x8_flag, unsigned int cfidc,
                            int QPprimeY, int QPprimeC[2], int16_t* mb_data, unsigned int mb_field_decoding_flag, unsigned int PicWidthInMbs,
                            uint16_t LevelScale4x4[6][6][16], uint16_t LevelScale8x8[2][6][64], unsigned int qpprime_y_zero_transform_bypass_flag,
                            unsigned int constrained_intra_pred_flag, unsigned int data_partitioning_slice_flag, unsigned int MbaffFrameFlag)
{
  // 5x5 arrays + 2 for cabac decoding
  uint8_t luma_nC_cache[28];
  uint8_t chroma_nC_cache[2][28];

  unsigned int i8x8, i4x4;
  const uint8_t* scan_table;
  int bypass = QPprimeY == 0 && qpprime_y_zero_transform_bypass_flag != 0;
  int QPprimeY_div6 = QPprimeY/6;
  int QPprimeY_mod6 = QPprimeY%6;

  curr_mb_attr->transform_bypass = bypass;

 // if (entropy_coding_mode_flag)
    fills_in_nC_cache_cabac(curr_mb_attr, curr_is_bot, mb_field_decoding_flag, PicWidthInMbs, luma_nC_cache,
        chroma_nC_cache, cfidc, mb_type, constrained_intra_pred_flag, data_partitioning_slice_flag, MbaffFrameFlag);
 // else
//    fills_in_nC_cache(curr_mb_attr, curr_is_bot, mb_field_decoding_flag, PicWidthInMbs, luma_nC_cache, chroma_nC_cache,
      //  cfidc, mb_type, constrained_intra_pred_flag, data_partitioning_slice_flag, MbaffFrameFlag);


  // Intra16x16
  if( MbPartPredMode0 == Intra_16x16 )
  {
    scan_table =  mb_field_decoding_flag ? inverse_4x4field_scan : inverse_4x4zigzag_scan;
    int TotalCoeff;
    int i;
    int16_t* data = mb_data;
    int DCLevelScale = LevelScale4x4[0][QPprimeY_mod6][0];
    int two_pow_qp_div6 = 1<<QPprimeY_div6;

    TotalCoeff = residual_block(cabac_ctx, bs, 0, mb_data, 16, LUMA_DC_LEVEL, luma_nC_cache, cfidc, scan_table, 0, 0, bypass,
        mb_field_decoding_flag, entropy_coding_mode_flag);

    inverse_transform(IHDM4x4, TotalCoeff, bypass, 0, data, 0, 0);
    for( i=0; i<16; i++, data++)
      *data = (int16_t) ((*data * DCLevelScale * two_pow_qp_div6 + (1<<5))>>6);
    curr_mb_attr->luma_dc_coeffs_non_null = TotalCoeff!=0;

    for( i8x8 = 0; i8x8 < 4; i8x8++ ) /* each luma 8x8 block */
    {
      if( CodedBlockPatternLuma & ( 1 << i8x8 ) )
      {
        for( i4x4 = 0; i4x4 < 4; i4x4++ )
        { /* each 4x4 sub-block of block */
          residual_block(cabac_ctx, bs, i8x8 * 4 + i4x4, mb_data + 16 - 1 + scan4x4[i8x8 * 4 + i4x4] * 15, 15, LUMA_AC_LEVEL,
              luma_nC_cache, cfidc, scan_table + 1, LevelScale4x4[0][QPprimeY_mod6], 1 << QPprimeY_div6, bypass,
              mb_field_decoding_flag, entropy_coding_mode_flag);
        }
      }
      else
      {
        luma_nC_cache[scan4x4toraster5x5[i8x8 * 4 + 0]] = 0;
        luma_nC_cache[scan4x4toraster5x5[i8x8 * 4 + 1]] = 0;
        luma_nC_cache[scan4x4toraster5x5[i8x8 * 4 + 2]] = 0;
        luma_nC_cache[scan4x4toraster5x5[i8x8 * 4 + 3]] = 0;
      }
    }
  }


  // CHROMA residual
  if (cfidc != 0)
  {

    unsigned int intra_inter_mb = (mb_type > SI) * 3;
    int QPprimeC_div6[2];
    int QPprimeC_mod6[2];
    unsigned int NumC8x8, iCbCr;
    const uint8_t* nC_cache_scan_table = cfidc == 3 ? scan4x4toraster5x5 : raster2x4toraster5x5;
    const uint8_t* chroma_scan_table = cfidc == 3 ? scan4x4 : scan2x4;
    unsigned int TotalCoeff[2] = {0, 0};
    int i, DCLevelScale, two_pow_qp_div6;

    mb_data += 256; // go to the chroma location

    scan_table =  mb_field_decoding_flag ? inverse_4x4field_scan : inverse_4x4zigzag_scan;

    // Deals with DC coeffs
    if (cfidc == 1)
    {
      NumC8x8 = 1;
      for (iCbCr = 0; iCbCr < 2; iCbCr++)
      {
        QPprimeC_div6[iCbCr] = QPprimeC[iCbCr] / 6;
        QPprimeC_mod6[iCbCr] = QPprimeC[iCbCr] % 6;
        if (CodedBlockPatternChroma & 3)  /* chroma DC residual present */
        {
          int16_t* data = mb_data+64*iCbCr;

          TotalCoeff[iCbCr] = residual_block(cabac_ctx, bs, 0, data, 4, CHROMA_DC_LEVEL, chroma_nC_cache[iCbCr], 1,
              chroma_inverse_dc_scan1, 0, 0, bypass, mb_field_decoding_flag, entropy_coding_mode_flag);
          inverse_transform(IHDM2x2, TotalCoeff[iCbCr], bypass, 0, data, 0, 0);
          DCLevelScale = LevelScale4x4[intra_inter_mb+iCbCr+1][QPprimeC_mod6[iCbCr]][0];
          two_pow_qp_div6 = 1<<(QPprimeC_div6[iCbCr]);
          for( i=0; i<4; i++, data++)
            *data = (int16_t) ((*data * DCLevelScale * two_pow_qp_div6)>>5);
        }
      }
    }

    curr_mb_attr->chroma_dc_coeffs_non_null = (TotalCoeff[0] != 0) + ((TotalCoeff[1] != 0)<<1);

    // Deals with AC coeffs
    for (iCbCr = 0; iCbCr < 2; iCbCr++)
      for (i8x8 = 0; i8x8 < NumC8x8; i8x8++)
        for (i4x4 = 0; i4x4 < 4; i4x4++)
          if (CodedBlockPatternChroma & 2)
          {
            /* chroma AC residual present */
            residual_block(cabac_ctx, bs, i8x8 * 4 + i4x4, mb_data + NumC8x8 * 64 * iCbCr + NumC8x8 * 4 - 1
                + chroma_scan_table[i8x8 * 4 + i4x4] * 15, 15, CHROMA_AC_LEVEL, chroma_nC_cache[iCbCr], cfidc,
                scan_table + 1, LevelScale4x4[intra_inter_mb + iCbCr + 1][QPprimeC_mod6[iCbCr]], 1
                    << (QPprimeC_div6[iCbCr]), bypass, mb_field_decoding_flag, entropy_coding_mode_flag);
          }
          else
          {
            chroma_nC_cache[iCbCr][nC_cache_scan_table[i8x8 * 4 + i4x4]] = 0;
          }
  }

  writes_back_nC_cache(curr_mb_attr, luma_nC_cache, chroma_nC_cache, cfidc);
}


static should_inline MB_TYPE decode_mb_type(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag,
    MbAttrib* curr, slice_type_t slice_type_modulo5)
{
    return cabac_decode_mb_type(cabac_ctx, bs, curr, slice_type_modulo5);
}

static should_inline unsigned int decode_transform_size_8x8_flag(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag,
    MbAttrib* curr_mb_attr)
{
    return cabac_decode_transform_size_8x8_flag(cabac_ctx, bs, curr_mb_attr);
}

static should_inline void decode_coded_block_pattern(unsigned int* CodedBlockPatternLuma, unsigned int* CodedBlockPatternChroma,
    CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag, PREDICTION_MODE prediction_mode,
    unsigned int cfidc, MbAttrib* curr)
{
    cabac_decode_coded_block_pattern(CodedBlockPatternLuma, CodedBlockPatternChroma, cabac_ctx, bs, curr, cfidc);

}

static should_inline int decode_mb_qp_delta(CABACContext* cabac_ctx, BitStreamContext* bs, unsigned int entropy_coding_mode_flag, int prevMbAddr_mb_qp_delta)
{
    return cabac_decode_mb_qp_delta(cabac_ctx, bs, prevMbAddr_mb_qp_delta);
}

static should_inline int derive_QPY(int QPY_PREV, int mb_qp_delta, int QpBdOffsetY)
{
  return ( ( QPY_PREV + mb_qp_delta + 52 + 2 * QpBdOffsetY ) % ( 52 + QpBdOffsetY ) ) - QpBdOffsetY;
}

// TODO readd the CABAC mode specific code (following the unsigned int entropy_coding_mode_flag, renabling)
static should_inline RetCode decode_macroblock_layer(CABACContext* cabac_ctx, BitStreamContext* bs,
                                              MbAttrib* curr_mb_attr, unsigned int mb_row, unsigned int mb_col, unsigned int mb_idx, int PicWidthInMbs,
                                              unsigned int slice_type_modulo5, unsigned int cfidc,
                                              unsigned int direct_8x8_inference_flag, unsigned int transform_8x8_mode_flag, unsigned int entropy_coding_mode_flag,
                                              unsigned int MbaffFrameFlag, unsigned int mb_field_decoding_flag,
                                              uint8_t num_ref_idx_active[2],
                                              int *QPY, int QPC[2], int BitDepthY, int BitDepthC, int QpBdOffsetY, int QpBdOffsetC,
                                              int chroma_qp_index_offset, int second_chroma_qp_index_offset, unsigned int qpprime_y_zero_transform_bypass_flag,
                                              uint16_t LevelScale4x4[6][6][16], uint16_t LevelScale8x8[2][6][64],
                                              unsigned int constrained_intra_pred_flag, nal_unit_type_t data_partitioning_slice_flag,
                                              int16_t* mb_data, unsigned int mb_data_size, int* prevMbAddr_mb_qp_delta)
{
  MB_TYPE mb_type; //uint8_t
  int transform_size_8x8_flag = 0; // uint8_t
  int noSubMbPartSizeLessThan8x8Flag; // uint8_t
  unsigned int CodedBlockPatternLuma; // uint8_t
  unsigned int CodedBlockPatternChroma = 0; // uint8_t
  PREDICTION_MODE MbPartPredMode0;
//  uint8_t* sub_mb_type;

  mb_type = decode_mb_type(cabac_ctx, bs, entropy_coding_mode_flag, curr_mb_attr, slice_type_modulo5); // 2  ue(v) | ae(v)

  // FIXME DEBUG
  //printf("MB %d - ", mb_idx);


  curr_mb_attr->mb_type = mb_type;

  {
    noSubMbPartSizeLessThan8x8Flag = 1;
    MbPartPredMode0 = MbPartPredMode(mb_type, 0, 0, 0); // for the next test we know mb_type != I_NxN, so this is correct else

    {
      mb_pred(cabac_ctx, bs, entropy_coding_mode_flag, mb_type, MbPartPredMode0, transform_size_8x8_flag,
              cfidc, MbaffFrameFlag, mb_field_decoding_flag, constrained_intra_pred_flag,
              num_ref_idx_active,
              curr_mb_attr, mb_row&1, PicWidthInMbs);
    }
    // MbPartPredMode0 == Intra_16x16
    {
      // derive CodedBlockPatternLuma and CodedBlockPatternChroma according to table 7.11 (cf ITU-T Rec. H.264 (03/2005) 87)
      curr_mb_attr->CodedBlockPatternLuma = CodedBlockPatternLuma = CodedBlockPatternLuma_MbPartHeight_array[mb_type];
      curr_mb_attr->CodedBlockPatternChroma = CodedBlockPatternChroma = cfidc == 0 ? 0 : CodedBlockPatternChroma_MbPartWidth_array[mb_type];
    }
    if( CodedBlockPatternLuma > 0 || CodedBlockPatternChroma > 0 || MbPartPredMode0 == Intra_16x16 )
    {
      int mb_qp_delta; // int8_t
      int QPprimeY;
      int QPprimeC[2];
      int qPi;

      int16_t* curr_mb_data = mb_data;

      // Set all coeffs to zero. Needed because only non null coeffs are written in the subsequent code...
      memset(curr_mb_data, 0, mb_data_size*sizeof(*curr_mb_data)); // could move this to the picture data allocation ? that would make one call for the whole picture.
      //Would it make big overhead (for instance for skip mb, the data are not necessary)

      *prevMbAddr_mb_qp_delta = mb_qp_delta = decode_mb_qp_delta(cabac_ctx, bs, entropy_coding_mode_flag, *prevMbAddr_mb_qp_delta); // 2  se(v) | ae(v)

      *QPY = derive_QPY(*QPY, mb_qp_delta, QpBdOffsetY);
      QPprimeY = *QPY + QpBdOffsetY;

      qPi = im_clip(-QpBdOffsetC, 51, *QPY+chroma_qp_index_offset)-30;
      QPC[0] = (qPi<0) ? qPi+30 : qPi_to_QPC[qPi]; // qPi if <30 or conv[qPi] if >=30
      qPi = im_clip(-QpBdOffsetC, 51, *QPY+second_chroma_qp_index_offset)-30;
      QPC[1] = (qPi<0) ? qPi+30 : qPi_to_QPC[qPi]; // qPi if <30 or conv[qPi] if >=30
      QPprimeC[0] = QPC[0] + QpBdOffsetC;
      QPprimeC[1] = QPC[1] + QpBdOffsetC;

      residual(cabac_ctx, bs, MbPartPredMode0, curr_mb_attr, mb_type,
                CodedBlockPatternLuma, CodedBlockPatternChroma, mb_row&1,
                entropy_coding_mode_flag, transform_size_8x8_flag, cfidc,
                QPprimeY, QPprimeC, curr_mb_data, mb_field_decoding_flag, PicWidthInMbs,
                LevelScale4x4, LevelScale8x8, qpprime_y_zero_transform_bypass_flag,
                constrained_intra_pred_flag, data_partitioning_slice_flag, MbaffFrameFlag
              );
    }


    curr_mb_attr->QPY = *QPY;
    curr_mb_attr->QPC[0] = QPC[0];
    curr_mb_attr->QPC[1] = QPC[1];

  }
  curr_mb_attr->transform_size_8x8_flag = transform_size_8x8_flag;
  return RET_SUCCESS;
}


// Derive neighbors mb flags (for MbaffFrameFlag==1) and neighbor availability
static should_inline void derive_neighbors_flags(MbAttrib* curr, unsigned int slice_num, unsigned int MbaffFrameFlag, unsigned int PicWidthInMbs,
    unsigned int curr_is_field, unsigned mb_col, unsigned mb_row)
{
  if (!MbaffFrameFlag)
  {
    curr->left_mb_is_available = mb_col > 0 && (curr-1)->slice_num == slice_num;
    curr->up_mb_is_available = mb_row > 0 && (curr-PicWidthInMbs)->slice_num == slice_num;
    curr->upleft_mb_is_available = mb_row > 0 && mb_col > 0 && (curr-1-PicWidthInMbs)->slice_num == slice_num;
    curr->upright_mb_is_available = mb_row > 0 &&  mb_col < (PicWidthInMbs-1) && (curr+1-PicWidthInMbs)->slice_num == slice_num;
  }
  else
  {
    // left mb
    MbAttrib* left = curr - 1;
    curr->left_mb_is_available = mb_col > 0 && left->slice_num == slice_num;
    if (curr->left_mb_is_available) // TODO: not doing this test may provoke read outside the allocated block, to check how this work
      curr->left_mb_is_field = left->mb_field_decoding_flag;

    // up mb
    unsigned int is_bot_mb = mb_row & 1;
    MbAttrib* up = curr - (PicWidthInMbs << (is_bot_mb && curr_is_field));
    curr->up_mb_is_available = mb_row > curr_is_field  && up->slice_num == slice_num;
    if (curr->up_mb_is_available) // TODO: not doing this test may provoke read outside the allocated block, to check how this work
      curr->up_mb_is_field = up->mb_field_decoding_flag;

    // up left mb
    MbAttrib* upleft = curr -1 -(PicWidthInMbs << (is_bot_mb && curr_is_field));
    curr->upleft_mb_is_available = mb_row > curr_is_field && mb_col > 0 && upleft->slice_num == slice_num;
    if (curr->upleft_mb_is_available) // TODO: not doing this test may provoke read outside the allocated block, to check how this work
      curr->upleft_mb_is_field = upleft->mb_field_decoding_flag;

    // up right mb
    MbAttrib* upright = curr +1 -(PicWidthInMbs << (is_bot_mb && curr_is_field));
    curr->upright_mb_is_available = mb_row > 1 && mb_col < (PicWidthInMbs-1) && upright->slice_num == slice_num;
    if (curr->upright_mb_is_available) // TODO: not doing this test may provoke read outside the allocated block, to check how this work
      curr->upright_mb_is_field = upright->mb_field_decoding_flag;

  }
}


#define CALC_MB_COORDS(currmb, mbidx, mbrow, mbcol, mbflag, picwidth) \
if (mbflag)                                                           \
{                                                                     \
  int top = (currmb)%2;                                               \
  int idx = (currmb)/2;                                               \
  (mbrow) = ((idx) / (picwidth))*2 + top;                             \
  (mbcol) = (idx) % (picwidth);                                       \
  (mbidx) = (mbrow) * (picwidth) + (mbcol);                           \
}                                                                     \
else                                                                  \
{                                                                     \
  (mbrow) = (currmb) / (picwidth);                                    \
  (mbcol) = (currmb) % (picwidth);                                    \
  (mbidx) = (currmb);                                                 \
}

#define CALC_MB_IDX(currmb, mbidx, mbflag, picwidth)                  \
if (mbflag)                                                           \
{                                                                     \
  int top = (currmb)%2;                                               \
  int idx = (currmb)/2;                                               \
  int mbrow = ( (idx) / (picwidth) )*2 + top;                         \
  int mbcol = (idx ) % (picwidth);                                    \
  (mbidx) = (mbrow) * (picwidth) + (mbcol);                           \
}                                                                     \
else                                                                  \
{                                                                     \
  (mbidx) = (currmb);                                                 \
}


// decode a slice with CAVLC method. it means that this function should be called when entropy_coding_mode_flag==0
// TODO re-add the CABAC specific code (unsigned int entropy_coding_mode_flag,)
static should_inline RetCode decode_slice_data_internal(PictureDecoderData* pdd, nal_unit_t* nalu, slice_header_t* sh,
    seq_parameter_set_rbsp_t* sps, pic_parameter_set_rbsp_t* pps,
    unsigned int entropy_coding_mode_flag, int cfidc, int BitDepthY, int BitDepthC, int QpBdOffsetY, int QpBdOffsetC,
    unsigned int MbaffFrameFlag)
{

  //slice_data_t* sd;
  BitStreamContext* bs = &nalu->bs;


//  int i;
  int16_t datarray[384];
  int mb_nb = 0;
  int i;

  unsigned int PicWidthInMbs = sps->PicWidthInMbs;
  int CurrMbAddr = sh->first_mb_in_slice * (1 + MbaffFrameFlag);
  int moreDataFlag = 1;
  int prevMbSkipped = 0;
//  int mb_skip_run;
  unsigned int mb_skip_flag = 0;
  uint8_t mb_field_decoding_flag = sh->field_pic_flag;
  int mb_row, mb_col, mb_idx; // Those numbers are computed so that it is real raster scan even when MbaffFrameFlag is set.
  Picture* pic = pdd->pic;
  unsigned int slice_num = pic->slice_num;

  //MbAttrib* mb_attr = pic->field_data.mb_attr;
  unsigned int mb_data_size = 256+2*MbSizeC[cfidc];
  int QPY = sh->SliceQPY;
  int QPC[2];
  CABACContext* cabac_ctx = &pdd->cabac_ctx;
  slice_type_t slice_type_modulo5 = sh->slice_type_modulo5;
  int prevMbAddr_mb_qp_delta = 0;
  unsigned int data_partitioning_slice_flag = is_data_partitioning_slice(nalu->nal_unit_type);



  {     // not really sure this is necessary ?! But safer anyway, in case first mb of the slice is not Intra16x16 and cbp==0, QPC will still have a default value...
    int qPi;
    qPi = im_clip(-QpBdOffsetC, 51, QPY + pps->chroma_qp_index_offset)-30;
    QPC[0] = (qPi<0) ? qPi+30 : qPi_to_QPC[qPi]; // qPi if <30 or conv[qPi] if >=30
    qPi = im_clip(-QpBdOffsetC, 51, QPY + pps->second_chroma_qp_index_offset)-30;
    QPC[1] = (qPi<0) ? qPi+30 : qPi_to_QPC[qPi]; // qPi if <30 or conv[qPi] if >=30
  }


  if (entropy_coding_mode_flag)
  {
    // Align on 8-bits boundary
    int alignlen = (8 - (bs->index & 0x7)) & 0x7;
#ifndef NDEBUG
    int cabac_alignment_one_bits = bs_read_un(bs, alignlen);
    LUD_DEBUG_ASSERT(cabac_alignment_one_bits == (1<<alignlen)-1);
#else // NDEBUG
    bs_skip_n(bs, alignlen);
#endif

    // Initialize cabac decoding context and engine
    init_cabac_context_variables(cabac_ctx, QPY, sh->cabac_init_idc, slice_type_modulo5);
    init_cabac_decoding_engine(cabac_ctx, bs);

  }
   int u;
   u = 0;
   FM_FILE* outfile;
   outfile = fm_open("rgbs.dat",writemode);
//   MbAttrib* curr_mb_attr;
   MbAttrib* curr_mb_attr2;


  int y,t;
  printf("\n\n");
  for (i = 0; i<8160; i++)
  {

    y = floor(i/120);
    t = i - 120*y;

    CALC_MB_COORDS(CurrMbAddr, mb_idx, mb_row, mb_col, MbaffFrameFlag, PicWidthInMbs);
    LUD_DEBUG_ASSERT(mb_idx < sh->PicSizeInMbs);
    //curr_mb_attr2 = mb_attr + i;
     curr_mb_attr2 = test_array + t + 120;
    if(t==0 && i!=0)
    {
          resetarray();
    }

    // Always an I or SI frame
    {
      derive_neighbors_flags(curr_mb_attr2, slice_num, MbaffFrameFlag, PicWidthInMbs, mb_field_decoding_flag, mb_col, mb_row);
      if (entropy_coding_mode_flag)
        cabac_derive_mb_neighbors(cabac_ctx, curr_mb_attr2, mb_row&1, mb_field_decoding_flag, PicWidthInMbs, MbaffFrameFlag);
    }

    if (moreDataFlag)
    {


      curr_mb_attr2->mb_skip_flag = 0;
      curr_mb_attr2->mb_field_decoding_flag = mb_field_decoding_flag;
      curr_mb_attr2->slice_num = slice_num;
      //mb_data = data + mb_idx * mb_data_size;

      decode_macroblock_layer(cabac_ctx, bs,
                              curr_mb_attr2, mb_row, mb_col, mb_idx, PicWidthInMbs,
                              slice_type_modulo5, cfidc,
                              sps->direct_8x8_inference_flag, pps->transform_8x8_mode_flag, entropy_coding_mode_flag,
                              MbaffFrameFlag, mb_field_decoding_flag,
                              sh->num_ref_idx_active,
                              &QPY, QPC, BitDepthY, BitDepthC, QpBdOffsetY, QpBdOffsetC,
                              pps->chroma_qp_index_offset, pps->second_chroma_qp_index_offset, sps->qpprime_y_zero_transform_bypass_flag,
                              pps->LevelScale4x4, pps->LevelScale8x8,
                              pps->constrained_intra_pred_flag, data_partitioning_slice_flag,
                              datarray, mb_data_size, &prevMbAddr_mb_qp_delta);



     decode_image(pdd, pic, 0, datarray, outfile,CurrMbAddr,curr_mb_attr2,topy,topcb,topcr,ysamp,cbsamp,crsamp);


      //TODO add motion vector call here !


      mb_nb++;
    }

    //if (!entropy_coding_mode_flag)
    //  moreDataFlag = more_rbsp_data(nalu, bs);
    //else
    {
      if (slice_type_modulo5 != I_SLICE && slice_type_modulo5 != SI_SLICE)
        prevMbSkipped = mb_skip_flag;
      if (MbaffFrameFlag && CurrMbAddr % 2 == 0)
        moreDataFlag = 1;
      else
      {
        unsigned int end_of_slice_flag = cabac_decode_end_of_slice_flag(cabac_ctx, bs); // 2 ae(v)
        moreDataFlag = !end_of_slice_flag;
      }
    }

    CurrMbAddr = NextMbAddress(sh->MbToSliceGroupMap, CurrMbAddr, sh->PicSizeInMbs, pps->num_slice_groups_minus1);
  }
  fm_close(outfile);


  pic->field_sh[slice_num]->mb_nb = mb_nb;
  return RET_SUCCESS;
}


// This function duplicate the code (static inline functions) with hardcoded flags in the hope that the compiler optimize the inlined code...
RetCode decode_slice_data(PictureDecoderData* pdd, nal_unit_t* nalu, slice_header_t* sh,
    seq_parameter_set_rbsp_t* sps, pic_parameter_set_rbsp_t* pps, unsigned int entropy_coding_mode_flag)
{
//  unsigned int cfidc = sps->chroma_format_idc;
//  unsigned int BitDepthY = sps->BitDepthY;
//  unsigned int BitDepthC = sps->BitDepthC;
//  unsigned int QpBdOffsetY = sps->QpBdOffsetY;
//  unsigned int QpBdOffsetC = sps->QpBdOffsetC;
//  unsigned int MbaffFrameFlag = sh->MbaffFrameFlag;


    //if (cfidc == 1 && BitDepthY == 8 && BitDepthC == 8 && QpBdOffsetY==0 && QpBdOffsetC == 0 && MbaffFrameFlag == 0)
      return decode_slice_data_internal(pdd, nalu, sh, sps, pps, 1, 1, 8, 8, 0, 0, 0);
   // else
     // return decode_slice_data_internal(pdd, nalu, sh, sps, pps, 1, sps->chroma_format_idc,
      //                                sps->BitDepthY, sps->BitDepthC, sps->QpBdOffsetY, sps->QpBdOffsetC, MbaffFrameFlag);


}
