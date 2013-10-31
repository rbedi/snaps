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


#include "decode.h"
#include "system.h"
#include "defaulttables.h"
#include <stdio.h>
#include <string.h>


typedef struct OUTPICLIST
{
  struct OUTPICLIST* next;
  Picture* pic;
} OUTPICLIST;

static OUTPICLIST* head = 0;
static OUTPICLIST* tail = (OUTPICLIST*) &head; // a bit tricky, it works because next is a pointer and first member of the struct


Picture* get_pic()
{
  if (head == 0)
    return 0;

  OUTPICLIST* p = head;
  Picture* pic = p->pic;
  head = p->next;

  if (!head)
    tail = (OUTPICLIST*) &head;

  return pic;
}



// Decoded Picture Buffer

#define NEXT(a) ((a+1) & 0xF)
#define PREV(a) ((a-1) & 0xF)


void init_dpb()
{
//  gdd.dpb.first = 0;
//  gdd.dpb.last = 0;
 // gdd.dpb.nb_of_images = 0;
//  gdd.dpb.last_added_image = 0;
}



void flush_dpb()
{
//  DecodedPictureBuffer* ob = &gdd.dpb;

//  if (ob->last_added_image)
//    insert_image(ob->last_added_image);
 // ob->last_added_image = 0;
//  flush_images();


}

void close_dpb()
{
}
