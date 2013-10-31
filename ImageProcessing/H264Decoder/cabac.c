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

#include <stdint.h>

static const uint8_t transIdxMPS[64]= {
  1, 2, 3, 4, 5, 6, 7, 8,
  9,10,11,12,13,14,15,16,
 17,18,19,20,21,22,23,24,
 25,26,27,28,29,30,31,32,
 33,34,35,36,37,38,39,40,
 41,42,43,44,45,46,47,48,
 49,50,51,52,53,54,55,56,
 57,58,59,60,61,62,62,63,
};

static const uint8_t transIdxLPS[64]= {
  0, 0, 1, 2, 2, 4, 4, 5,
  6, 7, 8, 9, 9,11,11,12,
 13,13,15,15,16,16,18,18,
 19,19,21,21,22,22,23,24,
 24,25,26,26,27,27,28,29,
 29,30,30,30,31,32,32,33,
 33,33,34,34,35,35,35,36,
 36,36,37,37,37,38,38,63,
};

uint8_t transIdxMPS_mod[128];
uint8_t transIdxLPS_mod[128];


void cabac_init_data()
{

  int i;
  for (i = 0; i < 64; ++i)
  {
    transIdxMPS_mod[2*i] = transIdxMPS[i]<<1;
    transIdxMPS_mod[2*i+1] = (transIdxMPS[i]<<1) +1;

    if (i)
    {
      transIdxLPS_mod[2*i] = transIdxLPS[i]<<1;
      transIdxLPS_mod[2*i+1] = (transIdxLPS[i]<<1) +1;
    }
    else
    {
      transIdxLPS_mod[2*i] = (transIdxLPS[i]<<1) +1;
      transIdxLPS_mod[2*i+1] = transIdxLPS[i]<<1;
    }
  }
}
