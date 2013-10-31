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



#ifndef __INVERSE_TRANSFORMS_H__
#define __INVERSE_TRANSFORMS_H__







typedef enum
{
  IDCT4x4,
  IDCT8x8,
  IHDM2x2,
  IHDM2x4,
  IHDM4x4,
} TRANSFORM_TYPE;


// This is inlined code, and used in 2 diff c files, then to make it inline, it need to be in this header file...


/*
 * Transform functions
 */

 // 4x4 IDCT-like transform
static should_inline void idct4x4(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i;

  block[0] += 32;

  for(i=0; i<4; i++)
  {
    const int z0=  block[0 + 4*i]     +  block[2 + 4*i];
    const int z1=  block[0 + 4*i]     -  block[2 + 4*i];
    const int z2= (block[1 + 4*i]>>1) -  block[3 + 4*i];
    const int z3=  block[1 + 4*i]     + (block[3 + 4*i]>>1);

    block[0 + 4*i]= z0 + z3;
    block[1 + 4*i]= z1 + z2;
    block[2 + 4*i]= z1 - z2;
    block[3 + 4*i]= z0 - z3;
  }

  for(i=0; i<4; i++)
  {
    const int z0=  block[i + 4*0]     +  block[i + 4*2];
    const int z1=  block[i + 4*0]     -  block[i + 4*2];
    const int z2= (block[i + 4*1]>>1) -  block[i + 4*3];
    const int z3=  block[i + 4*1]     + (block[i + 4*3]>>1);

    dst[i + 0*dy]= im_clip( 0, clip_val, dst[i + 0*dy] + ((z0 + z3) >> 6) );
    dst[i + 1*dy]= im_clip( 0, clip_val, dst[i + 1*dy] + ((z1 + z2) >> 6) );
    dst[i + 2*dy]= im_clip( 0, clip_val, dst[i + 2*dy] + ((z1 - z2) >> 6) );
    dst[i + 3*dy]= im_clip( 0, clip_val, dst[i + 3*dy] + ((z0 - z3) >> 6) );


    // DEBUG => to comment out !
    /*
    block[0*4+i]= (z0 + z3)>>6;
    block[1*4+i]= (z1 + z2)>>6;
    block[2*4+i]= (z1 - z2)>>6;
    block[3*4+i]= (z0 - z3)>>6;
    */

  }
}

// 8x8 IDCT-like transform
static should_inline void idct8x8(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i;
  int16_t (*src)[8] = (int16_t(*)[8])block;

  block[0] += 32;

  for( i = 0; i < 8; i++ )
  {
    const int a0 =  src[i][0] + src[i][4];
    const int a2 =  src[i][0] - src[i][4];
    const int a4 = (src[i][2]>>1) - src[i][6];
    const int a6 = (src[i][6]>>1) + src[i][2];

    const int b0 = a0 + a6;
    const int b2 = a2 + a4;
    const int b4 = a2 - a4;
    const int b6 = a0 - a6;

    const int a1 = -src[i][3] + src[i][5] - src[i][7] - (src[i][7]>>1);
    const int a3 =  src[i][1] + src[i][7] - src[i][3] - (src[i][3]>>1);
    const int a5 = -src[i][1] + src[i][7] + src[i][5] + (src[i][5]>>1);
    const int a7 =  src[i][3] + src[i][5] + src[i][1] + (src[i][1]>>1);

    const int b1 = (a7>>2) + a1;
    const int b3 =  a3 + (a5>>2);
    const int b5 = (a3>>2) - a5;
    const int b7 =  a7 - (a1>>2);

    src[i][0] = b0 + b7;
    src[i][7] = b0 - b7;
    src[i][1] = b2 + b5;
    src[i][6] = b2 - b5;
    src[i][2] = b4 + b3;
    src[i][5] = b4 - b3;
    src[i][3] = b6 + b1;
    src[i][4] = b6 - b1;
  }
  for( i = 0; i < 8; i++ )
  {
    const int a0 =  src[0][i] + src[4][i];
    const int a2 =  src[0][i] - src[4][i];
    const int a4 = (src[2][i]>>1) - src[6][i];
    const int a6 = (src[6][i]>>1) + src[2][i];

    const int b0 = a0 + a6;
    const int b2 = a2 + a4;
    const int b4 = a2 - a4;
    const int b6 = a0 - a6;

    const int a1 = -src[3][i] + src[5][i] - src[7][i] - (src[7][i]>>1);
    const int a3 =  src[1][i] + src[7][i] - src[3][i] - (src[3][i]>>1);
    const int a5 = -src[1][i] + src[7][i] + src[5][i] + (src[5][i]>>1);
    const int a7 =  src[3][i] + src[5][i] + src[1][i] + (src[1][i]>>1);

    const int b1 = (a7>>2) + a1;
    const int b3 =  a3 + (a5>>2);
    const int b5 = (a3>>2) - a5;
    const int b7 =  a7 - (a1>>2);

    dst[i + 0*dy] = im_clip(0, clip_val, dst[i + 0*dy] + ((b0 + b7) >> 6) );
    dst[i + 1*dy] = im_clip(0, clip_val, dst[i + 1*dy] + ((b2 + b5) >> 6) );
    dst[i + 2*dy] = im_clip(0, clip_val, dst[i + 2*dy] + ((b4 + b3) >> 6) );
    dst[i + 3*dy] = im_clip(0, clip_val, dst[i + 3*dy] + ((b6 + b1) >> 6) );
    dst[i + 4*dy] = im_clip(0, clip_val, dst[i + 4*dy] + ((b6 - b1) >> 6) );
    dst[i + 5*dy] = im_clip(0, clip_val, dst[i + 5*dy] + ((b4 - b3) >> 6) );
    dst[i + 6*dy] = im_clip(0, clip_val, dst[i + 6*dy] + ((b2 - b5) >> 6) );
    dst[i + 7*dy] = im_clip(0, clip_val, dst[i + 7*dy] + ((b0 - b7) >> 6) );

    // DEBUG
    /*
    src[0][i]= (b0 + b7)>>6;
    src[1][i]= (b2 + b5)>>6;
    src[2][i]= (b4 + b3)>>6;
    src[3][i]= (b6 + b1)>>6;
    src[4][i]= (b6 - b1)>>6;
    src[5][i]= (b4 - b3)>>6;
    src[6][i]= (b2 - b5)>>6;
    src[7][i]= (b0 - b7)>>6;
    */
  }
}


// Optimisation when the residual only contains the DC coeff
static should_inline void idct4x4_dc(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i, j;
  int dc = (block[0] + 32) >> 6;
//  printf(" %d ",dc);
  for( j = 0; j < 4; j++ )
  {
    for( i = 0; i < 4; i++ )
      dst[i] = im_clip(0, clip_val, dst[i] + dc);
    dst += dy;
  }
}

static should_inline void idct8x8_dc(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i, j;
  int dc = (block[0] + 32) >> 6;
  for( j = 0; j < 8; j++ )
  {
    for( i = 0; i < 8; i++ )
      dst[i] = im_clip(0, clip_val, dst[i] + dc);
    dst += dy;
  }
}

// Bypass transform => only add the coeffs to the predicted samples
static should_inline void add4x4(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i, j;
  int k = 0;
  for( j = 0; j < 4; j++ )
  {
    for( i = 0; i < 4; i++ )
    {
      dst[i] = im_clip(0, clip_val, dst[i] + block[k]);
      k++;
    }
    dst += dy;
  }
}

static should_inline void add8x8(pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  int i, j;
  int k = 0;
  for( j = 0; j < 8; j++ )
  {
    for( i = 0; i < 8; i++ )
    {
      dst[i] = im_clip(0, clip_val, dst[i] + block[k]);
      k++;
    }
    dst += dy;
  }
}


static void inverse_dc_transform4x4(int16_t *block)
{
  int i;

  for(i=0; i<4; i++)
  {
    const int z0=  block[0 + 4*i] + block[1 + 4*i];
    const int z1=  block[0 + 4*i] - block[1 + 4*i];
    const int z2=  block[2 + 4*i] + block[3 + 4*i];
    const int z3= -block[2 + 4*i] + block[3 + 4*i];

    block[0 + 4*i]= z0 + z2;
    block[1 + 4*i]= z0 - z2;
    block[2 + 4*i]= z1 + z3;
    block[3 + 4*i]= z1 - z3;
  }

  for(i=0; i<4; i++)
  {
    const int z0=  block[i + 4*0] + block[i + 4*1];
    const int z1=  block[i + 4*0] - block[i + 4*1];
    const int z2=  block[i + 4*2] + block[i + 4*3];
    const int z3= -block[i + 4*2] + block[i + 4*3];

    block[i + 0*4]= z0 + z2;
    block[i + 1*4]= z0 - z2;
    block[i + 2*4]= z1 + z3;
    block[i + 3*4]= z1 - z3;
  }
}

static void inverse_dc_transform2x2(int16_t *block)
{
  int i;

  for(i=0; i<2; i++)
  {
    const int z0=  block[0 + 2*i];
    const int z1=  block[1 + 2*i];

    block[0 + 2*i]= z0 + z1;
    block[1 + 2*i]= z0 - z1;
  }

  for(i=0; i<2; i++)
  {
    const int z0=  block[i + 2*0];
    const int z1=  block[i + 2*1];

    block[i + 0*2]= z0 + z1;
    block[i + 1*2]= z0 - z1;
  }
}

static void inverse_dc_transform2x4(int16_t *block)
{
  int i;

  for(i=0; i<4; i++)
  {
    const int z0=  block[0 + 2*i];
    const int z1=  block[1 + 2*i];

    block[0 + 2*i]= z0 + z1;
    block[1 + 2*i]= z0 - z1;
  }

  for(i=0; i<2; i++)
  {
    const int z0=  block[i + 2*0] + block[i + 2*1];
    const int z1=  block[i + 2*0] - block[i + 2*1];
    const int z2=  block[i + 2*2] + block[i + 2*3];
    const int z3= -block[i + 2*2] + block[i + 2*3];

    block[i + 0*2]= z0 + z2;
    block[i + 1*2]= z0 - z2;
    block[i + 2*2]= z1 + z3;
    block[i + 3*2]= z1 - z3;
  }
}


static should_inline void fill_dc_4x4(int16_t *block)
{
  uint16_t dc = block[0];


#ifndef ARCH_X86_64
  uint32_t v = 0x00010001 * dc;
  *(uint32_t*)(block+0) = v;
  *(uint32_t*)(block+2) = v;
  *(uint32_t*)(block+4) = v;
  *(uint32_t*)(block+6) = v;
  *(uint32_t*)(block+8) = v;
  *(uint32_t*)(block+10) = v;
  *(uint32_t*)(block+12) = v;
  *(uint32_t*)(block+14) = v;
#else
  uint64_t v = 0x0001000100010001ULL * dc;

  *(uint64_t*)(block+0) = v;
  *(uint64_t*)(block+4) = v;
  *(uint64_t*)(block+8) = v;
  *(uint64_t*)(block+12) = v;
#endif


}
static should_inline void fill_dc_2x2(int16_t *block)
{
  uint16_t dc = block[0];

#ifndef ARCH_X86_64
  uint32_t v = 0x00010001 * dc;

  *(uint32_t*)(block+0) = v;
  *(uint32_t*)(block+2) = v;
#else
  uint64_t v = 0x0001000100010001ULL * dc;

  *(uint64_t*)(block+0) = v;
#endif
}
static should_inline void fill_dc_2x4(int16_t *block)
{
  uint16_t dc = block[0];

#ifndef ARCH_X86_64
  uint32_t v = 0x00010001 * dc;

  *(uint32_t*)(block+0) = v;
  *(uint32_t*)(block+2) = v;
  *(uint32_t*)(block+4) = v;
  *(uint32_t*)(block+6) = v;
#else
  uint64_t v = 0x0001000100010001ULL * dc;

  *(uint64_t*)(block+0) = v;
  *(uint64_t*)(block+4) = v;
#endif
}


static should_inline void inverse_transform(TRANSFORM_TYPE type, unsigned int total_coeffs, unsigned int bypass_transform, pixel_t *dst, int16_t *block, int dy, int clip_val)
{
  if (!total_coeffs)
  {
    // Nothing to do, all coeffs are null !
  }

  else if (total_coeffs == 1 && block[0]) // only the DC coeff is non null
  {
    if (type == IDCT4x4)
      idct4x4_dc(dst, block, dy, clip_val);
    else if (type == IHDM4x4)
      fill_dc_4x4(block);
    else if (type == IHDM2x2)
      fill_dc_2x2(block);
    else // IHDM2x4
      fill_dc_2x4(block);
  }
  else
  {
    if (type == IDCT4x4)
      idct4x4(dst, block, dy, clip_val);
    else if (type == IHDM4x4)
      inverse_dc_transform4x4(block);
    else if (type == IHDM2x2)
      inverse_dc_transform2x2(block);
    else // IHDM2x4
      inverse_dc_transform2x4(block);
  }
}



#endif //__INVERSE_TRANSFORMS_H__
