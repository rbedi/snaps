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

#define PRED4x4_LOAD_LEFT_TOP_SAMPLE\
    const int lt= dst[-1-1*dy];

#define PRED4x4_LOAD_TOP_RIGHT_EDGE   \
    int unused_param t4, t5, t6, t7;  \
    if (topright_is_avail)            \
{                                 \
      t4 = dst[-dy+4];                \
      t5 = dst[-dy+5];                \
      t6 = dst[-dy+6];                \
      t7 = dst[-dy+7];                \
}                                 \
    else                              \
      t4 = t5 = t6 = t7 = dst[-dy+3];

#define PRED4x4_LOAD_LEFT_EDGE\
    const int unused_param l0= dst[-1+0*dy];\
    const int unused_param l1= dst[-1+1*dy];\
    const int unused_param l2= dst[-1+2*dy];\
    const int unused_param l3= dst[-1+3*dy];

#define PRED4x4_LOAD_TOP_EDGE\
    const int unused_param t0= dst[ 0-1*dy];\
    const int unused_param t1= dst[ 1-1*dy];\
    const int unused_param t2= dst[ 2-1*dy];\
    const int unused_param t3= dst[ 3-1*dy];


static should_inline unsigned int Intra16x16PredMode(MB_TYPE mb_type)
{
  LUD_DEBUG_ASSERT(mb_type >= I_16x16_0_0_0 && mb_type <= I_16x16_3_2_1);
  return Intra16x16PredMode_NumMbPart_array[mb_type];
}

/*
 * Intrat 4x4 prediction
 */

static should_inline void copy4samples(pixel_t* dst, const pixel_t* src)
{
#ifdef PIXEL_T_IS_8_BITS
  LUD_DEBUG_ASSERT(((unsigned long)dst&3) == 0 && ((unsigned long)src&3) == 0);
  *(uint32_t*)dst = *(uint32_t*)src; // could be unaligned 32/64 bits copy
#else // => PIXEL_T_IS_16_BITS
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0 && ((unsigned long)src&7) == 0);
  *(uint64_t*)dst = *(uint64_t*)src; // could be unaligned 32/64 bits copy
#endif
}
static should_inline void duplicate1to4samples(pixel_t* dst, const pixel_t v)
{
#ifdef PIXEL_T_IS_8_BITS
  LUD_DEBUG_ASSERT(((unsigned long)dst&3) == 0);
  *(uint32_t*)dst = (uint32_t)v * (uint32_t)0x01010101;
#else // => PIXEL_T_IS_16_BITS
# ifndef ARCH_X86_64
  LUD_DEBUG_ASSERT(((unsigned long)dst&3) == 0);
  ((uint32_t*)dst)[0] = ((uint32_t*)dst)[1] =  v * 0x00010001; // 64bit multiply is not a good idea on 32 bits architecture
# else
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0);
  *(uint64_t*)dst = (uint64_t)v * (uint64_t)0x0001000100010001;
# endif
#endif
}

/*
 * Intra 8x8 prediction
 */

#define DST(x,y) dst[(x)+(y)*dy]
#define SRC(x,y) src[(x)+(y)*dy]
#define PL(y)    const int l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;
#define PRED8x8_LOAD_LEFT_EDGE                                          \
    const int l0 = ((topleft_is_avail ? SRC(-1,-1) : SRC(-1,0))         \
                     + 2*SRC(-1,0) + SRC(-1,1) + 2) >> 2;               \
    PL(1) PL(2) PL(3) PL(4) PL(5) PL(6)                                 \
    const int l7 = (SRC(-1,6) + 3*SRC(-1,7) + 2) >> 2

#define PT(x)   const int t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;
#define PRED8x8_LOAD_TOP_EDGE                                           \
    const int t0 = ((topleft_is_avail ? SRC(-1,-1) : SRC(0,-1))         \
                     + 2*SRC(0,-1) + SRC(1,-1) + 2) >> 2;               \
    PT(1) PT(2) PT(3) PT(4) PT(5) PT(6)                                 \
    const int t7 = ((topright_is_avail ? SRC(8,-1) : SRC(7,-1)) \
                     + 2*SRC(7,-1) + SRC(6,-1) + 2) >> 2

#define PTR(x)  t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;
#define PRED8x8_LOAD_TOPRIGHT_EDGE                                      \
    int t8, t9, t10, t11, t12, t13, t14, t15;                           \
    if(topright_is_avail)                                               \
    {                                                                   \
        PTR(8) PTR(9) PTR(10) PTR(11) PTR(12) PTR(13) PTR(14)           \
        t15 = (SRC(14,-1) + 3*SRC(15,-1) + 2) >> 2;                     \
    }                                                                   \
    else                                                                \
      t8=t9=t10=t11=t12=t13=t14=t15= SRC(7,-1);

#define PRED8x8_LOAD_TOPLEFT_SAMPLE \
    const int lt = (SRC(-1,0) + 2*SRC(-1,-1) + SRC(0,-1) + 2) >> 2


static should_inline void copy8samples(pixel_t* dst, const pixel_t* src)
{
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0 && ((unsigned long)src&7) == 0);
#ifdef PIXEL_T_IS_8_BITS
  *(uint64_t*)dst = *(uint64_t*)src; // could be unaligned 32/64 bits copy
#else // => PIXEL_T_IS_16_BITS
  ((uint64_t*)dst)[0] = ((uint64_t*)src)[0]; // could be unaligned 32/64 bits copy
  ((uint64_t*)dst)[1] = ((uint64_t*)src)[1]; // could be unaligned 32/64 bits copy
#endif
}
static should_inline void duplicate1to8samples(pixel_t* dst, const pixel_t v)
{
#ifndef ARCH_X86_64
  LUD_DEBUG_ASSERT(((unsigned long)dst&3) == 0);

# ifdef PIXEL_T_IS_8_BITS
  ((uint32_t*)dst)[0] = ((uint32_t*)dst)[1] = (uint32_t)v * (uint32_t)0x01010101;
# else // => PIXEL_T_IS_16_BITS
  ((uint32_t*)dst)[0] = ((uint32_t*)dst)[1] = ((uint32_t*)dst)[2] = ((uint32_t*)dst)[3] = (uint32_t)v * (uint32_t)0x00010001;
# endif

#else // ARCH_X86_64
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0);

# ifdef PIXEL_T_IS_8_BITS
  ((uint64_t*)dst)[0] = (uint64_t)v * (uint64_t)0x0101010101010101;
# else // => PIXEL_T_IS_16_BITS
  ((uint64_t*)dst)[0] = ((uint64_t*)dst)[1] = (uint64_t)v * (uint64_t)0x0001000100010001;
# endif
#endif // ARCH_X86_64

}

/*
 * Intra 16x16 prediction
 */
static should_inline void copy16samples(pixel_t* dst, const pixel_t* src)
{
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0 && ((unsigned long)src&7) == 0);
#ifdef PIXEL_T_IS_8_BITS
  *((uint64_t*)dst) = ((uint64_t*)src)[0]; // could be unaligned 32/64 bits copy
  *((uint64_t*)dst+1) = ((uint64_t*)src)[1]; // could be unaligned 32/64 bits copy
#else // => PIXEL_T_IS_16_BITS
  ((uint64_t*)dst)[0] = ((uint64_t*)src)[0]; // could be unaligned 32/64 bits copy
  ((uint64_t*)dst)[1] = ((uint64_t*)src)[1]; // could be unaligned 32/64 bits copy
  ((uint64_t*)dst)[2] = ((uint64_t*)src)[2]; // could be unaligned 32/64 bits copy
  ((uint64_t*)dst)[3] = ((uint64_t*)src)[3]; // could be unaligned 32/64 bits copy
#endif
}
static should_inline void duplicate1to16samples(pixel_t* dst, const pixel_t v)
{

#ifndef ARCH_X86_64
  LUD_DEBUG_ASSERT(((unsigned long)dst&3) == 0);
# ifdef PIXEL_T_IS_8_BITS
  ((uint32_t*)dst)[0] = ((uint32_t*)dst)[1] = ((uint32_t*)dst)[2] = ((uint32_t*)dst)[3] = (uint32_t)v * (uint32_t)0x01010101;
# else // => PIXEL_T_IS_16_BITS
  ((uint32_t*)dst)[0] = ((uint32_t*)dst)[1] = ((uint32_t*)dst)[2] = ((uint32_t*)dst)[3] =
      ((uint32_t*)dst)[4] = ((uint32_t*)dst)[5] = ((uint32_t*)dst)[6] = ((uint32_t*)dst)[7] = (uint32_t)v * (uint32_t)0x00010001;
# endif

#else // ARCH_X86_64

# ifdef PIXEL_T_IS_8_BITS
  LUD_DEBUG_ASSERT(((unsigned long)dst&7) == 0);
  ((uint64_t*)dst)[0] = ((uint64_t*)dst)[1] = (uint64_t)v * (uint64_t)0x0101010101010101;
# else // => PIXEL_T_IS_16_BITS
  ((uint64_t*)dst)[0] = ((uint64_t*)dst)[1] = ((uint64_t*)dst)[2] = ((uint64_t*)dst)[3] = (uint64_t)v * (uint64_t)0x0001000100010001;
# endif

#endif // ARCH_X86_64

}

// mode: 0=avr of left and top samples, 1: avr of left, 2: avr of top, 3: default value zero
static should_inline void pred_intra_16x16_dc(int dy, pixel_t* dst, int mode, pixel_t zero)
{
  int dc;
  if (mode == 0)
  {
    int a, b;
    const pixel_t* sa = dst-dy;
    const pixel_t* sb = dst-1;
    a =  *sa++; b =  *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa++; b += *sb; sb += dy;
    a += *sa;   b += *sb;
    dc = (a + b + 16) >> 5;

//    int a, b;

  }
  else if (mode == 1)
  {
    int b;
    const pixel_t* sb = dst-1;
    b =  *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb; sb += dy;
    b += *sb;
    dc = (b + 8) >> 4;

  }
  else if (mode == 2)
  {
    int a;
    const pixel_t* sa = dst-dy;
    a =  *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa++;
    a += *sa;
    dc = (a + 8) >> 4;


  }
  else // mode = 3
    dc = zero;
  {
    duplicate1to16samples(dst, (pixel_t)dc);
    //duplicate1to16samples(dst, (pixel_t)1);

      int k;
    int j;
    for(j=0;j<16;j++)
    {
        for(k=0;k<16;k++)
        {
          dst[j*48+k]=dst[k];
        }
    }

/*
   copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); d += dy;
    copy16samples(d, src); */

  }
}

/*
 * Intra Chroma pred
 */
static should_inline int add4vertsamples(const pixel_t* src, const int dy)
{
  int a = src[0+0*dy] + src[0+1*dy];
  int b = src[0+2*dy] + src[0+3*dy];
  return a+b;
}
static should_inline int add4horizsamples(const pixel_t* src)
{
  int a = src[0] + src[1];
  int b = src[2] + src[3];
  return a+b;
}
static should_inline void fill_4x4_block(pixel_t* dst, const int dy, const pixel_t v)
{
  pixel_t* d = dst + dy;
  const pixel_t* src;
  duplicate1to4samples(dst, v); src = dst;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src);
}
static should_inline void fill_8x4_block(pixel_t* dst, const int dy, const pixel_t v)
{
  const pixel_t* src;
  duplicate1to8samples(dst, v); src = dst;
    int k;
    int j;
    for(j=0;j<4;j++)
    {
        for(k=0;k<8;k++)
        {
          dst[j*24+k]=dst[k];
        }
    }
  //copy8samples(d, src); d += dy;
 // copy8samples(d, src); d += dy;
  //copy8samples(d, src);
}

static should_inline void fill_4x8_block(pixel_t* dst, const int dy, const pixel_t v)
{
  pixel_t* d = dst + dy;
  const pixel_t* src;
  duplicate1to4samples(dst, v); src = dst;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src); d += dy;
  copy4samples(d, src);
}

#define LOAD_TOP_4SAMPLES_SUM(idx) \
  const int ts##idx = add4horizsamples((src)-(dy)+4*(idx));
#define LOAD_LEFT_4SAMPLES_SUM(idx) \
  const int ls##idx = add4vertsamples((src)-1+4*(idx)*(dy), dy);

static should_inline void pred_intra_chroma_dc_full(int cfidc, int dy, pixel_t* dst, pixel_t* cbsamp, unsigned int left_is_avail, unsigned int top_is_avail, pixel_t zero)
{
  const pixel_t* src = dst;
  pixel_t* d = dst;
  int mode = 0;

  if (!top_is_avail) // top not available
    mode += 1;
  if (!left_is_avail) // left not available
    mode += 2;

  if (mode == 0) // Both left and top edge are available
  {
    LOAD_TOP_4SAMPLES_SUM(0);
    LOAD_TOP_4SAMPLES_SUM(1);
    LOAD_LEFT_4SAMPLES_SUM(0);
    LOAD_LEFT_4SAMPLES_SUM(1);

    fill_4x4_block(d, dy, (ts0 + ls0 + 4) >> 3);
    fill_4x4_block(d+4, dy, (ts1 + 2) >> 2);
    d += 4*dy;
    fill_4x4_block(d, dy, (ls1 + 2) >> 2);
    fill_4x4_block(d+4, dy, (ts1 + ls1 + 4) >> 3);

  }
  else if (mode == 1) // Only left edge is available
  {
    LOAD_LEFT_4SAMPLES_SUM(0);
    LOAD_LEFT_4SAMPLES_SUM(1);
    const int dc0 = (ls0 + 2) >> 2;
    const int dc1 = (ls1 + 2) >> 2;

    fill_8x4_block(d, dy, dc0);
    d += 4*dy;
    fill_8x4_block(d, dy, dc1);

  }
  else if (mode == 2) // Only top edge is available
  {
    LOAD_TOP_4SAMPLES_SUM(0);
    LOAD_TOP_4SAMPLES_SUM(1);
    const int dc0 = (ts0 + 2) >> 2;
    const int dc1 = (ts1 + 2) >> 2;

    fill_4x8_block(d, dy, dc0);
    d += 4;
    fill_4x8_block(d, dy, dc1);

  }
  else // mode = 3 // no edge is available
  {
    if (cfidc <= 2)
    {
      duplicate1to8samples(dst, zero); src = dst;

        int k;
        int j;
        for(j=0;j<8;j++)
        {
            for(k=0;k<8;k++)
            {
              dst[j*24+k]=dst[k];
            }
        }
      //copy8samples(d, src); d += dy;
      //copy8samples(d, src); d += dy;
     // copy8samples(d, src); d += dy;
     // copy8samples(d, src); d += dy;
    //  copy8samples(d, src); d += dy;
     // copy8samples(d, src); d += dy;
    //  copy8samples(d, src);
    }
  }
}






static should_inline void pred_intra16x16(int meanY, int Intra16x16PredMode, pixel_t* dst, int dy,
                                   unsigned int left_is_avail, unsigned int top_is_avail)
{
  switch (Intra16x16PredMode)
  {
    case 2:
    {
      int mode = 0;
      pixel_t zero = meanY;
      if (!top_is_avail) // top not available
        mode += 1;
      if (!left_is_avail) // left not available
        mode += 2;
      pred_intra_16x16_dc(dy, dst, mode, zero);
      break;
    }

    default:
      LUD_DEBUG_ASSERT(0); // non-existing Intra16x16PredMode !!
      break;
  }
}


static should_inline void compute_left_edge_availability(MbAttrib* curr_mb_attr, MB_TYPE mb_type, unsigned int mb_field_decoding_flag,
    unsigned int curr_is_bot_mb,unsigned int PicWidthInMbs, unsigned int MbaffFrameFlag, unsigned int constrained_intra_pred_flag,
    unsigned int* left_is_avail_top, unsigned int* left_is_avail_bot)
{
  if (constrained_intra_pred_flag && curr_mb_attr->left_mb_is_available)
  {
    if (!MbaffFrameFlag || !(curr_mb_attr->left_mb_is_field ^ mb_field_decoding_flag))
    {
      unsigned int avail;
      MbAttrib* left = curr_mb_attr-1;
      avail = left->mb_type < SI || (left->mb_type == mb_type); // The second test is =SI because mb_type is a Intra Mb and the first test failed
      *left_is_avail_top = *left_is_avail_bot = avail;
    }
    else if (mb_field_decoding_flag) // Only one of the left MB must satisfy the constraint
    {
      MbAttrib* left0 = curr_mb_attr -1 -PicWidthInMbs * curr_is_bot_mb;
      MbAttrib* left1 = left0 + PicWidthInMbs;

      *left_is_avail_top = left0->mb_type < SI || (left0->mb_type == mb_type);
      *left_is_avail_bot = left1->mb_type < SI || (left1->mb_type == mb_type);
    }
    else // Both top and bot MB must satisfy the constraint
    {
      unsigned int avail;
      MbAttrib* left0 = curr_mb_attr -1 -PicWidthInMbs * curr_is_bot_mb;
      MbAttrib* left1 = left0 + PicWidthInMbs;

      avail =  (left0->mb_type < SI || (left0->mb_type == mb_type)) && (left1->mb_type < SI || (left1->mb_type == mb_type));
      *left_is_avail_top = *left_is_avail_bot = avail;
    }
  }
  else
  {
    *left_is_avail_top = *left_is_avail_bot = curr_mb_attr->left_mb_is_available;
  }
}
static should_inline unsigned int get_up_edge_availability(MbAttrib* curr_mb_attr, MB_TYPE mb_type, unsigned int mb_field_decoding_flag,
    unsigned int curr_is_bot_mb,unsigned int PicWidthInMbs, unsigned int MbaffFrameFlag)
{
  if (!MbaffFrameFlag)
  {
    MbAttrib* up = curr_mb_attr-PicWidthInMbs;
    return up->mb_type < SI || (up->mb_type == mb_type); // The second test is =SI because mb_type is a Intra Mb and the first test failed
  }
  else
  {
    MbAttrib* up = get_up_mbaff_mb(curr_mb_attr, PicWidthInMbs, mb_field_decoding_flag, curr_mb_attr->up_mb_is_field, curr_is_bot_mb);
    return up->mb_type < SI || (up->mb_type == mb_type); // The second test is =SI because mb_type is a Intra Mb and the first test failed
  }
}
