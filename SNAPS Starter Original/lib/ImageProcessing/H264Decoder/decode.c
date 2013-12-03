#include "syntax.h"
#include "decode.h"
#include <string.h>
#include "defaulttables.h"
#include "bitstream.h"
#include "decode_slice.h"
#include "system.h"
#include "cabac.h"

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
PictureDecoderData pdd;

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
GlobalDecoderData gdd;


static should_inline unsigned int is_new_picture(slice_header_t* prev_sh,
                                          slice_header_t* curr_sh, seq_parameter_set_rbsp_t* curr_sps)
{
  seq_parameter_set_rbsp_t* prev_sps = prev_sh->sps;

  if (!prev_sps)
    return 1;

  return 0;
}


// Invoked when a new picture is detected;
static should_inline void start_picture(PictureDecoderData* pdd, slice_header_t* sh, seq_parameter_set_rbsp_t* sps, pic_parameter_set_rbsp_t* pps)
{
  LUD_DEBUG_ASSERT(!pdd->pic_not_finished);
  pdd->pic_not_finished = 1;

  // Start to assign picture number (in decoding order) to the current picture
  pdd->dec_num++;

  // Detect if this is the second field of a complementary field pair, the process is different if this is a reference picture or not
  pdd->is_second_field_of_a_pair = 0;


  //Picture* prev_pic = pdd->pic;
  if (!pdd->is_second_field_of_a_pair)
  {
    // Allocate memory for that picture
    pdd->pic = alloc_picture(pdd, sh);
  }

  pdd->pic->dec_num[sh->bottom_field_flag] = pdd->dec_num;

}

RetCode decode_slice(PictureDecoderData* pdd, nal_unit_t* nalu)
{
  RetCode r;
  slice_header_t* sh;
  seq_parameter_set_rbsp_t* sps;
  pic_parameter_set_rbsp_t* pps;
  Picture* pic;
  unsigned int bottom_field_flag;

  if (RET_SUCCESS != (r=parse_slice_header(nalu, &sh)))
    return r;

  sps = sh->sps;
  pps = sh->pps;
  bottom_field_flag = sh->bottom_field_flag;

  if (is_new_picture(pdd->prev_sh, sh, sps))
  {
    // Execute actions for starting a new picture decoding
    start_picture(pdd, sh, sps, pps);
    pic = pdd->pic;
  }

  pic->field_sh[pic->slice_num] = sh;

  // Store the slice header to compare it with the next slice
  pdd->prev_sh = sh;

  unsigned int entropy_coding_mode_flag = pps->entropy_coding_mode_flag;
  if (RET_SUCCESS != (r=decode_slice_data(pdd, nalu, sh, sps, pps, entropy_coding_mode_flag)))
    return r;

  LUD_DEBUG_ASSERT(RET_SUCCESS == parse_rbsp_slice_trailing_bits(nalu, entropy_coding_mode_flag));

  return RET_SUCCESS;
}



RetCode decoder_init()
{

  // Set default values of gdd and pdd when necessary
  memset(&pdd, 0, sizeof(pdd));
//  memset(&gdd, 0, sizeof(gdd));

  static slice_header_t dummy_sh; // a all 0-filled structure that will prevent from testing if prev_sh pointer is non null each time it is accessed !

  pdd.max_num_of_slices = 1;
  pdd.decoding_finished = 0;

  memset(&dummy_sh, 0, sizeof(dummy_sh)); // all 0 should make a good "before-first" slice
  pdd.prev_sh = &dummy_sh; // needed so that there is no need to test if the pointer is non null

  // Init CABAC tables
  cabac_init_data();

  init_dpb();

  return RET_SUCCESS;
}


// Decode the next nal unit from the buffer. The buffer must start with a start code. length is: in(input buffer size)/out(consumed bytes into the input buffer)
RetCode decode_nalu(uint8_t* data, uint32_t* length)
{
  nal_unit_t nalu;
  RetCode r;
  if (!data) // the caller mean the end of file(/stream), so we can finish and tide up the current decoding session
  {
 //   end_decoding(&pdd);
    return RET_SUCCESS;
  }

  r = parse_nal_unit(data, length, &nalu, 0);

  if (r != RET_SUCCESS)
  {
    LUD_DEBUG_ASSERT(0);
    return r;
  }

  switch (nalu.nal_unit_type)
  {
    case NALU_TYPE_IDR:
    {
      LUD_TRACE(TRACE_INFO, "Parsing an IDR\n");
      r = decode_slice(&pdd, &nalu);
      LUD_DEBUG_ASSERT(RET_SUCCESS == r);
      break;
    }
    case NALU_TYPE_SPS:
    {
      // Finish the previous pic decoding
     // end_picture(&pdd);
      LUD_TRACE(TRACE_INFO, "Parsing SPS\n");
      seq_parameter_set_rbsp_t* sps;
      r = parse_sps(&nalu, &sps);
      if (RET_SUCCESS != r)
      {
        LUD_TRACE(TRACE_ERROR, "Error %d while parsing SPS, nalu dropped...\n", r);
        break;
      }
      if (gdd.sps[sps->seq_parameter_set_id]!=0)
        release_sps(gdd.sps[sps->seq_parameter_set_id]);
      gdd.sps[sps->seq_parameter_set_id] = sps;
      break;
    }
    case NALU_TYPE_PPS:
    {
      // Finish the previous pic decoding
//      end_picture(&pdd);
      LUD_TRACE(TRACE_INFO, "Parsing PPS\n");
      pic_parameter_set_rbsp_t* pps;
      r = parse_pps(&nalu, &pps);
      if (RET_SUCCESS != r)
      {
        LUD_TRACE(TRACE_ERROR, "Error %d while parsing PPS, nalu dropped...\n", r);
        break;
      }
      if (gdd.pps[pps->pic_parameter_set_id]!=0)
        release_pps(gdd.pps[pps->pic_parameter_set_id]);
      gdd.pps[pps->pic_parameter_set_id] = pps;
      break;
    }
    default:
      LUD_TRACE(TRACE_INFO, "NALU type %d is unknown\n", nalu.nal_unit_type);
      LUD_DEBUG_ASSERT(0); // Unknown NAL Unit type !
      break;
  }

  return RET_SUCCESS;
}

