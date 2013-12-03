#include <stdio.h>
#include "populatearray.h"
#include "image.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif

/*
int readimage(FRAMECHAR *CurrentFrame, int g)
{
    CurrentFrame->width = Widths;
    CurrentFrame->height = Heights;
    CurrentFrame->position = g;
    int i,j,k;
    k = 2;
    if(k == 1)
    {
        for(i=0;i<Heights;i++)
        {
              for(j=0;j<Widths*3;j++)
              {
                if(j%3==0)
                {
                    CurrentFrame->framebits[i][j] = i*2;
                }
                else
                {
                    CurrentFrame->framebits[i][j] = 100;
                }

              }
        }
    }
    else
    {
        unsigned char buffer[Widths*3];
        FM_FILE* infile;
        char buffers[50];
        sprintf(buffers,"SDCard/Video/%d.bmp",g);
        infile = fm_open(buffers,readmode);
        unsigned int row, col;
        for(row = 0;row < Heights; row++) {
        fm_seek(infile,-(3*Widths*(row + 1)),SEEK_END);
                    //Read row from matrix
        fm_read(buffer, 1,Widths*3, infile);
            for(col = 0; col < Widths*3; col++) {\
               // printf("%d - %d \n",buffer[col], buffer[col]-128);
                CurrentFrame->framebits[row][col] = buffer[col];
            }
        }
    }
    return (0);
}
*/
