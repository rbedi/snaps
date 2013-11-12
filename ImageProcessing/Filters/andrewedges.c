#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void cornerdetect(unsigned char* buffer, int col2, int row2,unsigned char* numberofhits, unsigned char* rows, unsigned char* cols)
{
    unsigned char currarray2[32][32];
    unsigned int maxvalues[16];
    int row = 0;
    int col = 0;
    int k2 = 0;
    float k = 0;
    int maxrow = 0;
    int currentmax = 0;
    float theta = 0;
    int posx = 0;
    int posy = 0;
    int stposx = 0;
    int stposy = 0;
    int startpos = 0;
    float magnitude = 0;
    int totalvalue = 0;

    if(cols<50 && cols>40 && rows<11)
    {
        return;
    }

    for(row=0;row<32;row++)
    {
        for(col=0;col<32;col++)
        {
            currarray2[row][col] = buffer[row2*15360+col2*16+col+row*960];
            totalvalue = totalvalue + currarray2[row][col];
        }
    }


    if(totalvalue < 6000)
    {
        return;
    }

    for(k=0;k<16;k++)
    {
        maxrow = 0;
        theta = (3.14159/16)*k;

        for(startpos=0;startpos<32;startpos++)
        {
            currentmax = 0;
            magnitude = 0;

            if(theta<3.14159/2)
            {
                if(startpos<16)
                {
                    stposx=0;
                    stposy=startpos*2;
                }
                else
                {
                    stposx=(startpos-16)*2;
                    stposy = 0;
                }
            }
            else
            {
                if(startpos<16)
                {
                    stposx=0;
                    stposy=startpos*2;
                }
                else
                {
                    stposx=(startpos-16)*2;
                    stposy = 31;
                }

            }

            while(1)
            {

                int x = magnitude*sinf(theta);
                int y = magnitude*cosf(theta);
                posx = stposx + x;
                posy = stposy + y;

                if(posx>31 || posy>31 || posy<0 || posx<0 )
                {
                    break;
                }
                currentmax = currentmax + currarray2[posx][posy];
                magnitude = magnitude + 1;
            }

            if(currentmax > maxrow)
            {
                maxrow = currentmax;
            }

        }

        maxvalues[k2] = maxrow;
        //printf(" \n\nHERE:  %u ",maxvalues[k2]);
        k2 = k2 + 1;


    }
    for(k2=0;k2<8;k2++)
    {
        if(maxvalues[k2]>3300)
        {
            if(k2<7)
            {
                if(maxvalues[8+k2]>3300 || maxvalues[7+k2]>3300 || maxvalues[9+k2]>3300)
                {
                    rows[numberofhits[0]] = row2;
                    cols[numberofhits[0]] = col2;
                    numberofhits[0] = numberofhits[0] + 1;
                    break;

                }
            }
            else
            {
                if(maxvalues[8+k2]>3300 || maxvalues[7+k2]>3300 || maxvalues[0]>3300)
                {
                    rows[numberofhits[0]] = row2;
                    cols[numberofhits[0]] = col2;
                    numberofhits[0] = numberofhits[0] + 1;
                    break;
                }
            }


        }
    }


   /* if(maxrow1>3300 && maxrow2 >3300)
    {
        if(maxrow2pos == maxrow1pos + 7 || maxrow2pos == maxrow1pos + 8 || maxrow2pos == maxrow1pos + 9)
        {
            rows[numberofhits[0]] = row2;
            cols[numberofhits[0]] = col2;
            numberofhits[0] = numberofhits[0] + 1;
        }
    } */


    return;
}
