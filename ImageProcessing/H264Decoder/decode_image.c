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

#include "common.h"
#include "decode.h"
#include "defaulttables.h"
#include "intmath.h"
#include "decode_slice.h"
#include "system.h"
#include "inverse_transforms.h"
#include "../image.h"
#include <math.h>
#ifdef STM32F4XX
    #include "ff.h"
#endif


#include "decode_image_intra.h"

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
unsigned char rgbbuffer[92160];



static should_inline void decode_intra_mb(MbAttrib* curr_mb_attr, MB_TYPE mb_type, int16_t* mb_data, unsigned int PicWidthY,
    unsigned int PicWidthC,
    pixel_t* ysamp, pixel_t* cbsamp, pixel_t* crsamp,
    unsigned int mbc_height, unsigned int mbc_width, int clipY, int clipC, int meanY, int meanC,
    unsigned int mb_field_decoding_flag, unsigned int curr_is_bot_mb, unsigned int PicWidthInMbs, unsigned int MbaffFrameFlag,
    unsigned int constrained_intra_pred_flag)
{

   unsigned int left_is_avail_top, left_is_avail_bot;
   compute_left_edge_availability(curr_mb_attr, mb_type, mb_field_decoding_flag, curr_is_bot_mb, PicWidthInMbs, MbaffFrameFlag,
       constrained_intra_pred_flag, &left_is_avail_top, &left_is_avail_bot);
   unsigned int top_is_avail = (constrained_intra_pred_flag && curr_mb_attr->up_mb_is_available) ?
       get_up_edge_availability(curr_mb_attr, mb_type, mb_field_decoding_flag, curr_is_bot_mb, PicWidthInMbs, MbaffFrameFlag) :
       curr_mb_attr->up_mb_is_available;

   //if (mb_type == I_NxN || mb_type == SI)
   {
     // luma 16x16 pred
     //int pred_mode = Intra16x16PredMode(mb_type);


     int i;
     int16_t temp;
     int16_t* data = mb_data;
     pixel_t* ysamp2;

      pred_intra16x16(128, 2, ysamp, 48, left_is_avail_top && left_is_avail_bot, top_is_avail);

     // The transform the 16 DC coeffs is done in the decode_slice process
     data += 15;
     temp = data[0]; // store the 16th DC coeff
     // Transform the 16*15 AC coeffs. a DC coeff is taken from the previous list
     for (i=0; i<15; i++)
     {
       data[0] = mb_data[i];
       ysamp2 = ysamp+((i>>2)*48 + (i&3))*4;
       inverse_transform(IDCT4x4, curr_mb_attr->luma_total_coeff[i]+(data[0]!=0), curr_mb_attr->transform_bypass, ysamp2, data, 48, clipY);
       data+= 15;
     }
     data[0] = temp;
     ysamp2 = ysamp+((i>>2)*48 + (i&3))*4;
    inverse_transform(IDCT4x4, curr_mb_attr->luma_total_coeff[i]+(data[0]!=0), curr_mb_attr->transform_bypass, ysamp2, data, 48, clipY);
   }


     unsigned int i, r, c;
     int16_t temp;
     pixel_t* cbsamp2;
     pixel_t* crsamp2;
     int16_t* data;
     int num_blocks;


     // r is used for shifting and c is used as a mask

     mb_data+=256; // go to chroma data
       num_blocks = 4;
       r = 1;
       c = 1;


     // Cb
     data = mb_data;
     //pixel_t zero = meanC;
     pred_intra_chroma_dc_full(1, 24, cbsamp, cbsamp, left_is_avail_top, top_is_avail, meanC);

     data += num_blocks-1;
     temp = data[0]; // store the 16th DC coeff
     // Transform the 16*15 AC coeffs. a DC coeff is taken from the previous list
     for (i=0; i<num_blocks-1; i++)
     {
       data[0] = mb_data[i];
       cbsamp2 = cbsamp + ((i>>r)*24 + (i&c))*4;
      inverse_transform(IDCT4x4, curr_mb_attr->chroma_total_coeff[0][i]+(data[0]!=0), curr_mb_attr->transform_bypass, cbsamp2, data, 24, clipC);

       data+= 15;
     }
     data[0] = temp;
     cbsamp2 = cbsamp + ((i>>r)*24 + (i&c))*4;
     inverse_transform(IDCT4x4, curr_mb_attr->chroma_total_coeff[0][i]+(data[0]!=0), curr_mb_attr->transform_bypass, cbsamp2, data, 24, clipC);

     // Cr
     mb_data += num_blocks*16;
     data = mb_data;
     pred_intra_chroma_dc_full(1, 24, crsamp, crsamp, left_is_avail_top, top_is_avail, meanC);
     data += num_blocks-1;
     temp = data[0]; // store the 16th DC coeff
     // Transform the 16*15 AC coeffs. a DC coeff is taken from the previous list
     for (i=0; i<num_blocks-1; i++)
     {
       data[0] = mb_data[i];
       crsamp2 = crsamp + ((i>>r)*24 + (i&c))*4;
       inverse_transform(IDCT4x4, curr_mb_attr->chroma_total_coeff[1][i]+(data[0]!=0), curr_mb_attr->transform_bypass, crsamp2, data, 24, clipC);
       data+= 15;
     }
     data[0] = temp;
     crsamp2 = crsamp + ((i>>r)*24 + (i&c))*4;
     inverse_transform(IDCT4x4, curr_mb_attr->chroma_total_coeff[1][i]+(data[0]!=0), curr_mb_attr->transform_bypass, crsamp2, data, 24, clipC);

}

static should_inline void rgbconvert(pixel_t* ybuffer, pixel_t* ubuffer, pixel_t* vbuffer, FM_FILE* outfile, pixel_t* topy, pixel_t* topcb,
                                     pixel_t* topcr, int i, pixel_t* ysamp, pixel_t* cbsamp, pixel_t* crsamp)
{


       unsigned int j;
       unsigned int k;
       unsigned short row;
       unsigned short pos;


       row = floor(i/120);
       pos = i - row*120;

       unsigned short tempj;
       unsigned short tempk;
       unsigned int bytes_written;
       int red;
       int blue;
       int green;
       unsigned char grey[16][16];


       for(j=0;j<16;j++)
       {
            for(k=0;k<16;k++)
            {
               tempj = floor(j/2);
               tempk = floor(k/2);

                red = 1.16414*ybuffer[j*48+k] + 1.59579*vbuffer[tempj*24+tempk] - (0.001789*ubuffer[tempj*24+tempk]) - 222.658;
                green = 1.16414*ybuffer[j*48+k] - 0.8135*vbuffer[tempj*24+tempk] - (0.3914*ubuffer[tempj*24+tempk]) + 135.604;
                blue = 1.16414*ybuffer[j*48+k] - 0.001246*vbuffer[tempj*24+tempk] + (2.01783*ubuffer[tempj*24+tempk]) - 276.749;

                if(red>255)
                {
                    red = 255;
                }
                if(green>255)
                {
                    green = 255;
                }
                if(blue>255)
                {
                    blue = 255;
                }

                if(red<0)
                {
                    red = 0;
                }
                if(green<0)
                {
                    green = 0;
                }
                if(blue<0)
                {
                    blue = 0;
                }

               rgbbuffer[j*48+k*3+pos*768]=blue;
               rgbbuffer[j*48+k*3+1+pos*768]=green;
               rgbbuffer[j*48+k*3+2+pos*768]=red;
               grey[j][k] = (red+blue+green)/3;



               if(pos!=119)
               {
                    ysamp[768+j*48+k] = ybuffer[j*48+k];
                    cbsamp[192+tempj*24+tempk] = ubuffer[tempj*24+tempk];
                    crsamp[192+tempj*24+tempk] = vbuffer[tempj*24+tempk];
               }

                ysamp[j*48+k] = topy[j*1920+pos*16+k];
                cbsamp[tempj*24+tempk] = topcb[tempj*960+pos*8+tempk];
                crsamp[tempj*24+tempk] = topcr[tempj*960+pos*8+tempk];

                if(pos==118)
                {
                    ysamp[j*48+k+16]            = topy[j*1920+(pos+1)*16+k];
                    cbsamp[tempj*24+tempk+8]    = topcb[tempj*960+(pos+1)*8+tempk];
                    crsamp[tempj*24+tempk+8]    = topcr[tempj*960+(pos+1)*8+tempk];
                }
                else if(pos==119)
                {

                    ysamp[j*48+k+16]            = topy[j*1920+(0)*16+k];
                    cbsamp[tempj*24+tempk+8]    = topcb[tempj*960+(0)*8+tempk];
                    crsamp[tempj*24+tempk+8]    = topcr[tempj*960+(0)*8+tempk];

                    ysamp[j*48+k+32]            = topy[j*1920+(1)*16+k];
                    cbsamp[tempj*24+tempk+16]   = topcb[tempj*960+(1)*8+tempk];
                    crsamp[tempj*24+tempk+16]   = topcr[tempj*960+(1)*8+tempk];
                }
                else
                {
                    ysamp[j*48+k+16]            = topy[j*1920+(pos+1)*16+k];
                    cbsamp[tempj*24+tempk+8]    = topcb[tempj*960+(pos+1)*8+tempk];
                    crsamp[tempj*24+tempk+8]    = topcr[tempj*960+(pos+1)*8+tempk];

                    ysamp[j*48+k+32]            = topy[j*1920+(pos+2)*16+k];
                    cbsamp[tempj*24+tempk+16]   = topcb[tempj*960+(pos+2)*8+tempk];
                    crsamp[tempj*24+tempk+16]   = topcr[tempj*960+(pos+2)*8+tempk];
                }

               topy[j*1920+pos*16+k] = ybuffer[j*48+k];
               topcb[tempj*960+pos*8+tempk] = ubuffer[tempj*24+tempk];
               topcr[tempj*960+pos*8+tempk] = vbuffer[tempj*24+tempk];
            }
        }

        unsigned char average;

        for(j=0;j<8;j++)
        {
            for(k=0;k<8;k++)
            {
                 average = (grey[j*2][k*2] + grey[j*2][k*2+1] + grey[j*2+1][k*2] + grey[j*2+1][k*2+1])/4;
                 CurrentFrame.framebits[row*8+j][pos*8+k] = average;
            }
        }

        if(pos==119)
        {
            bytes_written = 1;
            fm_write(rgbbuffer,92160,bytes_written,outfile);
        }

    return;
}


static should_inline void decode_image_internal(PictureDecoderData* pdd, FM_FILE* outfile, int16_t* datarray, int CurrMbAddr, MbAttrib* curr_mb_attr,
                                                pixel_t* topy, pixel_t* topcb, pixel_t* topcr, pixel_t* ysamp, pixel_t* cbsamp, pixel_t* crsamp)
{
  unsigned int mb_row;
  unsigned int mb_col;
  unsigned int mb_idx;
  unsigned int i;
  unsigned int j;


  Picture* pic = pdd->pic;
  slice_header_t* slice0 = pic->field_sh[0]; // get a slice header. It is used for variables that are the same for the whole picture
  seq_parameter_set_rbsp_t* sps = slice0->sps;
  pic_parameter_set_rbsp_t* pps = slice0->pps;
  int PicWidthInMbs = sps->PicWidthInMbs;

  unsigned int mbc_width = MbWidthC[1];
  unsigned int mbc_height = MbHeightC[1];
  int clipY = (1<<sps->BitDepthY)-1;
  int meanY = 1<<(sps->BitDepthY-1);
  int clipC = (1<<sps->BitDepthC)-1;
  int meanC = 1<<(sps->BitDepthC-1);

  slice_header_t* sh;
  unsigned int constrained_intra_pred_flag = pps->constrained_intra_pred_flag;

  pixel_t* ysamp2;
  pixel_t* cbsamp2;
  pixel_t* crsamp2;

  ysamp2 = ysamp + 768 + 16;
  cbsamp2 = cbsamp + 192 + 8;
  crsamp2 = crsamp + 192 + 8;

  for (j = 0; j<=pic->slice_num; j++)
  {
    sh = pic->field_sh[j];
    //CurrMbAddr = sh->first_mb_in_slice;

    //for (i = 0; i<sh->mb_nb; i++)
     for (i = 0; i<1; i++)
    {
        //printf(" %d ",CurrMbAddr);

      mb_row = (CurrMbAddr) / PicWidthInMbs;
      mb_col = (CurrMbAddr) % PicWidthInMbs;
      mb_idx = (CurrMbAddr);
      //curr_mb_attr = &mb_attr[mb_idx];
      //curr_mb_data = mb_data + mb_idx * mb_data_size;

        {
          MB_TYPE mb_type = curr_mb_attr->mb_type;
          unsigned int mb_field_decoding_flag = curr_mb_attr->mb_field_decoding_flag;

            decode_intra_mb(curr_mb_attr, mb_type, datarray, 1920, 960,
               ysamp2, cbsamp2, crsamp2,
               mbc_height, mbc_width, clipY, clipC, meanY, meanC, mb_field_decoding_flag, mb_row&1, PicWidthInMbs, 0,
              constrained_intra_pred_flag);


           rgbconvert(ysamp2, cbsamp2, crsamp2, outfile,topy,topcb,topcr,CurrMbAddr,ysamp,cbsamp,crsamp);


        }
    }
  }
}

void decode_image(PictureDecoderData* pdd, Picture* pic, unsigned int bottom_field_flag,int16_t* datarray, FM_FILE* outfile,int CurrMbAddr, MbAttrib* curr_mb_attr,
                  pixel_t* topy, pixel_t* topcb, pixel_t* topcr, pixel_t* ysamp, pixel_t* cbsamp, pixel_t* crsamp)
{
  slice_header_t* slice0 = pic->field_sh[0]; // get a slice header. It is used for variables that are the same for the whole picture
  seq_parameter_set_rbsp_t* sps = slice0->sps;


  LUD_DEBUG_ASSERT(sps->residual_colour_transform_flag == 0); // not implemented yet
  LUD_DEBUG_ASSERT(sps->BitDepthY == 8); // Not tested yet
  LUD_DEBUG_ASSERT(sps->BitDepthC == 8); // Not tested yet
  decode_image_internal(pdd, outfile,datarray,CurrMbAddr,curr_mb_attr,topy,topcb,topcr,ysamp,cbsamp,crsamp);

}



