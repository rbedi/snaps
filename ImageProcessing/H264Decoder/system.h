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



#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "decode.h"
#include "syntax_types.h"


Picture* set_ref_pic(PictureDecoderData* pdd, slice_header_t* sh);
Picture* allocate_and_set_non_existing_ref_pic(int PicOrderCnt, int TopFieldOrderCnt, int BottomFieldOrderCnt, int frame_num, int ref_struct, unsigned int bottom_field_flag);
Picture* alloc_picture(PictureDecoderData* pdd, slice_header_t* sh);
void check_free_picture(Picture* pic);
void release_picture_refpics(Picture* pic, unsigned int bottom_field_flag);


static should_inline void use_picture(Picture* pic)
{
  pic->field_use_count[0]++;
  pic->field_use_count[1]++;
}
static should_inline void use_picture_field(Picture* pic, unsigned int bottom_field_flag)
{
  pic->field_use_count[bottom_field_flag]++;
}
static should_inline void use_picture_dec_data(Picture* pic)
{
  pic->dec_data_use_count++;
}
static should_inline void release_picture(Picture* pic)
{
  if (pic)
  {
    //LUD_DEBUG_ASSERT(pic->field_use_count[0]>=0 && pic->field_use_count[1]>=0);
    pic->field_use_count[0]--;
    pic->field_use_count[1]--;
    check_free_picture(pic);
  }
}
static should_inline void release_picture_field(Picture* pic, unsigned int bottom_field_flag)
{
  if (pic)
  {
    //LUD_DEBUG_ASSERT(pic->field_use_count[bottom_field_flag]>=0);
    pic->field_use_count[bottom_field_flag]--;
    check_free_picture(pic);
  }
}
static should_inline void release_picture_field_struct(Picture* pic, unsigned int field_struct)
{
  if (pic)
  {
    if (field_struct&1)
    {
      //LUD_DEBUG_ASSERT(pic->field_use_count[0]>=0);
      pic->field_use_count[0]--;
    }
    if (field_struct&2)
    {
      //LUD_DEBUG_ASSERT(pic->field_use_count[1]>=0);
      pic->field_use_count[1]--;
    }

    check_free_picture(pic);
  }
}
static should_inline void release_picture_dec_data(Picture* pic)
{
  if (pic)
  {
    //LUD_DEBUG_ASSERT(pic->dec_data_use_count>=0);
    pic->dec_data_use_count--;
    check_free_picture(pic);
  }
}



// DEBUG
void print_nb_of_allocated_pictures();


#endif //__SYSTEM_H__

