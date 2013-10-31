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


#ifndef __INTMATH_H__
#define __INTMATH_H__

#include <inttypes.h>
#include "common.h"

static const uint8_t log2_lookup_table[256] =
{
  0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};



#ifdef ARCH_X86_64
#  define LEGACY_REGS "=Q"
#else
#  define LEGACY_REGS "=q"
#endif

#if defined(ARCH_X86)
// avoid +32 for shift optimization (gcc should do that ...)
static should_inline  int32_t NEG_SSR32( int32_t a, int8_t s){
    asm ("sarl %1, %0\n\t"
         : "+r" (a)
         : "ic" ((uint8_t)(-s))
    );
    return a;
}
static should_inline uint32_t NEG_USR32(uint32_t a, int8_t s){
    asm ("shrl %1, %0\n\t"
         : "+r" (a)
         : "ic" ((uint8_t)(-s))
    );
    return a;
}
#else
#    define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#    define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif


static should_inline uint16_t bswap_16(uint16_t x)
{
#if defined(ARCH_X86)
    __asm("rorw $8, %0"   :
          LEGACY_REGS (x) :
          "0" (x));
#else
    x= (x>>8) | (x<<8);
#endif
    return x;
}

static should_inline uint32_t bswap_32(uint32_t x)
{
#if defined(ARCH_X86)
#if __CPU__ != 386
    __asm("bswap   %0":
          "=r" (x)    :
#else
    __asm("xchgb   %b0,%h0\n"
          "rorl    $16,%0 \n"
          "xchgb   %b0,%h0":
          LEGACY_REGS (x)  :
#endif
          "0" (x));
#else
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
#endif
    return x;
}

static should_inline uint64_t bswap_64(uint64_t x)
{
  union {
      uint64_t ll;
      uint32_t l[2];
  } w, r;
  w.ll = x;
  r.l[0] = bswap_32 (w.l[1]);
  r.l[1] = bswap_32 (w.l[0]);
  return r.ll;
}

// this is a floor function
static should_inline int im_log2(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += log2_lookup_table[v];

    return n;
}
static should_inline int im_ceillog2(unsigned int v)
{
  return im_log2((v<<1)-1);
}

// scale value v:
// if s==-1 return v/2
// if s== 1 return v*2
// if s== 0 return v
static should_inline int im_scale2(int v, int s)
{
  int t[3] = {v/2, v, v*2};
  return t[s+1];
}
// same as above except that the scaler s is biased
static should_inline int im_scale2_biased(int v, unsigned int s)
{
  int t[4] = {v, v*2, v/2, v};
  return t[s];
}



// 32 bits operators
static should_inline uint32_t get32bitsvalue(void* p)
{
  assert(((unsigned long)p & 3) == 0);
  return *((uint32_t*)p);
}

static should_inline void set32bitsvalue(void* p, uint32_t v)
{
  assert(((unsigned long)p & 3) == 0);
  *((uint32_t*)p) = v;
}

static should_inline void copy32bitsvalue(void* t, const void* f)
{
  assert(((unsigned long)t & 3) == 0);
  assert(((unsigned long)f & 3) == 0);

  *((uint32_t*)t) = *((uint32_t*)f);// Not correct with C ISO Aliasing rules...
  /*asm("mov (%0), %%eax\n\t"
      "mov %%eax, (%1)\n\t"
      : // no output
      : "r" (f), "r" (t)

  );*/
}



// 64 bits operators
static should_inline void set64bitsvalue(void* p, uint64_t v)
{
  assert(((unsigned long)p & 7) == 0);
  *((uint64_t*)p) = v;
}

static should_inline void copy64bitsvalue(void* t, const void* f)
{
  assert(((unsigned long)t & 7) == 0);
  assert(((unsigned long)f & 7) == 0);

  //*((uint64_t*)t) = *((uint64_t*)f); Not correct with C ISO Aliasing rules...
}


// TODO: optimize min, max, abs, clip functions (different implementation on different arch)
#define im_min(a,b)  ((a)<(b) ? (a) : (b))
#define im_max(a,b)  ((a)>(b) ? (a) : (b))
#define im_abs(a) ((a)>0 ? (a) : -(a))

// Output of clip is r where i <= r <= s
//#define im_clip(i, s, x) im_max(im_min((s), (x)), (i))
#define im_clip(i, s, x) ((x) < (i) ? (i) : (x) > (s) ? (s) : (x))
/*static int im_clip(int i, int s, int x)
{
  if (i != 0 || s != 255)
    printf("  %d, %d  |", i, s);
  return im_max(im_min((s), (x)), (i));
}*/



#endif //__INTMATH_H__
