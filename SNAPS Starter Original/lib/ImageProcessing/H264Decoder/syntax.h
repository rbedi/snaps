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


#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "syntax_types.h"
#include "bitstream_types.h"


/* inline functions */

static should_inline int more_rbsp_data(nal_unit_t* nalu, BitStreamContext *bs)
{
  return nalu->data_length_in_bits > bs->index;
}

#include "decode.h"


// Parsing function definition
RetCode parse_nal_unit(uint8_t* stream_buffer, uint32_t* stream_available_consumed, nal_unit_t* nalu, unsigned int preparsed);
RetCode parse_sps(nal_unit_t* nalu, seq_parameter_set_rbsp_t** sps);
void release_sps(seq_parameter_set_rbsp_t* sps);
RetCode parse_pps(nal_unit_t* nalu, pic_parameter_set_rbsp_t** pps);
void release_pps(pic_parameter_set_rbsp_t* pps);
RetCode parse_slice_header(nal_unit_t* nalu, slice_header_t** sh);
void release_slice_header(slice_header_t* sh);

#ifndef NDEBUG
RetCode parse_rbsp_slice_trailing_bits(nal_unit_t* nalu, unsigned int entropy_coding_mode_flag);
RetCode parse_rbsp_trailing_bits(nal_unit_t* nalu, BitStreamContext* bs, unsigned int entropy_coding_mode_flag);
#else // NDEBUG
#define parse_rbsp_slice_trailing_bits(nalu, entropy_coding_mode_flag)
#define parse_rbsp_trailing_bits(nalu, bs, entropy_coding_mode_flag)
#endif // NDEBUG

#endif // __SYNTAX_H__

