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


#include <inttypes.h>
#include <string.h>
#include "common.h"
#include "system.h"
#include "decode.h"
#include "defaulttables.h"


Picture* set_ref_pic(PictureDecoderData* pdd, slice_header_t* sh)
{
  Picture* pic = pdd->pic;

  if (sh->field_pic_flag)
    use_picture_field(pic, sh->bottom_field_flag);
  else
    use_picture(pic);

  pic->ref_struct = pic->structure;

  return pic;
}


static should_inline void init_mb_attr(MbAttrib* mb_attr, unsigned int nb)
{
  int i;
  while (nb > 15)
  {
    for (i = 0; i < 16; ++i)
    {
      mb_attr->slice_num = -1;
      //mb_attr->mtDecInfo = 0;
      mb_attr++;
    }
    nb -= 16;
  }
  for (i = 0; i < nb; ++i)
  {
    mb_attr->slice_num = -1;
    //mb_attr->mtDecInfo = 0;
    mb_attr++;
  }
}

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
static Picture static_pic;

// Allocate necessary data for a picture
Picture* alloc_picture(PictureDecoderData* pdd, slice_header_t* sh)
{
  Picture* pic = &static_pic;

//  unsigned int PicSizeInMbs = sh->PicSizeInMbs;
  unsigned int field_pic_flag = sh->field_pic_flag;
//  unsigned int FrameSizeInMbs = PicSizeInMbs << (field_pic_flag!=0);
//  unsigned int cfidc = sh->sps->chroma_format_idc;
//  unsigned int chroma_mb_size = MbSizeC[cfidc];
  unsigned int bottom_field_flag = sh->bottom_field_flag;


  // Pic structure alloc
  memset(&static_pic, 0, sizeof(static_pic)); // ensure that all fields are default to 0...

  // Set ref counter to -1 (unsused). When set to 0 (memset above) it means it used 1 time.
  if (field_pic_flag)
    pic->field_use_count[1-bottom_field_flag] = -1;
  pic->dec_data_use_count = -1;

  // Pic decoded data alloc

  // I allocate 1 more line here (+ PicWidthInMbs*16) because some pixel transforms function process 2 lines at a time (when using SIMD instructions)


  // Pic mb data alloc
    pic->field_data.mb_attr = (MbAttrib*)pic->field_data.mb_attr_data;
    //lud_malloc(1909440);
//    printf("\n Pic md data alloc: %d ",((256+chroma_mb_size*2)*sizeof(*pic->field_data.data)+sizeof(*pic->field_data.mb_attr))*FrameSizeInMbs);
//  LUD_DEBUG_ASSERT(pic->field_data[0].mb_attr);
//    printf("\n\n HERE: %d ",sizeof(*pic->field_data.mb_attr));
    pic->field_data.data  =
    (int16_t*)((uint8_t*)pic->field_data.mb_attr + 240*sizeof(*pic->field_data.mb_attr));

  init_mb_attr(pic->field_data.mb_attr, 240);


  // Slice pointers alloc
  // We need to do 2 separate mallocs because if the array is too small it can be reallocated while decoding...
  // The second malloc is done when we have a field picture (see bellow)
  //pic->field_sh = lud_malloc(sizeof(*pic->field_sh[0]) * pdd->max_num_of_slices);
  //LUD_DEBUG_ASSERT(pic->field_sh[0]);

  // When dealing with fields, just need to add an offset for the second field

  {
    pic->structure = 7;
  }

  pic->frame_num = sh->frame_num;

  return pic;
}


// Check if some part (or whole) of the picture can be free'd and do it !
void check_free_picture(Picture* pic)
{
  unsigned int i;
  slice_header_t** sh;

  LUD_DEBUG_ASSERT(pic->dec_data_use_count>=-1);
  // This is a bit tricky: we actually increment this counter so that we are sure that the picture structure will NOT be free'd in the middle of the process
  //  it could indeed happen otherwise: all field counter are set to -1 when the second field was referencing the first one of the same pic !
  pic->dec_data_use_count++;

  // if 2 different mallocs for each field data
  {
    if (pic->field_sh && pic->field_use_count[0] < 0 && pic->field_use_count[1] < 0)
    {
      sh = pic->field_sh;
      pic->field_sh[0] = pic->field_sh[1] = 0; // set to 0 so that recursion will not double free...
      for (i = 0; i <= pic->slice_num; ++i)
        release_slice_header(sh[i]);
    }
  }

  pic->dec_data_use_count--; // see comment at the beginning of this function (increment of the same variable)

}


