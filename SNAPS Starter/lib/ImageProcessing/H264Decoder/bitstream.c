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
#include "bitstream_types.h"
#include "intmath.h"
#include <string.h>

int allocate_sub_table(VLCReader* vlc, unsigned int prefix, unsigned int prefix_length,
                                const uint8_t* code_table, const uint8_t* length_table, unsigned int nb_of_symbols)
{
  unsigned int subtable_max_length = 0;
  unsigned int i;
  unsigned int table_size;


  for (i = 0; i < nb_of_symbols; i++)
  {
    if ((length_table[i] > prefix_length) && (((unsigned int) code_table[i]
        >> (length_table[i] - prefix_length)) == prefix))
    {
      subtable_max_length = im_max(length_table[i] - prefix_length, subtable_max_length);
    }
  }
  table_size = 1<<subtable_max_length;
  LUD_DEBUG_ASSERT(vlc->nb_of_subtables<255); // max number of sub tables If you are breaking here this alogithm has to be redone in order to support more subtables
  if (vlc->nb_of_subtables)
  {
    //vlc->subtable = realloc(vlc->subtable, (vlc->nb_of_subtables+1)*sizeof(*vlc->subtable));
  }

  assert(table_size <= 256);
  //vlc->subtable[vlc->nb_of_subtables].code_symbol = malloc(table_size*2);

  if (!vlc->subtable[vlc->nb_of_subtables].code_symbol)
    return -1;
  vlc->subtable[vlc->nb_of_subtables].code_length = (int8_t*)vlc->subtable[vlc->nb_of_subtables].code_symbol + table_size;
  vlc->subtable[vlc->nb_of_subtables].table_size = table_size;
  memset(vlc->subtable[vlc->nb_of_subtables].code_symbol, -1, table_size);
  memset(vlc->subtable[vlc->nb_of_subtables].code_length, 0, table_size);
  vlc->nb_of_subtables++;
  return subtable_max_length;
}


int init_vlc(VLCReader* vlc, unsigned int nb_of_symbols, const uint8_t* symbol_table, const uint8_t* code_table, const uint8_t* length_table, unsigned int nb_of_bits)
{
  unsigned int i, j;
  unsigned int table_size = 1<<nb_of_bits;
  unsigned int max_length = 0;
  unsigned int len, code, symbol;

  //vlc->subtable = 0;

  // Check that the entries are not too long and find the longest code
  for (i=0; i<nb_of_symbols; i++)
  {
    len = length_table[i];
    if (nb_of_bits*2 < len)
    {
      LUD_DEBUG_ASSERT(nb_of_bits*2 >= len); // We only allow one indirection when processing VLC codes...
      return -1;
    }
    max_length = im_max(max_length, len);
  }
  nb_of_bits = im_min(nb_of_bits, max_length); // ensure that we are not over allocating the VLC table

  vlc->bits = nb_of_bits;
  assert(table_size <= 512);
  //vlc->table.code_symbol = malloc(table_size*2);
  if (!vlc->table.code_symbol)
  {
    LUD_DEBUG_ASSERT(vlc->table.code_symbol);
    return -1;
  }
  vlc->table.code_length = (int8_t*)vlc->table.code_symbol + table_size;
  vlc->nb_of_subtables = 0;

  memset(vlc->table.code_symbol, -1, table_size);
  memset(vlc->table.code_length, 0, table_size);


  for (i=0; i<nb_of_symbols; i++)
  {
    unsigned int index;
    len = length_table[i];
    code = code_table[i];
    symbol = symbol_table[i];
    if (len <= nb_of_bits)
    {
      // easy, the code is put in the primary table !
      index = code << (nb_of_bits-len);
      if (vlc->table.code_symbol[index] != (uint8_t)-1)
      {
        LUD_DEBUG_ASSERT(vlc->table.code_symbol[index] == (uint8_t)-1); // the entry should be free. Otherwise there is something wrong
        return -1;
      }
      vlc->table.code_symbol[index] = symbol;
      vlc->table.code_length[index] = len;
    }
    else
    {
      int sub_length;
      unsigned int prefix, subtable_id;
      len -= nb_of_bits;
      prefix = code >> len;
      // the entry is longer than the table size, this entry will need a sub table...
      // check if the sub table is already allocated
      if (vlc->table.code_length[prefix] == 0)
      {
        // the table is not allocated
        // we need to reallocate it
        sub_length = allocate_sub_table(vlc, prefix, nb_of_bits, code_table, length_table, nb_of_symbols);
        if (sub_length < 0 )
        {
          LUD_DEBUG_ASSERT(sub_length > 0);
          return -1;
        }
        subtable_id = vlc->table.code_symbol[prefix] = vlc->nb_of_subtables-1;
        vlc->table.code_length[prefix] = -sub_length;
      }
      else
      {
        subtable_id = vlc->table.code_symbol[prefix];
        sub_length = -vlc->table.code_length[prefix];
      }
      // Fills in the sub table: just insert the new symbol
      if (sub_length <= 0) // if length is positive it would mean a real symbol and not a subtable, this should not happen !
      {
        LUD_DEBUG_ASSERT(sub_length > 0);
        return -1;
      }
      index =  (( code << (sub_length-len) ) & ((1<<sub_length)-1) );
      vlc->subtable[subtable_id].code_symbol[index] = symbol_table[i];
      vlc->subtable[subtable_id].code_length[index] = length_table[i]-nb_of_bits;
    }
  }

#ifndef NDEBUG
  vlc->total_size = table_size*2;
#endif // NDEBUG
  // Fills in the rest of the values
  symbol = 0;
  len = 0;
  //printf("Main table\n");
  for(i=0; i < table_size; i++)
  {
    if (vlc->table.code_length[i] == 0)
    {
      vlc->table.code_symbol[i] = symbol;
      vlc->table.code_length[i] = len;
    }
    else
    {
      symbol = vlc->table.code_symbol[i];
      len = vlc->table.code_length[i];
    }
    //printf("Line %4d = %02x, Symbol %4d, Length %2d\n", i, i, vlc->table.code_symbol[i], vlc->table.code_length[i]);
  }
  for( j=0; j < vlc->nb_of_subtables; j++)
  {
    symbol = 0;
    len = 0;
    //printf("Sub table %d, Size %d\n", j, vlc->subtable[j].table_size);
    for(i=0; i < vlc->subtable[j].table_size; i++)
    {
      if (vlc->subtable[j].code_length[i] == 0)
      {
        vlc->subtable[j].code_symbol[i] = symbol;
        vlc->subtable[j].code_length[i] = len;
      }
      else
      {
        symbol = vlc->subtable[j].code_symbol[i];
        len = vlc->subtable[j].code_length[i];
      }
      //printf("Line %4d = %02x, Symbol %4d, Length %2d\n", i, i, vlc->subtable[j].code_symbol[i], vlc->subtable[j].code_length[i]);
    }
#ifndef NDEBUG
    vlc->total_size += vlc->subtable[j].table_size*2;
#endif // NDEBUG
  }
  return 0;
}




