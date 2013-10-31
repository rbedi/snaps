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

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__


#include "intmath.h"
#include "bitstream_types.h"
#include "syntax_types.h"


static const uint8_t bs_exp_golomb_len[512] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const uint8_t bs_exp_golomb_ue[512] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
  7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int8_t bs_exp_golomb_se[512] =
{
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,
  4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};


static const uint8_t bs_exp_golomb_me_intra[48+16] =
{
  // chroma_format_idc != 0
  47, 31, 15,  0, 23, 27, 29, 30, 7, 11, 13, 14, 39, 43, 45, 46, 16, 3, 5, 10, 12, 19, 21, 26, 28, 35, 37, 42, 44, 1, 2, 4, 8, 17, 18, 20, 24, 6, 9, 22, 25, 32, 33, 34, 36, 40, 38, 41,
  // chroma_format_idc == 0
  15, 0, 7, 11, 13, 14, 3, 5, 10, 12, 1, 2, 4, 8, 6, 9
};


static const uint8_t bs_exp_golomb_me_inter[48+16] =
{
  // chroma_format_idc != 0
  0, 16, 1, 2, 4, 8, 32, 3, 5, 10, 12, 15, 47, 7, 11, 13, 14, 6, 9, 31, 35, 37, 42, 44, 33, 34, 36, 40, 39, 43, 45, 46, 17, 18, 20, 24, 19, 21, 26, 28, 23, 27, 29, 30, 22, 25, 38, 41,
  // chroma_format_idc == 0
  0, 1, 2, 4, 8, 3, 5, 10, 12, 15, 7, 11, 13, 14, 6, 9
};




// Initialize the bitstream context
static should_inline void init_bitstream(BitStreamContext *bs, uint8_t* data)
{

  // The CPU need to support UN-aligned address access (not 32bits aligned but 8bit aligned actually)
  bs->buffer = data;
  bs->index = 0; // we have read 32 bits and put them into the cache

}

// get the next (unsigned!) bit from the bitstream
static should_inline uint32_t bs_read_u1(BitStreamContext *bs)
{
    uint8_t cache;
    cache = bs->buffer[bs->index>>3] << (bs->index & 0x7);
    bs->index++;
    return (uint32_t)(cache >> (8-1));
}


// get unsigned bits: from 0 to 32 bits. It could be really really easily expended to 64-7=57 bit (basically : change the return type and remove the 32bit cast)
static should_inline uint32_t bs_read_un(BitStreamContext *bs, int n)
{

  LUD_DEBUG_ASSERT(n<=32);

  if (n == 0)
    return 0;

#ifndef ARCH_X86_64
  // This is just for optimization on non 64 bits CPU. The loss of the if jump (break pipeline) is compensated by the gain
  //   of manipulating 32 bits integers instead of 64 bits integers.
  // On 64 bits CPU there is no reason to keep this if statement.
  if (n<26)
  {
    uint32_t cache;
    cache = bswap_32(*(uint32_t*)((uint8_t*)bs->buffer+(bs->index>>3))) << ((bs->index & 0x7));
    bs->index+=n;
    return (uint32_t)(cache>>(32-n));
  }
  else
#endif
  {
    uint32_t a = ((uint32_t*)(bs->buffer+(bs->index>>3)))[0];
    uint32_t b = ((uint32_t*)(bs->buffer+(bs->index>>3)))[1];

    uint64_t cache = ((uint64_t)b << 32) | a;
    cache = bswap_64(cache) << ((bs->index & 0x7));
    bs->index+=n;
    return (uint32_t)(cache>>(64-n));
  }

}

// show the next unsigned bits (from 0 to 32 bits). It does not change the bitstream reader pointer.
static should_inline uint32_t bs_show_un(BitStreamContext *bs, int n)
{
  LUD_DEBUG_ASSERT(n<=32);

  if (n == 0)
    return 0;

#ifndef ARCH_X86_64
  // This is just for optimization on non 64 bits CPU. The loss of the if jump (break pipeline) is compensated by the gain
  //   of manipulating 32 bits integers instead of 64 bits integers.
  // On 64 bits CPU there is no reason to keep this if statement.
  if (n<26)
  {
    uint32_t cache;
    cache = bswap_32(*(uint32_t*)(bs->buffer+(bs->index>>3))) << ((bs->index & 0x7));
    return (uint32_t)(cache>>(32-n));
  }
  else
#endif
  {
    uint32_t a = ((uint32_t*)(bs->buffer+(bs->index>>3)))[0];
    uint32_t b = ((uint32_t*)(bs->buffer+(bs->index>>3)))[1];

    uint64_t cache = ((uint64_t)b << 32) | a;
    cache = bswap_64(cache) << ((bs->index & 0x7));
    return (uint32_t)(cache>>(64-n));
  }

}


// move the reader pointer of the bitstream, it can move it of an arbitrary amount of bit, provided you do not overflow of course...
static should_inline void bs_skip_n(BitStreamContext *bs, int n)
{
  bs->index += n;
}

// Get the next Exp-Golomb unsigned number
static inline uint32_t bs_read_ue(BitStreamContext *bs)
{
  uint32_t buf;
  int clen;

  buf = bs_show_un(bs, 32); // TODO FIXME: check what is the max length of an exp Golomb code. The less the better so we have to fetch minimum amount of bits by default

  if(buf >= (1<<27)) // If this condition is met it means we have a 1 into the 5 higher bits => exp-golomb length is less or equal to 9...
  {
    buf >>= 32 - 9;
    bs_skip_n(bs, bs_exp_golomb_len[buf]);
    return bs_exp_golomb_ue[buf];
  }
  else
  {
    clen= 2*im_log2(buf) - 31; // give the complementary of the length : 32-length.
    buf >>= clen;
    buf--;
    bs_skip_n(bs, 32 - clen);
    return buf;
  }
}

// Get the next Exp-Golomb signed number
static inline int32_t bs_read_se(BitStreamContext *bs)
{
  uint32_t buf;
  int clen;

  buf = bs_show_un(bs, 32); // TODO FIXME: check what is the max length of an exp Golomb code. The less the better so we have to fetch minimum amount of bits by default

  if(buf >= (1<<27)) // If this condition is met it means we have a 1 into the 5 higher bits => exp-golomb length is less or equal to 9...
  {
    buf >>= 32 - 9;
    bs_skip_n(bs, bs_exp_golomb_len[buf]);
    return bs_exp_golomb_se[buf];
  }
  else
  {
    LUD_DEBUG_ASSERT(buf > (1<<15));  // check that the Exp-Golomb code fit into a 32 bit data.
    clen= 2*im_log2(buf) - 31; // give the complementary of the length : 32-length.
    buf >>= clen;
    bs_skip_n(bs, 32 - clen);
    if (buf&1) buf = -(buf>>1);
    else       buf =  (buf>>1);
    return buf;
  }
}

// Get the next Exp-Golomb coded block number (see MPEG4 AVC chap 9.1.2 for more info)
static should_inline uint8_t bs_read_me(BitStreamContext *bs, PREDICTION_MODE prediction_mode, unsigned int chroma_format_idc)
{
  uint32_t i = bs_read_ue(bs);
  LUD_DEBUG_ASSERT((chroma_format_idc != 0 && i < 48) || (chroma_format_idc == 0 && i < 16));

  i += (chroma_format_idc == 0 ) ? 48 : 0;

  if (prediction_mode <= Intra_8x8)
    return bs_exp_golomb_me_intra[i];
  else
    return bs_exp_golomb_me_inter[i];
}

static should_inline uint32_t bs_read_te(BitStreamContext *bs, unsigned int range)
{
  LUD_DEBUG_ASSERT(range >= 1);

    if(range==1) return bs_read_u1(bs)^1;
    else         return bs_read_ue(bs);
}


int init_vlc(VLCReader* vlc, unsigned int nb_of_symbols, const uint8_t* symbol_table, const uint8_t* code_table, const uint8_t* length_table, unsigned int nb_of_bits);
int destroy_vlc(VLCReader* vlc);

static inline uint8_t bs_read_vlc(BitStreamContext *bs, VLCReader* vlc)
{
  unsigned int buf;
  unsigned int symbol;
  int length;

  buf = bs_show_un(bs, vlc->bits);
  symbol  = vlc->table.code_symbol[buf];
  length  = vlc->table.code_length[buf];
  if (length < 0)
  {
    uint8_t*  st;
    int8_t*   lt;
    LUD_DEBUG_ASSERT(symbol < vlc->nb_of_subtables); // otherwise we are going to read garbage (overflow)
    st = vlc->subtable[symbol].code_symbol;
    lt = vlc->subtable[symbol].code_length;
    bs_skip_n(bs, vlc->bits);
    buf = bs_show_un(bs, -length);
    symbol  = st[buf];
    length  = lt[buf];
  }
  LUD_DEBUG_ASSERT(length>0); // we only allow one sub-table jump ! (i.e. a vlc code of maximal length 2*vlc->bits)
  bs_skip_n(bs, length);

  return symbol;
}



#endif //__BITSTREAM_H__
