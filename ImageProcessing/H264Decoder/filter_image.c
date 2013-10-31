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




#include "common.h"
#include "decode.h"
#include "defaulttables.h"
#include "intmath.h"
#include "decode_slice.h"

#define IS_INTRA_MB(m)        ((m)->mb_type <= SI)
#define IS_INTER_MB(m)        ((m)->mb_type >  SI)
#define IS_SI_SP_SLICE_MB(m)  (sh[(m)->slice_num]->slice_type_modulo5 >= SP_SLICE)




static const uint8_t indexA_to_alpha[52] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 5, 6, 7, 8, 9, 10, 12, 13,
  15, 17, 20, 22, 25, 28, 32, 36, 40, 45, 50, 56, 63, 71, 80, 90, 101, 113, 127, 144, 162, 182, 203, 226, 255, 255
};


static const uint8_t indexB_to_beta[52] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,
  6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18
};


static const uint8_t bS_indexA_to_tc0[3][52] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8, 9, 10, 11, 13 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 7, 8, 8, 10, 11, 12, 13, 15, 17 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8, 9, 10, 11, 13, 14, 16, 18, 20, 23, 25 }
};



#define comp(a, b) (refPic_p[a] == refPic_q[b] && field_p[a] == field_q[b])
// i and j are set to 0 or 1 (num of the list used). They are derived so that RefPic(i) = RefPic(j)


// test if Inter mb satisfy condition on page 189 of H264.2005/03 standard



static should_inline void fills_in_bS_col(uint8_t bSr[4], unsigned int either_or_both_is_intra, unsigned int either_or_both_is_SI_SP,
                                   uint8_t* left_coeffs, uint8_t* curr_coeffs, int8_t* refIdxLX_p, int8_t* refIdxLX_q,
                                   RefListEntry* RefPicList_p[2], RefListEntry* RefPicList_q[2],
                                   int16_t* mvL_p, int16_t* mvL_q, unsigned int mb_field_decoding_flag, unsigned int MbaffFrameFlag)
{
  int i;

  if (either_or_both_is_intra || either_or_both_is_SI_SP)
  {
    *(uint32_t*)bSr = 0x03030303; // could be unaligned 32bits write ?
  }
  else
  {
    for (i=0; i<4; i++)
    {
      if (curr_coeffs[i*4] || left_coeffs[i*4])
      {
        bSr[i] = 2;
      }
//      else if (inter_bS_test(0, refIdxLX_p+i*4, refIdxLX_q+i*4, RefPicList_p, RefPicList_q, mvL_p+i*8, mvL_q+i*8, MbaffFrameFlag, mb_field_decoding_flag))
      //{
      //  bSr[i] = 1;
     // }
      else
      {
        bSr[i] = 0;
      }
    }
  }
}

// this function is used when curr_is_field ^ left_is_field == 0 == mixedModeEdgeFlag
static should_inline void fills_in_bS_col_mbaff2(uint8_t bSr[8], unsigned int either_or_both_is_intra, unsigned int either_or_both_is_SI_SP,
                                   uint8_t* left_coeffs, uint8_t* curr_coeffs, int8_t* refIdxLX_p, int8_t* refIdxLX_q,
                                   RefListEntry* RefPicList_p[2], RefListEntry* RefPicList_q[2],
                                   int16_t* mvL_p, int16_t* mvL_q, unsigned int mb_field_decoding_flag)
{
  int i;

  if (either_or_both_is_intra || either_or_both_is_SI_SP)
  {
    *(uint64_t*)bSr = 0x0303030303030303ULL; // could be unaligned 32bits write ?
  }
  else
  {
    for (i=0; i<4; i++)
    {
      if (curr_coeffs[i*4] || left_coeffs[i*4])
      {
        ((uint16_t*)bSr)[i] = 0x0202;
      }
//      else if (inter_bS_test(0, refIdxLX_p+i*4, refIdxLX_q+i*4, RefPicList_p, RefPicList_q, mvL_p+i*8, mvL_q+i*8, 1, mb_field_decoding_flag))
    //  {
     //   ((uint16_t*)bSr)[i] = 0x0101;
     // }
      else
      {
        ((uint16_t*)bSr)[i] = 0;
      }
    }
  }
}

// this function is used when curr_is_field && !left_is_field => mixedModeEdgeFlag = 1
static should_inline void fills_in_bS_col_mbaff0(uint8_t bSr[4],
                                         uint8_t* left_coeffs, uint8_t* curr_coeffs, int8_t* refIdxLX_p, int8_t* refIdxLX_q,
                                         RefListEntry* RefPicList_p[2], RefListEntry* RefPicList_q[2],
                                         int16_t* mvL_p, int16_t* mvL_q)
{
  int i;

  for (i=0; i<4; i++)
  {
    if (curr_coeffs[(i>>1)*4] || left_coeffs[i*4])
    {
      bSr[i] = 2;
    }
//    else if (inter_bS_test(1, refIdxLX_p+i*4, refIdxLX_q+(i>>1)*4, RefPicList_p, RefPicList_q, mvL_p+i*8, mvL_q+(i>>1)*8, 1, 0))
   // {
   //   bSr[i] = 1;
   // }
    else
    {
      bSr[i] = 0;
    }
  }
}

// this function is used when !curr_is_field && left_is_field => mixedModeEdgeFlag = 1
// left_offset is equal to 0 (left top mb) or 1 (left bottom mb)
static should_inline void fills_in_bS_col_mbaff1(uint8_t bSr[8], unsigned int left_offset,
                                          uint8_t* left_coeffs, uint8_t* curr_coeffs, int8_t* refIdxLX_p, int8_t* refIdxLX_q,
                                          RefListEntry* RefPicList_p[2], RefListEntry* RefPicList_q[2],
                                          int16_t* mvL_p, int16_t* mvL_q)
{
  int i;

  for (i=0; i<4; i++)
  {
    if (curr_coeffs[i*4] || left_coeffs[(i>>1)*4])
    {
      bSr[(i<<1)+left_offset] = 2;
    }
//    else if (inter_bS_test(1, refIdxLX_p+(i>>1)*4, refIdxLX_q+i*4, RefPicList_p, RefPicList_q, mvL_p+(i>>1)*8, mvL_q+i*8, 1, 0))
  //  {
  //    bSr[(i<<1)+left_offset] = 1;
   // }
    else
    {
      bSr[(i<<1)+left_offset] = 0;
    }
  }
}



// vertical edges:   bS[0][col][row]
// horizontal edges: bS[1][row][col]
// bS array should be aligned on a 4 bytes bondary
static should_inline void fills_in_bS_nonmbaff(uint8_t bS[2][4][4], int8_t qPav[3][3], MbAttrib* curr_mb_attr, unsigned int mb_row, unsigned int mb_col,
                                        int PicWidthInMbs, unsigned int disable_deblocking_filter_idc, slice_header_t** sh,
                                        unsigned int slice_num, RefListEntry* RefPicList[2], unsigned int field_pic_flag,
                                        unsigned int transform_size_8x8_flag, unsigned int cfidc)
{
  uint8_t* bSr;
  unsigned int either_or_both_is_intra;
  unsigned int either_or_both_is_SI_SP;
  MbAttrib* upper_mb_attr;
  MbAttrib* left_mb_attr;

  qPav[0][0] = curr_mb_attr->QPY;
  qPav[0][1] = curr_mb_attr->QPC[0];
  qPav[0][2] = curr_mb_attr->QPC[1];


  // fills in horizontal edges
  // 1st row
  bSr = bS[1][0];
  upper_mb_attr = curr_mb_attr-PicWidthInMbs;
  if (mb_row > 0 && (!disable_deblocking_filter_idc || upper_mb_attr->slice_num == slice_num))
  {
    qPav[2][0] = (upper_mb_attr->QPY + curr_mb_attr->QPY + 1) >> 1; // Y
    qPav[2][1] = (upper_mb_attr->QPC[0] + curr_mb_attr->QPC[0] + 1) >> 1; // Cb
    qPav[2][2] = (upper_mb_attr->QPC[1] + curr_mb_attr->QPC[1] + 1) >> 1; // Cr
    either_or_both_is_intra = IS_INTRA_MB(curr_mb_attr) || IS_INTRA_MB(upper_mb_attr);
    either_or_both_is_SI_SP = IS_SI_SP_SLICE_MB(curr_mb_attr) || IS_SI_SP_SLICE_MB(upper_mb_attr);
    if ( (!field_pic_flag && (either_or_both_is_intra || either_or_both_is_SI_SP )))
    {
      *(uint32_t*)bSr = 0x04040404; // could be unaligned 32bits write ?
    }

  }
  else // no filtering of 1st row
    *(uint32_t*)bSr = 0; // could be unaligned 32bits write ?

  // 2nd row
  bSr += 4;
  either_or_both_is_intra = IS_INTRA_MB(curr_mb_attr);
  either_or_both_is_SI_SP = IS_SI_SP_SLICE_MB(curr_mb_attr);

  // 3rd row
  bSr += 4;

  // 4th row
  bSr += 4;


  // fills in vertical edges
  left_mb_attr = curr_mb_attr-1;
  bSr = bS[0][0];
  // 1st col
  if (mb_col>0 && (!disable_deblocking_filter_idc || left_mb_attr->slice_num == slice_num))
  {
    qPav[1][0] = (left_mb_attr->QPY + curr_mb_attr->QPY + 1) >> 1;
    qPav[1][1] = (left_mb_attr->QPC[0] + curr_mb_attr->QPC[0] + 1) >> 1;
    qPav[1][2] = (left_mb_attr->QPC[1] + curr_mb_attr->QPC[1] + 1) >> 1;
    either_or_both_is_intra = IS_INTRA_MB(curr_mb_attr) || IS_INTRA_MB(left_mb_attr);
    either_or_both_is_SI_SP = IS_SI_SP_SLICE_MB(curr_mb_attr) || IS_SI_SP_SLICE_MB(left_mb_attr);
    if ( (either_or_both_is_intra || either_or_both_is_SI_SP) )
    {
      *(uint32_t*)bSr = 0x04040404; // could be unaligned 32bits write ?
    }
  }
  else // no filtering for the first col
  {
    *(uint32_t*)bSr = 0; // could be unaligned 32bits write ?
  }

  // 2nd col
  bSr += 4;
  either_or_both_is_intra = IS_INTRA_MB(curr_mb_attr);
  either_or_both_is_SI_SP = IS_SI_SP_SLICE_MB(curr_mb_attr);
  if (!transform_size_8x8_flag || cfidc > 2)
  {
    fills_in_bS_col(bSr, either_or_both_is_intra, either_or_both_is_SI_SP,
                    curr_mb_attr->luma_total_coeff, curr_mb_attr->luma_total_coeff+1,
                    &curr_mb_attr->pred.inter.RefIdxL[0][0], &curr_mb_attr->pred.inter.RefIdxL[0][1],
                    RefPicList, RefPicList,
                    curr_mb_attr->pred.inter.MvL[0][0], curr_mb_attr->pred.inter.MvL[0][1],
                    field_pic_flag, 0);
  }

  // 3rd col
  bSr += 4;
  fills_in_bS_col(bSr, either_or_both_is_intra, either_or_both_is_SI_SP,
                  curr_mb_attr->luma_total_coeff+1, curr_mb_attr->luma_total_coeff+2,
                  &curr_mb_attr->pred.inter.RefIdxL[0][1], &curr_mb_attr->pred.inter.RefIdxL[0][2],
                  RefPicList, RefPicList,
                  curr_mb_attr->pred.inter.MvL[0][1], curr_mb_attr->pred.inter.MvL[0][2],
                  field_pic_flag, 0);

  // 4th col
  bSr += 4;
  if (!transform_size_8x8_flag || cfidc > 2)
  {
    fills_in_bS_col(bSr, either_or_both_is_intra, either_or_both_is_SI_SP,
                    curr_mb_attr->luma_total_coeff+2, curr_mb_attr->luma_total_coeff+3,
                    &curr_mb_attr->pred.inter.RefIdxL[0][2], &curr_mb_attr->pred.inter.RefIdxL[0][3],
                    RefPicList, RefPicList,
                    curr_mb_attr->pred.inter.MvL[0][2], curr_mb_attr->pred.inter.MvL[0][3],
                    field_pic_flag, 0);
  }
}






#define LOAD_8_SAMPLES(comp, stride)        \
const int q0 = (comp)[0];                   \
const int q1 = (comp)[(stride)];            \
const int q2 = (comp)[(stride)*2];          \
const int q3 = (comp)[(stride)*3];          \
const int p0 = (comp)[-(signed)(stride)];   \
const int p1 = (comp)[-(signed)(stride)*2]; \
const int p2 = (comp)[-(signed)(stride)*3]; \
const int p3 = (comp)[-(signed)(stride)*4];

#define LOAD_6F_SAMPLES(comp, stride)       \
const int q0 = (comp)[0];                   \
const int q1 = (comp)[(stride)];            \
const int q2 = (comp)[(stride)*2];          \
const int p0 = (comp)[-(signed)(stride)];   \
const int p1 = (comp)[-(signed)(stride)*2]; \
const int p2 = (comp)[-(signed)(stride)*3];


#define LOAD_4F_SAMPLES(comp, stride)       \
const int q0 = (comp)[0];                   \
const int q1 = (comp)[(stride)];            \
const int p0 = (comp)[-(signed)(stride)];   \
const int p1 = (comp)[-(signed)(stride)*2];

#define LOAD_2M_SAMPLES(comp, stride)       \
const int q2 = (comp)[(stride)*2];          \
const int p2 = (comp)[-(signed)(stride)*3];

#define LOAD_4L_SAMPLES(comp, stride)       \
const int q2 = (comp)[(stride)*2];          \
const int q3 = (comp)[(stride)*3];          \
const int p2 = (comp)[-(signed)(stride)*3]; \
const int p3 = (comp)[-(signed)(stride)*4];

int cubu_debug = 0; //cubu

static should_inline void filtering_for_bS_less_than_4(const int p0, const int p1, const int p2, const int q0, const int q1, const int q2,
                                                const int tc0, const int beta, unsigned int is_luma, pixel_t* out, int stride, int max_value)
{
  int tc;
  int delta;

  unsigned int ap_flag = im_abs(p2-p0) < beta;
  unsigned int aq_flag = im_abs(q2-q0) < beta;


  if (is_luma)
    tc = tc0 + ap_flag + aq_flag;
  else
    tc = tc0 + 1;

  delta = im_clip(-tc, tc, ((((q0-p0)<<2)+(p1-q1)+4)>>3));

  out[0] = im_clip(0, max_value, q0 - delta); // q prime 0
  out[-1*stride] = im_clip(0, max_value, p0 + delta); // p prime 0
  if (is_luma && aq_flag)
    out[1*stride] = q1 + im_clip(-tc0, tc0, (q2+((p0+q0+1)>>1)-(q1<<1))>>1); // q prime 1
  if (is_luma && ap_flag)
    out[-2*stride] = p1 + im_clip(-tc0, tc0, (p2+((p0+q0+1)>>1)-(p1<<1))>>1); // p prime 1
}


static should_inline void filtering_for_bS_equal_to_4(const int p0, const int p1, const int p2, const int p3, const int q0, const int q1, const int q2, const int q3,
                                               const int alpha, const int beta, unsigned int is_luma, pixel_t* out, int stride)
{
  unsigned int test = (im_abs(p0-q0) < ((alpha>>2)+2));
  int ap_flag = im_abs(p2-p0) < beta;
  int aq_flag = im_abs(q2-q0) < beta;

  // p prime 0..2
  if (is_luma && ap_flag && test)
  {
    out[-1*stride] = ( p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4 ) >> 3;
    out[-2*stride] = ( p2 + p1 + p0 + q0 + 2 ) >> 2;
    out[-3*stride] = ( 2*p3 + 3*p2 + p1 + p0 + q0 + 4 ) >> 3;
  }
  else
    out[-1*stride] =  ( 2*p1 + p0 + q1 + 2 ) >> 2;

  // q prime 0..2
  if (is_luma && aq_flag && test)
  {
    out[0*stride] = ( p1 + 2*p0 + 2*q0 + 2*q1 + q2 + 4 ) >> 3;
    out[1*stride] = ( p0 + q0 + q1 + q2 + 2 ) >> 2;
    out[2*stride] = ( 2*q3 + 3*q2 + q1 + q0 + p0 + 4 ) >> 3;
  }
  else
    out[0*stride] = ( 2*q1 + q0 + p1 + 2 ) >> 2;

}

// block offset is for chroma processing
static should_inline void filter_one_edge(uint8_t bSr[4], int indexA, int alpha, int beta, pixel_t* samples_,
                                   unsigned int is_horizontal_edge, unsigned int is_luma, unsigned int PicWidth, unsigned int maxV,
                                   unsigned int block_offset, unsigned int bit_depth_minus8)
{
  int bSv;
  int tc0;
  unsigned int i, k;
  int stride = is_horizontal_edge ? PicWidth : 1;
  int sample_offset = is_horizontal_edge ? 1 : PicWidth;
  int sample_offset4 = (4/block_offset)*sample_offset;
  pixel_t* samples;

  for ( i=0; i<4; i++)
  {
    if ((bSv = bSr[i]) != 0)
    {
      samples = samples_ + i*sample_offset4;
      if (bSv<4)
      {
        tc0 = bS_indexA_to_tc0[bSv-1][indexA] << bit_depth_minus8;
        for ( k=0; k<4; k+=block_offset)
        {
          LOAD_4F_SAMPLES(samples, stride);
          //if (cubu_debug)
          //  printf(" indexA %d, Alpha %d, Beta %d, bS %d |", indexA, alpha, beta, bSv);// CUBU DEBUG
          if (cubu_debug)
            printf(" %d", bSv); // CUBU DEBUG
          if (im_abs(p0-q0) < alpha && im_abs(p1-p0) < beta && im_abs(q1-q0) < beta) // then yes we will do the filtering  for those samples !
          {
            LOAD_2M_SAMPLES(samples, stride);
            if (cubu_debug)
              printf(" %d", tc0); // CUBU DEBUG
            filtering_for_bS_less_than_4(p0, p1, p2, q0, q1, q2, tc0, beta, is_luma,
                                         samples, stride, maxV);
          }
          samples += sample_offset;
        }
      }
      else // bS == 4
      {
        for ( k=0; k<4; k+=block_offset)
        {
          LOAD_4F_SAMPLES(samples, stride);
          //if (cubu_debug)
          //  printf(" indexA %d, Alpha %d, Beta %d, bS %d |", indexA, alpha, beta, bSv);// CUBU DEBUG
          if (cubu_debug)
            printf(" %d", bSv); // CUBU DEBUG
          if (im_abs(p0-q0) < alpha && im_abs(p1-p0) < beta && im_abs(q1-q0) < beta) // then yes we will do the filtering  for those samples !
          {
            LOAD_4L_SAMPLES(samples, stride);

            filtering_for_bS_equal_to_4(p0, p1, p2, p3, q0, q1, q2, q3, alpha, beta, is_luma,
                                        samples, stride);
          }
          samples += sample_offset;
        }
      }
    }
  }
}




should_inline void filter_mb(MbAttrib* curr_mb_attr, unsigned int mb_row, unsigned int mb_col,
    unsigned int disable_deblocking_filter_idc, unsigned int slice_num, int PicWidthInMbs, unsigned int field_pic_flag,
    slice_header_t** sh, unsigned int PicWidthY, unsigned int PicWidthC, pixel_t* Y, pixel_t* C[2],
    unsigned int maxY, unsigned int maxC, unsigned int mbc_sub_width, unsigned int mbc_sub_height,
    unsigned int mbc_height, unsigned int cfidc, unsigned int MbaffFrameFlag, unsigned int FilterOffsetA,
    unsigned int FilterOffsetB, unsigned int bit_depth_luma_minus8, unsigned int bit_depth_chroma_minus8, RefListEntry* RefPicList[2])
{
  pixel_t* y;
//  pixel_t* c;
  int j;


  if (!MbaffFrameFlag)
  {
#ifdef STM32F4XX
   #pragma data_alignment = 4
#endif
    uint8_t bS[2][4][4];// declare_aligned(4);
    int8_t qPav[3][3]; // [border][component]:   border: 0-inner, 1-left, 2-upper; component: 0-Y, 1-Cb, 2-Cr
    unsigned int transform_size_8x8_flag = curr_mb_attr->transform_size_8x8_flag;
    int indexA, indexA_inner;
    int indexB;
    int alpha, alpha_inner;
    int beta, beta_inner;


    fills_in_bS_nonmbaff(bS, qPav, curr_mb_attr, mb_row, mb_col, PicWidthInMbs, disable_deblocking_filter_idc,
        sh, slice_num, RefPicList, field_pic_flag, transform_size_8x8_flag, cfidc);



    // do Luma filtering
    // do vertical filtering
    y = Y;
    // 1st col
    if ( *(uint32_t*)bS[0][0] != 0)
    {
      indexA = im_clip(0, 51, qPav[1][0] + FilterOffsetA);
      alpha = indexA_to_alpha[indexA] << bit_depth_luma_minus8;
      indexB = im_clip(0, 51, qPav[1][0] + FilterOffsetB);
      beta = indexB_to_beta[indexB] << bit_depth_luma_minus8;
      //printf("indexA %d, Alpha %d, Beta %d |", indexA, alpha, beta); // CUBU DEBUG
      filter_one_edge(bS[0][0], indexA, alpha, beta, y, 0, 1, PicWidthY, maxY, 1, bit_depth_luma_minus8);
    }
    //printf("\n"); // CUBU DEBUG
    y += 4;
    // 2nd-4th col
    indexA_inner = im_clip(0, 51, qPav[0][0] + FilterOffsetA);
    alpha_inner = indexA_to_alpha[indexA_inner] << bit_depth_luma_minus8;
    indexB = im_clip(0, 51, qPav[0][0] + FilterOffsetB);
    beta_inner = indexB_to_beta[indexB] << bit_depth_luma_minus8;

      for (j=1; j<4; j += 1)
      {

        filter_one_edge(bS[0][j], indexA_inner, alpha_inner, beta_inner, y, 0, 1, PicWidthY, maxY, 1, bit_depth_luma_minus8);
        y += 4;
        //printf("\n"); // cubu
      }


    // do horizontal filtering
    y = Y;
    // 1st row

    //printf("\n"); // CUBU DEBUG
    y += 4*PicWidthY;
    // 2nd-4th row
    // inner params are already calculated !

      for (j=1; j<4; j += 1)
      {
        //printf("edge: %d ", j); // CUBU DEBUG
        //printf("indexA %d, Alpha %d, Beta %d |", indexA_inner, alpha_inner, beta_inner); // CUBU DEBUG
        filter_one_edge(bS[1][j], indexA_inner, alpha_inner, beta_inner, y, 1, 1, PicWidthY, maxY, 1, bit_depth_luma_minus8);
        y += 4*PicWidthY;
        //printf("\n"); // cubu
      }

    //printf("\n"); // CUBU DEBUG
    //cubu_debug = 0;

    //printf("Lets See what this is: %.2X ",qPav[1][1]);
    cubu_debug = 0;
  }


}


// TODO: hard code some params (ex: cfidc) in order to optimize the compiled code

void filter_image(Picture* pic, unsigned int bottom_field_flag)
{
//  slice_header_t* slice0 = pic->field_sh[0]; // get a slice header. It is used for variables that are the same for the whole picture
//  unsigned int mb_row, mb_col/*, mb_idx*/;
//  seq_parameter_set_rbsp_t* sps = slice0->sps;
 // unsigned int PicWidthInMbs = sps->PicWidthInMbs;
//  unsigned int PicWidthY = PicWidthInMbs * 16;
  /*
  unsigned int PicHeightInMbs = slice0->PicHeightInMbs;
  unsigned int MbaffFrameFlag = slice0->MbaffFrameFlag;
//  MbAttrib* curr_mb_attr = pic->field_data.mb_attr;
  unsigned int cfidc = sps->chroma_format_idc;
  unsigned int mbc_width = MbWidthC[cfidc];
  unsigned int mbc_height = MbHeightC[cfidc];
  unsigned int mbc_size = mbc_width*mbc_height;
  unsigned int PicWidthC = PicWidthInMbs * mbc_width;
  unsigned int clipY = (1<<sps->BitDepthY)-1;
  unsigned int clipC = (1<<sps->BitDepthC)-1;
  unsigned int bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
  unsigned int bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
  unsigned int mbc_sub_width = SubWidthC[cfidc];
  unsigned int mbc_sub_height = SubHeightC[cfidc];
  unsigned int field_pic_flag = slice0->field_pic_flag;
  unsigned int is_field_pic_or_mbaff = MbaffFrameFlag || field_pic_flag;


  LUD_DEBUG_ASSERT(bit_depth_luma_minus8 == 0); // Not yet supported
  LUD_DEBUG_ASSERT(bit_depth_chroma_minus8 == 0); // Not yet supported

  pixel_t* Y;
  pixel_t* C[2];
  pixel_t* y;
  pixel_t* c[2];

  Y = (pixel_t*)((uint32_t)pic->Y + (bottom_field_flag!=0)*PicWidthY);
  C[0] = (pixel_t*)((uint32_t)pic->C[0] + (bottom_field_flag!=0)*PicWidthC);
  C[1] = (pixel_t*)((uint32_t)pic->C[1] + (bottom_field_flag!=0)*PicWidthC);

  if (field_pic_flag) // Hereafter, PicWidthX is actually used as image stride, and not really as the width of the image
  {
    PicWidthY <<= 1;
    PicWidthC <<= 1;
  }

  for (mb_row = 0; mb_row<PicHeightInMbs; mb_row+=(1+MbaffFrameFlag))
  {
    y = Y;
    c[0] = C[0];
    c[1] = C[1];
    for (mb_col = 0; mb_col<PicWidthInMbs; mb_col++)
    {
      unsigned int slice_num = curr_mb_attr->slice_num;
      slice_header_t** sh = pic->field_sh;
      unsigned int disable_deblocking_filter_idc = sh[slice_num]->disable_deblocking_filter_idc;
      unsigned int FilterOffsetA = sh[slice_num]->FilterOffsetA;
      unsigned int FilterOffsetB = sh[slice_num]->FilterOffsetB;
      if ( disable_deblocking_filter_idc != 1)
      {
        */

        /*if (pdd->dec_num == 17 && mb_col == 4 && mb_row == 2)
          printf("toto\n");*/

          /*
        filter_mb(curr_mb_attr, mb_row, mb_col, disable_deblocking_filter_idc, slice_num, PicWidthInMbs,
            field_pic_flag, sh, PicWidthY, PicWidthC, y, c, clipY, clipC, mbc_sub_width, mbc_sub_height, mbc_height,
            cfidc, MbaffFrameFlag, FilterOffsetA, FilterOffsetB, bit_depth_luma_minus8, bit_depth_chroma_minus8,
            sh[slice_num]->RefPicList);



      // FIXME DEBUG
        //if (mb_col==5 && mb_row==4)
//          print_mb_samples2(curr_mb_attr+PicWidthInMbs, mb_col, mb_row+1, y+16*PicWidthY, c[0]+PicWidthC*8, c[1]+PicWidthC*8, PicWidthY, PicWidthC);


      }
      curr_mb_attr++;
      y += 16;
      c[0] += mbc_width;
      c[1] += mbc_width;
    }
    curr_mb_attr += MbaffFrameFlag * PicWidthInMbs;
    Y += 16*16 * (1+is_field_pic_or_mbaff) * PicWidthInMbs;

    C[0] += mbc_size * (1+is_field_pic_or_mbaff) * PicWidthInMbs;
    C[1] += mbc_size * (1+is_field_pic_or_mbaff) * PicWidthInMbs;
  }

*/
}


