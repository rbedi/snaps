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


#ifndef __BITSTREAM_TYPES_H__
#define __BITSTREAM_TYPES_H__



typedef struct
{
  uint8_t*      buffer;     // pointer to the following bytes in the stream
  unsigned int  index;      // this is a index pointing out the next bit to read into the stream. Actually this is the number of bit read (including the ones in the cache)

} BitStreamContext;

typedef struct
{
  uint8_t  code_symbol[1024];
  int8_t*   code_length;
  uint16_t  table_size;
} vlc_table;

typedef struct
{
  uint8_t     bits;
  vlc_table   table;
  vlc_table  subtable[255];
  uint8_t     nb_of_subtables;
#ifndef NDEBUG
  unsigned int total_size;
#endif
} VLCReader;




#endif //__BITSTREAM_TYPES_H__
