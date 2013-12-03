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



#ifndef __SYNTAX_TYPES_H__
#define __SYNTAX_TYPES_H__

#include <inttypes.h>
#include "common.h"
#include "bitstream_types.h"


#define Extended_SAR      255


typedef enum
{
  NALU_TYPE_SLICE    = 1,
  NALU_TYPE_DPA      = 2,
  NALU_TYPE_DPB      = 3,
  NALU_TYPE_DPC      = 4,
  NALU_TYPE_IDR      = 5,
  NALU_TYPE_SEI      = 6,
  NALU_TYPE_SPS      = 7,
  NALU_TYPE_PPS      = 8,
  NALU_TYPE_AUD      = 9,
  NALU_TYPE_EOSEQ    = 10,
  NALU_TYPE_EOSTREAM = 11,
  NALU_TYPE_FILL     = 12,
  NALU_TYPE_SPSE     = 13,
  NALU_TYPE_SLICEWP  = 19,
} nal_unit_type_t;


typedef enum
{
  P_SLICE       = 0,
  B_SLICE       = 1,
  I_SLICE       = 2,
  SP_SLICE      = 3,
  SI_SLICE      = 4,
  P_SLICE_CONT  = 5,
  B_SLICE_CONT  = 6,
  I_SLICE_CONT  = 7,
  SP_SLICE_CONT = 8,
  SI_SLICE_CONT = 9,
} slice_type_t;

// Do not change the order of those enum, the code is based on their order/value
typedef enum
{
  // From I Slices
  I_NxN          = 0,
  I_16x16_0_0_0,
  I_16x16_1_0_0,
  I_16x16_2_0_0,
  I_16x16_3_0_0,
  I_16x16_0_1_0,  // 5
  I_16x16_1_1_0,
  I_16x16_2_1_0,
  I_16x16_3_1_0,
  I_16x16_0_2_0,
  I_16x16_1_2_0, // 10
  I_16x16_2_2_0,
  I_16x16_3_2_0,
  I_16x16_0_0_1,
  I_16x16_1_0_1,
  I_16x16_2_0_1, // 15
  I_16x16_3_0_1,
  I_16x16_0_1_1,
  I_16x16_1_1_1,
  I_16x16_2_1_1,
  I_16x16_3_1_1, // 20
  I_16x16_0_2_1,
  I_16x16_1_2_1,
  I_16x16_2_2_1,
  I_16x16_3_2_1,
  I_PCM,         // 25

  // From SI Slices
  SI  = 26,

  // From P and SP Slices
  P_L0_16x16  = 27,
  P_L0_L0_16x8,
  P_L0_L0_8x16,
  P_8x8,
  P_8x8ref0,
  P_Skip,

  // From B Slices
  B_Direct_16x16= 33,
  B_L0_16x16,
  B_L1_16x16,
  B_Bi_16x16,
  B_L0_L0_16x8,
  B_L0_L0_8x16,
  B_L1_L1_16x8,
  B_L1_L1_8x16,
  B_L0_L1_16x8,
  B_L0_L1_8x16,
  B_L1_L0_16x8,
  B_L1_L0_8x16,
  B_L0_Bi_16x8,
  B_L0_Bi_8x16,
  B_L1_Bi_16x8,
  B_L1_Bi_8x16,
  B_Bi_L0_16x8,
  B_Bi_L0_8x16,
  B_Bi_L1_16x8,
  B_Bi_L1_8x16,
  B_Bi_Bi_16x8,
  B_Bi_Bi_8x16,
  B_8x8,
  B_Skip,

  NB_OF_MB_TYPE // = 57
} MB_TYPE;

// Do not change the order of those enum, the code is based on their order/value
typedef enum
{
  // From P Slices
  P_L0_8x8 = 0,
  P_L0_8x4,
  P_L0_4x8,
  P_L0_4x4,

  // From B Slices
  B_Direct_8x8 = 4,
  B_L0_8x8,
  B_L1_8x8,
  B_Bi_8x8,
  B_L0_8x4,
  B_L0_4x8,
  B_L1_8x4,
  B_L1_4x8,
  B_Bi_8x4,
  B_Bi_4x8,
  B_L0_4x4,
  B_L1_4x4,
  B_Bi_4x4,

  NB_OF_SUB_MB_TYPE // = 17
} SUB_MB_TYPE;

// Do not change the order of those enum, the code is based on their order/value
typedef enum
{
  Intra_4x4 = 0,
  Intra_8x8,
  Intra_16x16,
  Direct,
  Pred_L0,
  Pred_L1,
  BiPred
} PREDICTION_MODE;

// This enum is also set equal to ctxBlockCat
typedef enum
{
  LUMA_DC_LEVEL   = 0,
  LUMA_AC_LEVEL   = 1,
  LUMA_LEVEL      = 2,
  CHROMA_DC_LEVEL = 3,
  CHROMA_AC_LEVEL = 4,
  LUMA8x8_LEVEL   = 5,
} RESIDUAL_BLOCK_TYPE;

typedef struct nal_unit_s
{
  uint8_t         nal_ref_idc;        // All u(2)
  uint8_t         nal_unit_type;      // All u(5)
  uint32_t        NumBytesInRBSP;     // Computed
  uint8_t*        rbsp_byte;          // Computed
  uint8_t         trailing_bits;      // Computed. It include the "stop_one_bit", so the range is 1-7 included
  uint32_t        data_length_in_bits;// length in bit of the data into the RBSP
  BitStreamContext bs; // bitstream reader. It is setup into the parse nalu function and can be used for following parser functions
} nal_unit_t;




typedef struct hrd_parameters_s
{
  uint8_t   cpb_cnt_minus1; // 0 ue(v)
  uint8_t   bit_rate_scale; // 0 u(4)
  uint8_t   cpb_size_scale; // 0 u(4)
  uint32_t* bit_rate_value_minus1; // 0 ue(v) // dyn alloc array
  uint32_t* cpb_size_value_minus1; // 0 ue(v) // dyn alloc array
  uint8_t*  cbr_flag; // 0 u(1) // dyn alloc array
  uint8_t   initial_cpb_removal_delay_length_minus1; // 0 u(5)
  uint8_t   cpb_removal_delay_length_minus1; // 0 u(5)
  uint8_t   dpb_output_delay_length_minus1; // 0 u(5)
  uint8_t   time_offset_length; // 0 u(5)
} hrd_parameters_t;



typedef struct vui_parameters_s
{
  uint8_t   aspect_ratio_info_present_flag; // 0    u(1)
  uint8_t   aspect_ratio_idc; // 0    u(8)
  uint16_t  sar_width; // 0    u(16)
  uint16_t  sar_height; // 0    u(16)
  uint8_t   overscan_info_present_flag; // 0    u(1)
  uint8_t   overscan_appropriate_flag; // 0    u(1)
  uint8_t   video_signal_type_present_flag; // 0    u(1)
  uint8_t   video_format; // 0    u(3)
  uint8_t   video_full_range_flag; // 0    u(1)
  uint8_t   colour_description_present_flag; // 0    u(1)
  uint8_t   colour_primaries; // 0    u(8)
  uint8_t   transfer_characteristics; // 0    u(8)
  uint8_t   matrix_coefficients; // 0    u(8)
  uint8_t   chroma_loc_info_present_flag; // 0    u(1)
  uint8_t   chroma_sample_loc_type_top_field; // 0    ue(v)
  uint8_t   chroma_sample_loc_type_bottom_field; // 0  ue(v)
  uint8_t   timing_info_present_flag; // 0  u(1)
  uint32_t  num_units_in_tick; // 0  u(32)
  uint32_t  time_scale; // 0  u(32)
  uint8_t   fixed_frame_rate_flag; // 0  u(1)
  uint8_t   nal_hrd_parameters_present_flag; // 0  u(1)
  hrd_parameters_t* nal_hrd_parameters;
  uint8_t   vcl_hrd_parameters_present_flag; // 0  u(1)
  hrd_parameters_t* vcl_hrd_parameters;
  uint8_t   low_delay_hrd_flag; // 0  u(1)
  uint8_t   pic_struct_present_flag; // 0  u(1)
  uint8_t   bitstream_restriction_flag; // 0  u(1)
  uint8_t   motion_vectors_over_pic_boundaries_flag; // 0  u(1)
  uint8_t   max_bytes_per_pic_denom; // 0  ue(v)
  uint8_t   max_bits_per_mb_denom; //  0  ue(v)
  uint8_t   log2_max_mv_length_horizontal; // 0  ue(v)
  uint8_t   log2_max_mv_length_vertical; // 0  ue(v)
  uint8_t   num_reorder_frames; // 0  ue(v)
  uint8_t   max_dec_frame_buffering; // 0  ue(v)
} vui_parameters_t;



typedef struct seq_parameter_set_rbsp_s // SPS
{
  OBJECT_HEADER_DECL;

  uint8_t           profile_idc;              // 0 u(8)
  uint8_t           constraint_set0_flag;     // 0 u(1)
  uint8_t           constraint_set1_flag;     // 0 u(1)
  uint8_t           constraint_set2_flag;     // 0 u(1)
  uint8_t           constraint_set3_flag;     // 0 u(1)
  uint8_t           level_idc;                // 0 u(8)
  uint8_t           seq_parameter_set_id;     // 0 ue(v)
  uint8_t           chroma_format_idc; // 0 ue(v)
  uint8_t           residual_colour_transform_flag; // 0 u(1)
  uint8_t           bit_depth_luma_minus8; // 0 ue(v)
  uint8_t           bit_depth_chroma_minus8; // 0 ue(v)
  uint8_t           qpprime_y_zero_transform_bypass_flag; // 0 u(1)
  uint8_t           seq_scaling_matrix_present_flag; // 0 u(1)
  uint8_t           seq_scaling_list_present_flag[8]; // 0 u(1)
  uint8_t           ScalingList4x4[6][16];
  uint8_t           ScalingList8x8[2][64];
  uint8_t           log2_max_frame_num_minus4;// 0 ue(v)
  uint8_t           pic_order_cnt_type;       // 0 ue(v)
  uint8_t           log2_max_pic_order_cnt_lsb_minus4; // 0 ue(v)
  uint8_t           delta_pic_order_always_zero_flag; // 0 u(1)
  int32_t           offset_for_non_ref_pic;   // 0 se(v)
  int32_t           offset_for_top_to_bottom_field; // 0 se(v)
  uint8_t           num_ref_frames_in_pic_order_cnt_cycle; // 0 ue(v)
  int32_t*          offset_for_ref_frame; // 0 se(v)
  uint8_t           num_ref_frames;           // 0 ue(v)
  uint8_t           gaps_in_frame_num_value_allowed_flag; // 0 u(1)
  uint16_t          pic_width_in_mbs_minus1;  // 0 ue(v)
  uint16_t          pic_height_in_map_units_minus1; // 0 ue(v)
  uint8_t           frame_mbs_only_flag;      // 0 u(1)
  uint8_t           mb_adaptive_frame_field_flag; // 0 u(1)
  uint8_t           direct_8x8_inference_flag;// 0 u(1)
  uint8_t           frame_cropping_flag;      // 0 u(1)
  uint16_t          frame_crop_left_offset;   // 0 ue(v)
  uint16_t          frame_crop_right_offset;  // 0 ue(v)
  uint16_t          frame_crop_top_offset;    // 0 ue(v)
  uint16_t          frame_crop_bottom_offset; // 0 ue(v)
  uint8_t           vui_parameters_present_flag; // 0 u(1)
  vui_parameters_t* vui_parameters;

  // derived variables
  uint16_t          PicHeightInMapUnits;
  uint16_t          FrameHeightInMbs;
  uint16_t          PicWidthInMbs;
  uint16_t          PicSizeInMapUnits;
  uint8_t           BitDepthY;
  uint8_t           QpBdOffsetY;
  uint8_t           BitDepthC;
  uint8_t           QpBdOffsetC;
  uint32_t          MaxFrameNum;
} seq_parameter_set_rbsp_t; // SPS




typedef struct pic_parameter_set_rbsp_s // PPS
{
  OBJECT_HEADER_DECL;

  uint8_t   pic_parameter_set_id; // 1   ue(v)
  uint8_t   seq_parameter_set_id; // 1   ue(v)
  uint8_t   entropy_coding_mode_flag; // 1   u(1)
  uint8_t   pic_order_present_flag; // 1   u(1)
  uint8_t   num_slice_groups_minus1; // 1   ue(v)
  uint8_t   slice_group_map_type; // 1   ue(v)
  uint16_t* run_length_minus1; // 1   ue(v)
  uint16_t* top_left; // 1   ue(v)
  uint16_t* bottom_right; // 1   ue(v)
  uint8_t   slice_group_change_direction_flag; // 1   u(1)
  uint16_t  slice_group_change_rate_minus1; // 1   ue(v)
  uint16_t  pic_size_in_map_units_minus1; //1   ue(v)
  uint8_t*  slice_group_id; // 1   u(v)
  uint8_t   num_ref_idx_active[2]; // 1   ue(v) //  uint8_t   num_ref_idx_l1_active_minus1; // 1   ue(v)
  uint8_t   weighted_pred_flag; // 1   u(1)
  uint8_t   weighted_bipred_idc; // 1   u(2)
  int8_t    pic_init_qp_minus26; // 1   se(v)
  int8_t    pic_init_qs_minus26; // 1   se(v)
  int8_t    chroma_qp_index_offset; // 1   se(v)
  uint8_t   deblocking_filter_control_present_flag; // 1   u(1)
  uint8_t   constrained_intra_pred_flag; // 1   u(1)
  uint8_t   redundant_pic_cnt_present_flag; // 1   u(1)
  uint8_t   transform_8x8_mode_flag; // 1   u(1)
  uint8_t   pic_scaling_matrix_present_flag; // 1   u(1)
  uint8_t   pic_scaling_list_present_flag[ 8 ]; // 1   u(1)
  uint8_t   ScalingList4x4[6][16];
  uint8_t   ScalingList8x8[2][64];
  int8_t    second_chroma_qp_index_offset; // 1 se(v)

  // derived variables
  uint16_t  SliceGroupChangeRate;
  uint16_t  LevelScale4x4[6][6][16];
  uint16_t  LevelScale8x8[2][6][64];
} pic_parameter_set_rbsp_t; // PPS


typedef struct ref_pic_list_reordering_s
{
  uint32_t* pic_list_reordering_commands[2];
  uint8_t   ref_pic_list_reordering_flag[2]; // 2 u(1)
} ref_pic_list_reordering_t;


typedef struct pred_weight_table_s
{
  uint8_t   luma_log2_weight_denom;
  uint8_t   chroma_log2_weight_denom;
  int8_t    *luma_weight_l[2]; // [LX]   LX = 0 or 1
  int16_t   *luma_offset_l[2]; // [LX]
  int8_t    *chroma_weight_l[2][2]; // [iCbCr][LX] Cb=0, Cr=1
  int16_t   *chroma_offset_l[2][2]; // [iCbCr][LX]
} pred_weight_table_t;



typedef struct dec_ref_pic_marking_s
{
  uint8_t   no_output_of_prior_pics_flag; // 2|5 u(1)
  uint8_t   long_term_reference_flag; // 2|5 u(1)
  uint8_t   adaptive_ref_pic_marking_mode_flag; // 2|5 u(1)
  //uint32_t  difference_of_pic_nums_minus1; // 2|5 ue(v)
  //uint32_t  long_term_pic_num; // 2|5 ue(v)
  //uint32_t  long_term_frame_idx; // 2|5 ue(v)
  //uint32_t  max_long_term_frame_idx_plus1; // 2|5 ue(v)
#define MAX_NB_OF_MMCO_COMMANDS 68
  uint8_t   nb_of_mmco_cmd;
  uint32_t  mmco_commands[MAX_NB_OF_MMCO_COMMANDS];
} dec_ref_pic_marking_t;


typedef struct slice_header_s
{
  OBJECT_HEADER_DECL;

  uint16_t  mtDepCount;

  // Group Map variables
  uint8_t*  MbToSliceGroupMap;

  uint16_t  first_mb_in_slice; // 2    ue(v)
  uint8_t   slice_type; // 2    ue(v)
  uint8_t   pic_parameter_set_id; // 2    ue(v)
  uint16_t  frame_num; // 2    u(v)
  uint8_t   field_pic_flag; // 2    u(1)
  uint8_t   bottom_field_flag; // 2    u(1)
  uint16_t  idr_pic_id; // 2    ue(v)
  uint16_t  pic_order_cnt_lsb; // 2    u(v)
  int32_t   delta_pic_order_cnt_bottom; // 2    se(v)
  int32_t   delta_pic_order_cnt[2]; //  2    se(v)
  uint8_t   redundant_pic_cnt; // 2    ue(v)
  uint8_t   direct_spatial_mv_pred_flag; // 2    u(1)
  uint8_t   num_ref_idx_active_override_flag; // 2    u(1)
  uint8_t   num_ref_idx_active[2]; // 2    ue(v)
  ref_pic_list_reordering_t ref_pic_list_reordering;//                                                    2
  pred_weight_table_t* pred_weight_table; //                                                        2
  dec_ref_pic_marking_t* dec_ref_pic_marking; // 2
  uint8_t   cabac_init_idc; // 2    ue(v)
  int8_t    slice_qp_delta; // 2    se(v)
  uint8_t   sp_for_switch_flag; // 2    u(1)
  int8_t    slice_qs_delta; // 2 se(v)
  uint8_t   disable_deblocking_filter_idc; // 2 ue(v)
  int8_t    slice_alpha_c0_offset_div2; // 2 se(v)
  int8_t    slice_beta_offset_div2; // 2 se(v)
  uint16_t  slice_group_change_cycle; // 2 u(v)

  // inferred values needed for further processing
  seq_parameter_set_rbsp_t* sps;
  pic_parameter_set_rbsp_t* pps;
  struct RefListEntry_s* RefPicList[2]; // [LX][refPicIdx]
  uint8_t   slice_type_modulo5; // needed in order not to to the test for normal and _CONT slice_type.
  uint8_t   MbaffFrameFlag;
  uint16_t  PicSizeInMbs;
  uint16_t  PicHeightInMbs;
  uint16_t  original_frame_num; // this is equal to the original frame_num field but is not set to 0 with mmco equal to 5.
  uint8_t   has_mmco5;
  uint32_t  MaxPicNum;
  uint32_t  CurrPicNum;
  int8_t    SliceQPY;
  int8_t    FilterOffsetA; // 2 se(v)
  int8_t    FilterOffsetB; // 2 se(v)
  uint8_t   nal_ref_idc; // copied from the nalu struct
  uint8_t   nal_unit_type; // copied from the nalu struct
  uint16_t  mb_nb; // number of macroblocks in that slice

} slice_header_t; // SH





#endif //__SYNTAX_TYPES_H__
