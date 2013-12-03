#include "../image.h"
#include <math.h>
#include "circledetect.h"


unsigned char circlearray[circledetectmatrixsize][circledetectmatrixsize*3];
static int wide = 2*Widths/(circledetectmatrixsize);
static int high = 2*Heights/circledetectmatrixsize;

int pixeliswhite(int r, int g, int b)
{
    //printf("f %d %d %d",r,g,b);
    int ave,threshold;
    ave = (r+g+b)/3;
    threshold = ave*whitefilterthreshold;
    if(r==255 && g == 255 && b == 255)
    {
        return(0);
    }

    if(r>whitefilterred && g>whitefiltergreen && b>whitefilterblue)
    {

         if(r<ave+threshold && r>ave-threshold && b<ave+threshold && b>ave-threshold && g<ave+threshold && g>ave-threshold )
         {

             return(1);
         }
         return(0);
    }
    return(0);
}

/*void fillblock(FRAMECHAR *CurrentFrame, int y_block, int x_block)
{
    int row;
    int col;
    for(row=0;row<circledetectmatrixsize;row++)
    {
        for(col=0;col<circledetectmatrixsize*3;col++)
        {
            circlearray[row][col] = CurrentFrame->framebits[(circledetectmatrixsize*y_block)/2+row][(circledetectmatrixsize*3*x_block)/2+col];
            printf(" %d " ,circlearray[row][col] );
        }
    }
    return;
} */

int findwhite(void)
{
//    int j;
    int currentrating;
    currentrating = 0;
    int pixel1,pixel2,pixel3,pixel4,currentpixel;
    pixel1 = pixeliswhite(circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2-1],circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2-2],circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2-3]);
    pixel2 = pixeliswhite(circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2+2],circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2+1],circlearray[circledetectmatrixsize/2-1][circledetectmatrixsize*3/2]);
    pixel3 = pixeliswhite(circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2-1],circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2-2],circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2-3]);
    pixel4 = pixeliswhite(circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2+2],circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2+1],circlearray[circledetectmatrixsize/2][circledetectmatrixsize*3/2]);
    if(pixel1 && pixel2 && pixel3 && pixel4)
    {
        int theta;
        int radius;
        int x;
        int y;
        float theta2;
//        int broken;

        for(radius=2;radius<30;radius++)
        {
            for(theta=1;theta<radius*6;theta++)
            {
                theta2=1.0*theta/radius;
                x=round(sinf(theta2)*radius) + 32;
                y=round(cos(theta)*radius) + 32;

                currentpixel = pixeliswhite(circlearray[y][(x*3)+2],circlearray[y][(x*3)+1],circlearray[y][(x*3)]);
                if(currentpixel==0)
                {
                    break;
                }

            }
            currentrating = currentrating + 1;
            if(currentpixel==0)
            {
                break;
            }
        }
    }
    return(currentrating);
}

int circledetect(FRAMECHAR *CurrentFrame, int *earthdata)
{
    int rowblock;
    int colblock;
    int row;
    int col;
    int currentrating;
    float horizontalslope;
    currentrating = 0;

    if(earthdata[0]>0 && earthdata[2]>0)
    {
        horizontalslope = 1.0*(earthdata[2]-earthdata[0])/CurrentFrame->width;
        //printf("%f , \n",horizontalslope);
    }
    else if(earthdata[0]>0 && earthdata[2]<1)
    {
        if(earthdata[8]==-1) //TODO
        {
             if(earthdata[6]==0)
             {
                earthdata[6] = CurrentFrame->width/8;
             }
             horizontalslope = -1.0*(-earthdata[0])/(CurrentFrame->width);
             earthdata[1] = 1;
        }

        else
        {
             if(earthdata[6]==0)
             {
                earthdata[6] = CurrentFrame->width/8;
             }
             horizontalslope = -1.0*(-earthdata[0])/(CurrentFrame->width);
             earthdata[1] = 1;
        }


    }
    else if(earthdata[0]<1 && earthdata[2]>0)
    {
        if(earthdata[8]==-1) // TODO
        {
            if(earthdata[6]==0)
            {
            earthdata[6] = CurrentFrame->width*7/8;
            }
            horizontalslope = -1.0*(earthdata[2])/(CurrentFrame->width-earthdata[6]);
            earthdata[0]=earthdata[2] - earthdata[6]*horizontalslope;
            earthdata[1] = 1;
        }
        else
        {
            if(earthdata[6]==0)
            {
            earthdata[6] = CurrentFrame->width*7/8;
            }
            horizontalslope = -1.0*(earthdata[2])/(CurrentFrame->width-earthdata[6]);
            earthdata[0]=earthdata[2] - earthdata[6]*horizontalslope;
            earthdata[1] = 1;
        }

    }
    else
    {
        horizontalslope = 6;
        earthdata[0] = CurrentFrame->height;
        earthdata[1] = 1;
    }

  //  printf("%f  \n",horizontalslope);
    for(rowblock=0;rowblock<high-1;rowblock++)
    {
        for(colblock=0;colblock<wide-1;colblock++)
        {
                for(row=0;row<circledetectmatrixsize;row++)
                {
                    for(col=0;col<circledetectmatrixsize*3;col++)
                    {
                        if(earthdata[1]*(1.0*(rowblock)*circledetectmatrixsize/2+row) < earthdata[1]*(1.0*earthdata[0]+1.0*horizontalslope*(colblock/2*circledetectmatrixsize+col)))

                        {
                             circlearray[row][col] = CurrentFrame->framebits[(circledetectmatrixsize*rowblock)/2+row][(circledetectmatrixsize*3*colblock)/2+col];

                        }
                        else
                        {
                            circlearray[row][col] = 0;
                            CurrentFrame->framebits[(circledetectmatrixsize*rowblock)/2+row][(circledetectmatrixsize*3*colblock)/2+col] = 255;

                        }
                    }
                }

                currentrating = currentrating + findwhite();

        }
    }

    if(currentrating>250)
    {
        currentrating=250;
    }

    currentrating = 10*pow(currentrating,0.5);
    return(currentrating);

}
