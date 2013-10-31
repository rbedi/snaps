#include <stdio.h>
#include <stdlib.h>
#include "../image.h"
#include "fillpixelmatrix.h"


void fillpixelmatrix(signed char *pixelmatrix, unsigned int col, unsigned char *rgbbuffer)
{
    int i;
    int j;
    for(i=0;i<16;i++)
    {
        for(j=0;j<16;j++)
        {
            pixelmatrix[(i*48)+(j*3)] = rgbbuffer[(i*48)+(j*3)] - 128;
            pixelmatrix[(i*48)+(j*3)+1] = rgbbuffer[(i*48)+(j*3)+1] - 128;
            pixelmatrix[(i*48)+(j*3)+2] = rgbbuffer[(i*48)+(j*3)+2] - 128;

        }
    }
    return;
}

