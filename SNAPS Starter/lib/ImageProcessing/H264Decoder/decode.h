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


#ifndef __DECODE_H__
#define __DECODE_H__

#include "common.h"
#include "syntax.h"
#include "bitstream_types.h"
#include "../image.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif



#define NB_OF_REF_PICS  32

typedef enum
{
  UNUSED_FOR_REF = 0,
  SHORT_TERM_REF = 1,
  LONG_TERM_REF  = 2,
} REF_PIC_TYPE;


typedef struct RefListEntry_s
{
  struct Picture_s*   ref_pic;
  uint8_t   parity; // 0 = top field, 1 = bottom field. When field_pic_flag==1, parity overload ref_struct value. This is necessary when splitting frame into fields
  uint8_t   ref_pic_type; // see REF_PIC_TYPE enum
}
RefListEntry;



typedef struct
{
  uint8_t IntraPredMode[16];  // equals to Intra4x4PredMode or Intra8x8PredMode.
                              // When Intra8x8PredMode, each entry is duplicated so that Intra8x8PredMode[x] = IntraPredMode[y=(x>>1)*8 + (x&1)*2] (when x=0,1,2,3 => y=0,2,8,10)
                              // the index is in raster4x4 order
  uint8_t intra_chroma_pred_mode;
}
IntraPred;



typedef struct
{
  uint32_t partWidths;   // 2bits per partWidth/Height_div4-1. bit01=> first (sub)partition, bit23 => second (sub)partition...
  uint32_t partHeights;  // an array [partwidth][partHeight][curr idx] gives the next idx (see implementation)
  // Before caching (syntax parsing)
  //    RefIdxL[L0-L1][mpPartIdx(<4)]
  //    MvL[L0-L1][mpPartIdx*4+subMbPartIdx][X-Y]
  // In the cache
  //     RefIdxL[L0-L1][block row*4+block col]   <= MUST BE ALIGNED on 32 bits...
  //     MvL[L0-L1][sub block row*4+sub block col][X-Y]   <= MUST BE ALIGNED on 32 bits...
  int8_t RefIdxL[2][16];
  //  it is not necessary to store directly predFlagLX, as it can be derived as predFlagLX = refIdxLX>=0
  int16_t MvL[2][16][2];
  uint8_t sub_mb_type[4]; //sub_mb_type[mbPartIdx]
}
InterPred;

typedef struct
{
  union
  {
    IntraPred intra;
    InterPred inter;
  } pred;
  //uint32_t mtDecInfo;
  uint16_t slice_num;
  uint8_t mb_type;
  int8_t QPY;
  int8_t QPC[2];
  uint8_t luma_total_coeff[16]; // coeffs for each luma 4x4 block (in raster4x4 order)
  uint8_t chroma_total_coeff[2][16]; // coeffs for each chroma 4x4 block (size is worst case: cfidc==3) (in raster2x2, raster2x4, raster4x4 order (according to cfidc))
  unsigned int CodedBlockPatternLuma:4; // necessary for CABAC decoding
  unsigned int CodedBlockPatternChroma:2; // necessary for CABAC decoding
  unsigned int chroma_dc_coeffs_non_null:2; // Bit0:Cb, Bit1:Cr. When set to 1=> at least one DC coeff is available, 0 => all DC coeffs are null
  unsigned int luma_dc_coeffs_non_null:1; // only relevant for Intra16x16 mb. When set to 1=> at least one DC coeff is available, 0 => all DC coeffs are null
  unsigned int mb_field_decoding_flag:1; /* Relevant even when MbaffFrameFlag==0. When MbaffFrameFlag==0, this flag is equals to field_pic_flag. */
  unsigned int transform_size_8x8_flag:1;
  unsigned int transform_bypass:1; // when set to one, the scaling and transform operation should be bypassed
  unsigned int mb_skip_flag:1; // Used for CABAC decoding
  unsigned int left_mb_is_available:1;
  unsigned int left_mb_is_field:1;
  unsigned int up_mb_is_available:1;
  unsigned int up_mb_is_field:1;
  unsigned int upleft_mb_is_available:1;
  unsigned int upleft_mb_is_field:1;
  unsigned int upright_mb_is_available:1;
  unsigned int upright_mb_is_field:1;
}
MbAttrib;


typedef struct
{
  uint8_t mb_attr_data[60000];
  MbAttrib* mb_attr; // points to the global picture mb attributes
  int16_t* data;  // points to the global picture data buffer. Each mb has its data as: Y residual (or pcm data), Cb residual (or pcm data) and then Cr residual (or pcm data).
                  // the data inside a mb is in raster order
}
PictureData;


typedef struct
{
  void* Y;
  void* C[2];
}
DecodedFrameData;

typedef struct Picture_s
{
  // The picture will be freed only if those 3 ref counter a set to -1 (the value is initialized to 0 for 1 user)
  int32_t field_use_count[2]; // ref counter for each fields of the picture
  int32_t dec_data_use_count; // ref counter for decoded data of this picture

  //uint32_t mtPicInfo; // used for multithread decoding
  void* mtRevDep[2];
  int32_t mtSliceCount[2]; // used to order the mb decoding process when using multiple threads


  void* Y;      // contains decoded samples (fields are interleaved, when relevant)
  void* C[2];   //

  uint32_t image_buffer_size; // size of the image in bytes. Set and used in the image output module
  uint32_t width;
  uint32_t height;

  PictureData field_data; // 0-Top Field, 1-Bot Field. For Frames, both points to the frame. Contains pre-decoded data
  slice_header_t* field_sh[1]; // array of all slices of this picture, for each fields. For frame, point to the same array
  unsigned int slice_num; // number of slices for each fields

  unsigned int dec_num[2]; // picture number in decoding order
  int32_t   PicOrderCnt;
  int32_t   FieldOrderCnt[2]; //0-TopFieldOrderCnt,  1-BottomFieldOrderCnt
  uint16_t  frame_num;
  uint8_t   ref_struct; /* bit 0 specify that top fields is reference fields, bit 1 is for bottom field, bit 2 specify that this is a frame picture
                            a frame with only the bottom field as a reference will have the bits2-0= 110
                            a complementary reference field pair will have the bits2-0= 011
                            a non paired reference top field will have the bits2-0= 001
                         */
  uint8_t   structure; /* same as ref_struct field, but not for reference. For instance, a Comp Field Pair has a structure=011 and could have a ref_struct=001 */
  uint8_t   non_existing_flag; // 1 when the ref pic is a non-existing frame.


} Picture;

typedef struct
{
  uint8_t       cabac_ctx_vars[460]; // for each ctxIdx: = pStateIdx*2+valMPS => valMPS = cabac_ctx_vars&1, pStateIdx=cabac_ctx_vars>>1;
  unsigned int  codIRange;
  unsigned int  codIOffset;

  MbAttrib* mbA; // used to derive mb_skip_flag and mb_type
  MbAttrib* mbB; // they are put here so to be derived only once !
  uint8_t   cbpA; // used when MbaffFrameFlag==1 and curr and left mb are not the same kind (field/frame).
                  // see cabac_decode_coded_block_pattern().
} CABACContext;

typedef struct
{
  Picture* pic;
  slice_header_t* prev_sh; // point to the previously decoded slice header.
  unsigned int max_num_of_slices;

  // POC process variables
  int32_t prevPicOrderCntMsb;
  int32_t prevPicOrderCntLsb;
  int32_t PicOrderCntMsb;
  int32_t FrameNumOffset;
  int32_t prevFrameNumOffset;
  uint16_t prevFrameNum;
  uint16_t PrevRefFrameNum;


  // Number of pictures decoded since the beginning... debug purpose..
  unsigned int dec_num;
  unsigned int pic_not_finished; // 1- The curr pic is not finished, 0-the curr has finished to decode
  unsigned int decoding_finished; // set when no more nalu are available

  int           MaxLongTermFrameIdx;

  uint8_t       is_second_field_of_a_pair;

  CABACContext  cabac_ctx;

  uint32_t      minMbDecodingOrder; // The minimal decoding number for a MB in the current picture.
}
PictureDecoderData;

typedef struct
{
  seq_parameter_set_rbsp_t* sps[32];
  pic_parameter_set_rbsp_t* pps[256];
}
GlobalDecoderData;

extern GlobalDecoderData gdd;




RetCode decoder_init();
RetCode decoder_destroy();
RetCode decode_nalu(uint8_t* data, uint32_t* length);
Picture* get_pic();
void decode_image_init();
void decode_image(PictureDecoderData* pdd, Picture* pic, unsigned int bottom_field_flag, int16_t* datarray, FM_FILE* outfile, int currMbAddr,
                  MbAttrib* curr_mb_attr, pixel_t* topy, pixel_t* topcb, pixel_t* topcr, pixel_t* ysamp, pixel_t* cbsamp, pixel_t* crsamp);
void decode_image_end();
void filter_image(Picture* pic, unsigned int bottom_field_flag);
void init_dpb();
void add_image_to_dpb(Picture* image);
void flush_dpb();
void close_dpb();


#endif //__DECODE_H__
