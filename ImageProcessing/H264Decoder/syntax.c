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

#include "system.h"
#include "common.h"
#include "syntax.h"
#include "bitstream.h"
#include "defaulttables.h"

// Parse the next nal unit from the buffer. If preparsed==0, it work in place and can modify the stream_buffer.
//    Also, the content of nalu structure is valid as long as the stream_buffer content is valid and unmodified
//      (apart from this function of course). So do not expect that you can modify/release/... stream_buffer just
//      after this function returns (but rather when the current nalu has been decoded).
// stream_buffer must point to the input data buffer.
// stream_available_consumed is IN: bytes available into the stream_buffer, OUT: bytes consumed in the input buffer
// nalu: pointer to a nal_unit_t structure already allocated
// preparsed: 0: the nalu is not pre-parsed, so the stream_buffer must start with a start code, and it may contains
//                emulation_prevention_three_byte (000003 sequence)
//            1: the nalu is pre-parsed, so there is no start code at the beginning of stream_buffer, and any
//                emulation 3 bytes have already been removed from the buffer
// Note: when preparsed==0, the last nalu must be followed by a (fake) start code (000001). This will ensure the caller that
//        we indeed have a complete nalu !
RetCode parse_nal_unit(uint8_t* stream_buffer, uint32_t* stream_available_consumed, nal_unit_t* nalu, unsigned int preparsed)
{
  uint8_t* src = stream_buffer;
  uint32_t lenin = *stream_available_consumed;
  //lenin = lenin + 4;
  BitStreamContext* bs;


  LUD_DEBUG_ASSERT(lenin >= 2); // That would be a weird nalu ! (A size less than 2...)


  if (!preparsed)
  {
    uint8_t* dst = stream_buffer;
    int i = 0; // src index
    int j = 0;  // dst index
    int s = 0; // 1 => start code detected

    // Check for start code

    nalu->rbsp_byte = stream_buffer + i;
    dst += i;

    // Remove emulation 3bytes and go until next start code or end of stream
    while (i<(int)lenin-1)
    {
      if(src[i])
      {
        *(uint16_t*)(dst+j) = *(uint16_t*)(src+i); // Can be unaligned 16 bits copy !
        i += 2; j += 2;
        continue;
      }
      if(src[i-1]==0) {i--;j--;}
      if(src[i+1]==0 && (i+2<lenin) && src[i+2]<=3)
      {
        if(src[i+2]==1 || src[i+2]==0 ) // 0x000001 or 0x0000.(x*00).01 start code detected
        {
          s = 1;
          break;
        }
        else //0x000003 detected - this is "emulation_prevention_three_byte". see H264 spec
        {
          *(uint16_t*)(dst+j) = 0; // Can be unaligned 16 bits write !
          j += 2;
          i += 3;
          continue;
        }
      }
      else //false detection
      {
        *(uint16_t*)(dst+j) = *(uint16_t*)(src+i); // Can be unaligned 16 bits copy !
        i += 2; j += 2;
      }
    }

    // If we do not have a following start code, it means that the stream buffer was too short, we need to report the error

    if (j<1)
      return ERR_NALU_EMPTY;

    *stream_available_consumed = i;
    nalu->NumBytesInRBSP = j;
  }


  else // this nalu was preparsed, just set the structure !
  {
    nalu->rbsp_byte = stream_buffer;
    nalu->NumBytesInRBSP = lenin;
  }

  // Here we are: the NALU RBSP is cleaned from its start code and any 'emulation prevention bytes'
  bs = &nalu->bs;
  init_bitstream(bs, nalu->rbsp_byte);
#ifndef NDEBUG
  int forbidden_zero_bit = bs_read_u1(bs);
  LUD_DEBUG_ASSERT(forbidden_zero_bit == 0);
#else // NDEBUG
  bs_skip_n(bs, 1);  //forbidden_zero_bit All f(1)
#endif
  nalu->nal_ref_idc = bs_read_un(bs, 2); // nal_ref_idc        All u(2)
  nalu->nal_unit_type =  bs_read_un(bs, 5); //nal_unit_type      All u(5)


  nalu->rbsp_byte++;
  nalu->NumBytesInRBSP--;

  // Init Bitstream context for subsequent parsing functions...
  init_bitstream(bs, nalu->rbsp_byte);

  // Remove trailing bytes if any. This would include cabac_zero_word. (see parse_rbsp_slice_trailing_bits function)
  // TODO I may need to keep those byte when processing CABAC... we will see...
  while(nalu->NumBytesInRBSP > 1 && !nalu->rbsp_byte[nalu->NumBytesInRBSP-1])
    nalu->NumBytesInRBSP--;

  // Count trailing bits (including stop bit)
  {
    int r = 0;
    if (nalu->NumBytesInRBSP)
    {
      int v = nalu->rbsp_byte[nalu->NumBytesInRBSP-1];
      for(r=1; r<9; r++)
      {
        if(v&1) break;
        v>>=1;
      }
    }
    nalu->trailing_bits = (uint8_t)r;
  }

  nalu->data_length_in_bits = nalu->NumBytesInRBSP * 8 - nalu->trailing_bits;
  return RET_SUCCESS;
}






#ifndef NDEBUG
// This is only useful for error checking. Should be called only for debug
RetCode parse_rbsp_trailing_bits(nal_unit_t* nalu, BitStreamContext* bs, unsigned int entropy_coding_mode_flag)
{
  if (entropy_coding_mode_flag) // when doing CABAC decoding the rbsp_stop_one_bit is the last bit read before decoding end_of_slice_flag
    bs->index--;                // rewind 1 bit to check that it was indeed equal to 1 (cf H264(03/2005) 9.3.3.2.4, p242)

  int rbsp_stop_one_bit = bs_read_u1(bs);
  LUD_DEBUG_ASSERT(1 == rbsp_stop_one_bit);
  if (bs->index != nalu->data_length_in_bits+1)
  {
    //printf("trailing bits error: used bits: %d, total bits: %d\n", bs->index, nalu->data_length_in_bits+1);
  }
  else
  {
    //int nb_rbsp_alignment_zero_bit = nalu->NumBytesInRBSP*8 - bs->index;
   // int rbsp_alignment_zero_bits = bs_read_un(bs, nb_rbsp_alignment_zero_bit);
  }

  return RET_SUCCESS;
}

RetCode parse_rbsp_slice_trailing_bits(nal_unit_t* nalu, unsigned int entropy_coding_mode_flag)
{
  BitStreamContext* bs = &nalu->bs;
  return parse_rbsp_trailing_bits(nalu, bs, entropy_coding_mode_flag); // All
  // the cabac_zero_word have been removed when parsing the nal unit
  //if( entropy_coding_mode_flag )
  //  while( more_rbsp_trailing_data( ) )
  //    cabac_zero_word /* equal to 0x0000 */ All f(16)
}

#endif

static should_inline unsigned int parse_scaling_list(BitStreamContext* bs, uint8_t* scalingList, int sizeOfScalingList)
{
  int lastScale = 8;
  int nextScale = 8;
  int delta_scale;
  int j;
  unsigned int useDefaultScalingMatrixFlag = 0;
  for( j = 0; j < sizeOfScalingList; j++ )
  {
    if( nextScale != 0 )
    {
      delta_scale = bs_read_se(bs); // 0|1 se(v)
      nextScale = ( lastScale + delta_scale + 256 ) % 256;
      useDefaultScalingMatrixFlag = ( j == 0 && nextScale == 0 );
    }
    scalingList[ j ] = ( nextScale == 0 ) ? lastScale : nextScale;
    lastScale = scalingList[ j ];
  }
  return useDefaultScalingMatrixFlag;
}





void release_sps(seq_parameter_set_rbsp_t* sps)
{
  if (sps)
  {
    if (sps->ref_count != 0)
      sps->ref_count--;
  }
}

static seq_parameter_set_rbsp_t static_sps;
RetCode parse_sps(nal_unit_t* nalu, seq_parameter_set_rbsp_t** sps_)
{
 // RetCode r;
  seq_parameter_set_rbsp_t* sps = &static_sps;

  int i;
  BitStreamContext* bs=&nalu->bs;


  // Allocate seq_parameter_set_rbsp_t structure
  //ALLOC_OR_RET(sps, sizeof(*sps), release_sps(sps));
  *sps_ = sps;
  OBJECT_HEADER_INST(sps);

  // Init dynamic allocation pointer and default values
  sps->vui_parameters = 0;
  sps->offset_for_ref_frame = 0;
  sps->chroma_format_idc = 1;
  sps->residual_colour_transform_flag = 0;
  sps->bit_depth_luma_minus8 = 0;
  sps->bit_depth_chroma_minus8 = 0;
  sps->qpprime_y_zero_transform_bypass_flag = 0;
  sps->seq_scaling_matrix_present_flag = 0;
  sps->mb_adaptive_frame_field_flag = 0;
  sps->frame_crop_left_offset = 0;
  sps->frame_crop_right_offset = 0;
  sps->frame_crop_top_offset = 0;
  sps->frame_crop_bottom_offset = 0;
  sps->delta_pic_order_always_zero_flag = 0;

  // Parse the SPS
  sps->profile_idc = bs_read_un(bs, 8);    // 0 u(8)
  sps->constraint_set0_flag = bs_read_u1(bs);    // 0 u(1)
  sps->constraint_set1_flag = bs_read_u1(bs);    // 0 u(1)
  sps->constraint_set2_flag = bs_read_u1(bs);    // 0 u(1)
  sps->constraint_set3_flag = bs_read_u1(bs);    // 0 u(1)
#ifndef NDEBUG
  int reserved_zero_4bits = bs_read_un(bs, 4);    // /* equal to 0 */ 0 u(4)
  LUD_DEBUG_ASSERT(reserved_zero_4bits == 0);
#else //NDEBUG
  bs_skip_n(bs, 4);
#endif
  sps->level_idc = bs_read_un(bs, 8);    // 0 u(8)
  sps->seq_parameter_set_id = bs_read_ue(bs);    // 0 ue(v)
  if (sps->seq_parameter_set_id>31)
    release_sps(sps);

  if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
      sps->profile_idc == 122 || sps->profile_idc == 144)
  {
    sps->chroma_format_idc = bs_read_ue(bs);    // 0 ue(v)
    if (sps->chroma_format_idc == 3)
      sps->residual_colour_transform_flag = bs_read_u1(bs);    // 0 u(1)
    sps->bit_depth_luma_minus8 = bs_read_ue(bs);    // 0 ue(v)
    sps->bit_depth_chroma_minus8 = bs_read_ue(bs);    // 0 ue(v)
    sps->qpprime_y_zero_transform_bypass_flag = bs_read_u1(bs);    // 0 u(1)
    sps->seq_scaling_matrix_present_flag = bs_read_u1(bs);    // 0 u(1)
    if (sps->seq_scaling_matrix_present_flag)
      for (i = 0; i < 8; i++)
      {
        sps->seq_scaling_list_present_flag[i] = bs_read_u1(bs);    //  0 u(1)
        if (sps->seq_scaling_list_present_flag[i])
        {
          if (i < 6)   // 4x4 lists
          {
            int UseDefaultScalingMatrix4x4Flag;
            UseDefaultScalingMatrix4x4Flag = parse_scaling_list(bs, sps->ScalingList4x4[i], 16);
            if (UseDefaultScalingMatrix4x4Flag)
            {
              memcpy(sps->ScalingList4x4[i], ScalingList_Default_4x4[(i+1)>>2], 16);
            }
          }
          else // 8x8 lists
          {
            int UseDefaultScalingMatrix8x8Flag;
            UseDefaultScalingMatrix8x8Flag = parse_scaling_list(bs, sps->ScalingList8x8[i-6], 64);
            if (UseDefaultScalingMatrix8x8Flag)
            {
              memcpy(sps->ScalingList8x8[i-6], ScalingList_Default_8x8[i-6], 64);
            }
          }
        }
        else // sps->seq_scaling_list_present_flag[i] is false
        {
          switch (i)   // implements table 7.2 Scaling list fall-back rule set A of the standard
          {
            case 0: memcpy(sps->ScalingList4x4[0], ScalingList_Default_4x4[0] , 16); break;
            case 1: memcpy(sps->ScalingList4x4[1], sps->ScalingList4x4[0]     , 16); break;
            case 2: memcpy(sps->ScalingList4x4[2], sps->ScalingList4x4[1]     , 16); break;
            case 3: memcpy(sps->ScalingList4x4[3], ScalingList_Default_4x4[1] , 16); break;
            case 4: memcpy(sps->ScalingList4x4[4], sps->ScalingList4x4[3]     , 16); break;
            case 5: memcpy(sps->ScalingList4x4[5], sps->ScalingList4x4[4]     , 16); break;
            case 6: memcpy(sps->ScalingList8x8[0], ScalingList_Default_8x8[0] , 64); break;
            case 7: memcpy(sps->ScalingList8x8[1], ScalingList_Default_8x8[1] , 64); break;
            default:
              LUD_DEBUG_ASSERT(0);    // cannot happen, i = 0..7
              break;
          }
        }
      }
  }
  sps->log2_max_frame_num_minus4 = bs_read_ue(bs);    // 0 ue(v)
  sps->pic_order_cnt_type = bs_read_ue(bs);    // 0 ue(v)
  if (sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(bs);    // 0 ue(v)
  sps->num_ref_frames = bs_read_ue(bs);    // 0 ue(v)
  sps->gaps_in_frame_num_value_allowed_flag = bs_read_u1(bs);    // 0 u(1)
  sps->pic_width_in_mbs_minus1 = bs_read_ue(bs);    // 0 ue(v)
  sps->pic_height_in_map_units_minus1 = bs_read_ue(bs);    // 0 ue(v)
  sps->frame_mbs_only_flag = bs_read_u1(bs);    // 0 u(1)
  if (!sps->frame_mbs_only_flag)
    sps->mb_adaptive_frame_field_flag = bs_read_u1(bs);    // 0 u(1)
  sps->direct_8x8_inference_flag = bs_read_u1(bs);    // 0 u(1)
  sps->frame_cropping_flag = bs_read_u1(bs);    // 0 u(1)
  if (sps->frame_cropping_flag)
  {
    sps->frame_crop_left_offset = bs_read_ue(bs);    // 0 ue(v)
    sps->frame_crop_right_offset = bs_read_ue(bs);    // 0 ue(v)
    sps->frame_crop_top_offset = bs_read_ue(bs);    // 0 ue(v)
    sps->frame_crop_bottom_offset = bs_read_ue(bs);    // 0 ue(v)
  }
  sps->vui_parameters_present_flag = bs_read_u1(bs);    // 0 u(1)


  LUD_DEBUG_ASSERT(parse_rbsp_trailing_bits(nalu, bs, 0) == RET_SUCCESS);

  // Default inferred variables
  if (!sps->seq_scaling_matrix_present_flag)
  {
    // set Flat_4x4_16 and Flat_8x8_16 into the scaling lists
    memset(sps->ScalingList4x4, 16, 6*16);
    memset(sps->ScalingList8x8, 16, 2*64);
  }

  sps->PicWidthInMbs = sps->pic_width_in_mbs_minus1 + 1;
  sps->PicHeightInMapUnits = sps->pic_height_in_map_units_minus1 + 1;
  sps->FrameHeightInMbs = sps->frame_mbs_only_flag ? sps->PicHeightInMapUnits : 2 * sps->PicHeightInMapUnits;
  sps->PicSizeInMapUnits = sps->PicWidthInMbs * sps->PicHeightInMapUnits;
  sps->BitDepthY = 8 + sps->bit_depth_luma_minus8;
  sps->QpBdOffsetY = 6 * sps->bit_depth_luma_minus8;
  sps->BitDepthC = 8 + sps->bit_depth_chroma_minus8;
  sps->QpBdOffsetC = 6 * ( sps->bit_depth_chroma_minus8 + sps->residual_colour_transform_flag );
  sps->MaxFrameNum = 1 << (sps->log2_max_frame_num_minus4+4);

  return RET_SUCCESS;
}


void release_pps(pic_parameter_set_rbsp_t* pps)
{
  if (pps)
  {
    if (pps->ref_count == 0)
    {
    }
    else
      pps->ref_count--;
  }
}

static pic_parameter_set_rbsp_t static_pps;
RetCode parse_pps(nal_unit_t* nalu, pic_parameter_set_rbsp_t** pps_)
{
  pic_parameter_set_rbsp_t* pps = &static_pps;


  int i;
  BitStreamContext* bs=&nalu->bs;
  seq_parameter_set_rbsp_t* sps;


  // Allocate pic_parameter_set_rbsp_t structure
  //ALLOC_OR_RET(pps, sizeof(*pps), release_pps(pps));
  *pps_ = pps;
  OBJECT_HEADER_INST(pps);

  // init pointers and default values
  pps->run_length_minus1 = 0;
  pps->top_left = 0;
  pps->bottom_right = 0;
  pps->slice_group_id = 0;
  pps->transform_8x8_mode_flag = 0;
  pps->pic_scaling_matrix_present_flag = 0;

  pps->pic_parameter_set_id = bs_read_ue(bs); // 1   ue(v)
  pps->seq_parameter_set_id = bs_read_ue(bs); // 1   ue(v)
  if (pps->seq_parameter_set_id>31) //  || pps->pic_parameter_set_id>255 => always false since it is stored into a byte...
    release_pps(pps);
  sps = gdd.sps[pps->seq_parameter_set_id];
  if (!sps)
    release_pps(pps);
  pps->entropy_coding_mode_flag = bs_read_u1(bs); // 1   u(1)
  pps->pic_order_present_flag = bs_read_u1(bs); // 1   u(1)
  pps->num_slice_groups_minus1 = bs_read_ue(bs); // 1   ue(v)
  LUD_DEBUG_ASSERT(pps->num_slice_groups_minus1 < 8);

  pps->num_ref_idx_active[0] = bs_read_ue(bs)+1; // 1   ue(v)
  if (pps->num_ref_idx_active[0] > NB_OF_REF_PICS)
  {
    LUD_DEBUG_ASSERT(0); // num_ref_idx_active_minus1 overflow
    LUD_TRACE(TRACE_ERROR, "PPS num_ref_idx_active_minus1 L0 overflow, set to max number of ref minus 1\n");
    pps->num_ref_idx_active[0] = NB_OF_REF_PICS;
  }
  pps->num_ref_idx_active[1] = bs_read_ue(bs)+1; // 1   ue(v)
  if (pps->num_ref_idx_active[1] > NB_OF_REF_PICS)
  {
    LUD_DEBUG_ASSERT(0); // num_ref_idx_active_minus1 overflow
    LUD_TRACE(TRACE_ERROR, "PPS num_ref_idx_active_minus1 L0 overflow, set to max number of ref minus 1\n");
    pps->num_ref_idx_active[1] = NB_OF_REF_PICS;
  }
  pps->weighted_pred_flag = bs_read_u1(bs); // 1   u(1)
  pps->weighted_bipred_idc = bs_read_un(bs, 2); // 1   u(2)
  pps->pic_init_qp_minus26 = bs_read_se(bs); // 1   se(v)
  pps->pic_init_qs_minus26 = bs_read_se(bs); // 1   se(v)
  pps->chroma_qp_index_offset = bs_read_se(bs); // 1   se(v)
  pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset; // This is a fallback value in case it is not defined afterwards
  pps->deblocking_filter_control_present_flag = bs_read_u1(bs); // 1   u(1)
  pps->constrained_intra_pred_flag = bs_read_u1(bs); // 1   u(1)
  pps->redundant_pic_cnt_present_flag = bs_read_u1(bs); // 1   u(1)

  if( more_rbsp_data(nalu, bs) )
  {
    pps->transform_8x8_mode_flag = bs_read_u1(bs); // 1   u(1)
    pps->pic_scaling_matrix_present_flag = bs_read_u1(bs); // 1   u(1)
    if( pps->pic_scaling_matrix_present_flag )
      for( i = 0; i < 6 + 2 * pps->transform_8x8_mode_flag; i++ )
      {
        pps->pic_scaling_list_present_flag[i] = bs_read_u1(bs); // 1   u(1)
        if( pps->pic_scaling_list_present_flag[i] )
        {
          if( i < 6 ) // 4x4 lists
          {
            unsigned int UseDefaultScalingMatrix4x4Flag;
            UseDefaultScalingMatrix4x4Flag = parse_scaling_list(bs, pps->ScalingList4x4[i], 16);
            if (UseDefaultScalingMatrix4x4Flag)
            {
              memcpy(pps->ScalingList4x4[i], ScalingList_Default_4x4[(i+1)>>2], 16);
            }
          }
          else  // 8x8 lists
          {
            unsigned int UseDefaultScalingMatrix8x8Flag;
            UseDefaultScalingMatrix8x8Flag = parse_scaling_list(bs, pps->ScalingList8x8[i-6], 64);
            if (UseDefaultScalingMatrix8x8Flag)
            {
              memcpy(pps->ScalingList8x8[i-6], ScalingList_Default_8x8[i-6], 64);
            }
          }
        }
        else // pps->UseDefaultScalingMatrix4x4Flag[i] is false
        {
          if(sps->seq_scaling_matrix_present_flag)
          {
            switch (i)   // implements table 7.2 Scaling list fall-back rule set B of the standard
            {
              case 0: memcpy(pps->ScalingList4x4[0], sps->ScalingList4x4[0]     , 16); break;
              case 1: memcpy(pps->ScalingList4x4[1], pps->ScalingList4x4[0]     , 16); break;
              case 2: memcpy(pps->ScalingList4x4[2], pps->ScalingList4x4[1]     , 16); break;
              case 3: memcpy(pps->ScalingList4x4[3], sps->ScalingList4x4[3]     , 16); break;
              case 4: memcpy(pps->ScalingList4x4[4], pps->ScalingList4x4[3]     , 16); break;
              case 5: memcpy(pps->ScalingList4x4[5], pps->ScalingList4x4[4]     , 16); break;
              case 6: memcpy(pps->ScalingList8x8[0], sps->ScalingList8x8[0]     , 64); break;
              case 7: memcpy(pps->ScalingList8x8[1], sps->ScalingList8x8[1]     , 64); break;
              default:
                LUD_DEBUG_ASSERT(0);    // cannot happen, i = 0..7
                break;
            }
          }
          else
          {
            switch (i)   // implements table 7.2 Scaling list fall-back rule set A of the standard
            {
              case 0: memcpy(pps->ScalingList4x4[0], ScalingList_Default_4x4[0] , 16); break;
              case 1: memcpy(pps->ScalingList4x4[1], pps->ScalingList4x4[0]     , 16); break;
              case 2: memcpy(pps->ScalingList4x4[2], pps->ScalingList4x4[1]     , 16); break;
              case 3: memcpy(pps->ScalingList4x4[3], ScalingList_Default_4x4[1] , 16); break;
              case 4: memcpy(pps->ScalingList4x4[4], pps->ScalingList4x4[3]     , 16); break;
              case 5: memcpy(pps->ScalingList4x4[5], pps->ScalingList4x4[4]     , 16); break;
              case 6: memcpy(pps->ScalingList8x8[0], ScalingList_Default_8x8[0] , 64); break;
              case 7: memcpy(pps->ScalingList8x8[1], ScalingList_Default_8x8[1] , 64); break;
              default:
                LUD_DEBUG_ASSERT(0);    // cannot happen, i = 0..7
                break;
            }
          }
        }
      } //for
      pps->second_chroma_qp_index_offset = bs_read_se(bs); // 1 se(v)
  }

  LUD_DEBUG_ASSERT(parse_rbsp_trailing_bits(nalu, bs, 0) == RET_SUCCESS);

  // Fills in dome default values
  if (!pps->pic_scaling_matrix_present_flag)
  {
    // copy the scaling lists from the sps
    memcpy(pps->ScalingList4x4, sps->ScalingList4x4, 6*16);
    memcpy(pps->ScalingList8x8, sps->ScalingList8x8, 2*64);

  }

  {
    int l, i, j, k, m;
    pps->SliceGroupChangeRate = pps->slice_group_change_rate_minus1 + 1;

    // Derive LevelScale(m, i, j)
    for( l=0; l<6; l++)
    {
      uint8_t* weightScale = pps->ScalingList4x4[l];
      for( m=0; m<6; m++)
      {
        uint16_t* LevelScale4x4 = pps->LevelScale4x4[l][m];

        for(k=0; k<16; k++)
        {
          int idx = inverse_4x4zigzag_scan[k];
          i = idx >> 2;
          j = idx & 3;
          LevelScale4x4[idx] = weightScale[k] * normAdjust4x4[m][(i&1)+(j&1)];
        }
      }
    }
    // Derive LevelScale8x8(m, i, j)
    for( l=0; l<2; l++)
    {
      uint8_t* weightScale = pps->ScalingList8x8[l];
      for( m=0; m<6; m++)
      {
        uint16_t* LevelScale8x8 = pps->LevelScale8x8[l][m];

        for(k=0; k<64; k++)
        {
          int idx = inverse_8x8zigzag_scan[k];
          i = idx >> 3;
          j = idx & 7;
          LevelScale8x8[idx] = weightScale[k] * normAdjust8x8[m][(i&3)+(j&3)*4];
        }
      }
    }
  }

  return RET_SUCCESS;
}

// this parsing does not do allocation. Caller must allocate the structure
static should_inline RetCode parse_ref_pic_list_reordering(slice_header_t* sh, BitStreamContext* bs, ref_pic_list_reordering_t* rplr)
{
  rplr->pic_list_reordering_commands[0] = NULL;
  rplr->pic_list_reordering_commands[1] = NULL;

  return RET_SUCCESS;
}


#define ALIGN(p, a) (((unsigned long)(p) + (a) -1) & ~((a) -1))


static dec_ref_pic_marking_t static_drpm;
static should_inline RetCode parse_dec_ref_pic_marking(nal_unit_t* nalu, slice_header_t* sh, BitStreamContext* bs, dec_ref_pic_marking_t** drpm_)
{
  dec_ref_pic_marking_t* drpm = &static_drpm;

  uint8_t memory_management_control_operation;

  // Allocate dec_ref_pic_marking_t structure
  //ALLOC_OR_RET(drpm, sizeof(*drpm), free_dec_ref_pic_marking(drpm));
  *drpm_ = drpm;

  drpm->nb_of_mmco_cmd = 0;

  if( nalu->nal_unit_type == 5 )
  {
    drpm->no_output_of_prior_pics_flag = bs_read_u1(bs); // 2|5 u(1)
    drpm->long_term_reference_flag = bs_read_u1(bs); // 2|5 u(1)
    drpm->adaptive_ref_pic_marking_mode_flag = 0; // default value
  }
  else
  {
    drpm->no_output_of_prior_pics_flag = 0; // default value
    drpm->long_term_reference_flag = 0;     // default value
    drpm->adaptive_ref_pic_marking_mode_flag = bs_read_u1(bs); // 2|5 u(1)
    if( drpm->adaptive_ref_pic_marking_mode_flag )
    {
      int idx = 0;
      uint32_t* commands = drpm->mmco_commands;
      do
      {
        memory_management_control_operation = bs_read_ue(bs); // 2|5 ue(v)

        switch (memory_management_control_operation)
        {
          case 4: // 2|5 ue(v) max_long_term_frame_idx_plus1, bits 0..28
          case 6: // 2|5 ue(v) long_term_frame_idx, bits 0..28
          case 1: // 2|5 ue(v) difference_of_pic_nums_minus1 bits, 0..2
          case 2: // 2|5 ue(v) long_term_pic_num, bits 0..28
            commands[idx] = bs_read_ue(bs);
            break;
          case 3:
            commands[idx] = bs_read_ue(bs); // 2|5 ue(v) difference_of_pic_nums_minus1, bits 0..22 (23 bits is enough)
            commands[idx] |= bs_read_ue(bs) << 23; // 2|5 ue(v) long_term_frame_idx, bits 23..28 (6 bits is enough)
            break;
          case 5:
            sh->has_mmco5 = 1;
          case 0:
            commands[idx] = 0;
            break;
          default:
            LUD_DEBUG_ASSERT(0); // unknown command !!!
            break;
        }

        commands[idx] |= memory_management_control_operation << 29; // the command is stored into the bits 29..31 (3 bits is enough for 7 commands from 0 to 6)

        idx++;
      } while (memory_management_control_operation != 0 && idx<MAX_NB_OF_MMCO_COMMANDS);

      LUD_DEBUG_ASSERT(idx < MAX_NB_OF_MMCO_COMMANDS); // there are move mmco commands than we can store ?
      if (idx == MAX_NB_OF_MMCO_COMMANDS)
        commands[--idx] = 0; // close the command list (prevent overflow in subsequent code...)

      drpm->nb_of_mmco_cmd = idx;
    }
  }
  return RET_SUCCESS;
}

void release_slice_header(slice_header_t* sh)
{
  if (sh)
  {
    if (sh->ref_count != 0)
      sh->ref_count--;
  }
}

static slice_header_t static_sh;
RetCode parse_slice_header(nal_unit_t* nalu, slice_header_t** sh_)
{
  RetCode r;
  seq_parameter_set_rbsp_t* sps;
  pic_parameter_set_rbsp_t* pps;
  BitStreamContext* bs = &nalu->bs;
  slice_header_t* sh = &static_sh;


  // Allocate slice header structure
  //ALLOC_OR_RET(sh, sizeof(*sh), release_slice_header(sh));
  *sh_ = sh;
  OBJECT_HEADER_INST(sh);

  // Default values
  sh->pred_weight_table = 0;
  sh->field_pic_flag = 0;
  sh->bottom_field_flag = 0;
  sh->delta_pic_order_cnt[ 0 ] = sh->delta_pic_order_cnt[ 1 ] = 0;
  sh->delta_pic_order_cnt_bottom = 0;
  sh->has_mmco5 = 0;
  sh->disable_deblocking_filter_idc = 0;
  sh->slice_alpha_c0_offset_div2 = 0;
  sh->slice_beta_offset_div2 = 0;
  sh->MbToSliceGroupMap = 0;



  sh->first_mb_in_slice = bs_read_ue(bs); // 2    ue(v)
  sh->slice_type = bs_read_ue(bs); // 2    ue(v)
  sh->slice_type_modulo5 = sh->slice_type < 5 ? sh->slice_type : sh->slice_type - 5;
  sh->pic_parameter_set_id = bs_read_ue(bs); // 2    ue(v)
  pps = gdd.pps[sh->pic_parameter_set_id];
  if (!pps)
    return ERR_REFERING_NON_EXISTING_PPS;
  sps = gdd.sps[pps->seq_parameter_set_id];
  if (!sps)
    return ERR_REFERING_NON_EXISTING_SPS;
  sh->frame_num = bs_read_un(bs, sps->log2_max_frame_num_minus4+4 ); // 2    u(v)
  sh->original_frame_num = sh->frame_num;
  if( !sps->frame_mbs_only_flag )
  {
    sh->field_pic_flag = bs_read_u1(bs); // 2    u(1)
    if( sh->field_pic_flag )
      sh->bottom_field_flag = bs_read_u1(bs); // 2    u(1)
  }
  if( nalu->nal_unit_type == 5 )
    sh->idr_pic_id = bs_read_ue(bs); // 2    ue(v)
  if( sps->pic_order_cnt_type == 0 )
  {
    sh->pic_order_cnt_lsb = bs_read_un(bs, sps->log2_max_pic_order_cnt_lsb_minus4+4); // 2    u(v)
    if( pps->pic_order_present_flag && !sh->field_pic_flag )
      sh->delta_pic_order_cnt_bottom = bs_read_se(bs); // 2    se(v)
  }
  if( sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag )
  {
    sh->delta_pic_order_cnt[ 0 ] = bs_read_se(bs); // 2    se(v)
    if( pps->pic_order_present_flag && !sh->field_pic_flag )
      sh->delta_pic_order_cnt[ 1 ] = bs_read_se(bs); // 2    se(v)
  }
  if( pps->redundant_pic_cnt_present_flag )
    sh->redundant_pic_cnt = bs_read_ue(bs); // 2    ue(v)
  if( sh->slice_type_modulo5 == B_SLICE )
    sh->direct_spatial_mv_pred_flag = bs_read_u1(bs); // 2    u(1)
  sh->num_ref_idx_active[0] = pps->num_ref_idx_active[0]; // default value
  sh->num_ref_idx_active[1] = pps->num_ref_idx_active[1]; // default value
  if( sh->slice_type_modulo5 == P_SLICE || sh->slice_type_modulo5 == SP_SLICE || sh->slice_type_modulo5 == B_SLICE )
  {
    sh->num_ref_idx_active_override_flag = bs_read_u1(bs); // 2    u(1)
    if( sh->num_ref_idx_active_override_flag )
    {
      sh->num_ref_idx_active[0] = bs_read_ue(bs)+1; // 2    ue(v)
      if (sh->num_ref_idx_active[0] > NB_OF_REF_PICS)
      {
        LUD_DEBUG_ASSERT(0); // num_ref_idx_active_minus1 overflow
        LUD_TRACE(TRACE_ERROR, "num_ref_idx_active_minus1 L0 overflow, set to max number of ref minus 1\n");
        sh->num_ref_idx_active[0] = NB_OF_REF_PICS;
      }
      if( sh->slice_type_modulo5 == B_SLICE )
      {
        sh->num_ref_idx_active[1] = bs_read_ue(bs)+1; // 2    ue(v)
        if (sh->num_ref_idx_active[1] > NB_OF_REF_PICS)
        {
          LUD_DEBUG_ASSERT(0); // num_ref_idx_active_minus1 overflow
          LUD_TRACE(TRACE_ERROR, "num_ref_idx_active_minus1 L1 overflow, set to max number of ref minus 1\n");
          sh->num_ref_idx_active[1] = NB_OF_REF_PICS;
        }
      }
    }
  }

  // the structure itself is allocated into the slice header structure ! However the function allocates tables...
  if (RET_SUCCESS != (r = parse_ref_pic_list_reordering(sh, bs, &sh->ref_pic_list_reordering)))
    release_slice_header(sh);


  if( nalu->nal_ref_idc != 0 )
  {
    if (RET_SUCCESS != (r = parse_dec_ref_pic_marking(nalu, sh, bs, &sh->dec_ref_pic_marking)))
      release_slice_header(sh);
  }
  else
    sh->dec_ref_pic_marking = 0;

  if( pps->entropy_coding_mode_flag && sh->slice_type_modulo5 != I_SLICE && sh->slice_type_modulo5 != SI_SLICE )
    sh->cabac_init_idc = bs_read_ue(bs); // 2    ue(v)
  sh->slice_qp_delta = bs_read_se(bs); // 2    se(v)
  if( sh->slice_type_modulo5 == SP_SLICE || sh->slice_type_modulo5 == SI_SLICE )
  {
    if( sh->slice_type_modulo5 == SP_SLICE )
      sh->sp_for_switch_flag = bs_read_u1(bs); // 2    u(1)
    sh->slice_qs_delta = bs_read_se(bs); // 2 se(v)
  }
  if( pps->deblocking_filter_control_present_flag )
  {
    sh->disable_deblocking_filter_idc = bs_read_ue(bs); // 2 ue(v)
    if( sh->disable_deblocking_filter_idc != 1 )
    {
      sh->slice_alpha_c0_offset_div2 = bs_read_se(bs); // 2 se(v)
      sh->slice_beta_offset_div2 = bs_read_se(bs); // 2 se(v)
    }
  }
  if( pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
    sh->slice_group_change_cycle = bs_read_un(bs, im_ceillog2( sps->PicSizeInMapUnits / pps->SliceGroupChangeRate + 1 )); // 2 u(v)

  // compute derived variables
  {
    sh->pps = pps;
    use_object(pps);
    sh->sps = sps;
    use_object(sps);
    sh->MbaffFrameFlag = ( sps->mb_adaptive_frame_field_flag && !sh->field_pic_flag );
    sh->PicHeightInMbs = sps->FrameHeightInMbs >> sh->field_pic_flag;
    sh->PicSizeInMbs = sps->PicWidthInMbs * sh->PicHeightInMbs;
    if (sh->field_pic_flag)
    {
      sh->MaxPicNum =  sps->MaxFrameNum * 2;
      sh->CurrPicNum = sh->frame_num * 2 + 1;
    }
    else
    {
      sh->MaxPicNum =  sps->MaxFrameNum;
      sh->CurrPicNum = sh->frame_num;
    }
    sh->SliceQPY = 26 + pps->pic_init_qp_minus26 + sh->slice_qp_delta;

    // TODO FIXME remove non necessary variables from the struct!:  slice_alpha_c0_offset_div2 slice_beta_offset_div2
    sh->FilterOffsetA = sh->slice_alpha_c0_offset_div2 << 1;
    sh->FilterOffsetB = sh->slice_beta_offset_div2 << 1;

    sh->nal_ref_idc = nalu->nal_ref_idc;
    sh->nal_unit_type = nalu->nal_unit_type;
    sh->RefPicList[0] = sh->RefPicList[1] = 0;
  }

  return RET_SUCCESS;
}




