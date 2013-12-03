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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ludh264.h"
#include "../image.h"
#include "lud.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif

//uint8_t* streamBuffer = (unsigned char*)CurrentFrame.framebits;
#define min(a, b) ((a)<(b) ? (a) : (b))
#define max(a, b) ((a)>(b) ? (a) : (b))

//#define BENCHMARK


int lud(unsigned char* streamBuffer, unsigned int *iframelength, unsigned char *spsbuffer, unsigned char *ppsbuffer, int j)
{

  unsigned int lengthsps[1];
  unsigned int lengthpps[1];

  *lengthsps=10;
  *lengthpps=4;

  ludh264_init();

  Picture* pic;

   //if(j==0)
   {
        ludh264_get_frame(&pic, spsbuffer, lengthsps);
        ludh264_get_frame(&pic, ppsbuffer, lengthpps);
   }
   printf("\nNEW FRAME\n------------\nDecoding Frame %d\n",j);

   ludh264_get_frame(&pic, streamBuffer, iframelength);


 return 0;
}
