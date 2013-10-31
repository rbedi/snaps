#include <stdio.h>
#include <stdlib.h>
#include "image.h"

void sortimagearray(unsigned short *positionarray1, unsigned short *ratingarray1, unsigned short *positionarray2, unsigned short *ratingarray2)
{
    int i;
    int currentrating;
    int position;
    int j;
 //   int searchrate;
//    int searchpos;
    j = 0;
    while(1)
    {
        currentrating = 0;
        i = 0;
        while(&ratingarray1[i] != NULL)
        {
            if(ratingarray1[i]>currentrating && ratingarray1[i]<65500 )
            {
                currentrating = ratingarray1[i];
                position = i;
            }
            i = i + 1;
        }
        if(currentrating==0)
        {
            break;
        }
       // searchpos = positionarray1[j];
       // searchrate = ratingarray1[j];
       // positionarray1[j] = position;
        //ratingarray1[j] = currentrating;
       // positionarray1[position] = searchpos;
        //ratingarray1[position] = searchrate;
        ratingarray2[j] = currentrating;
        positionarray2[j] = position;
        ratingarray1[position] = 65530;
        printf("Frame:%d , Rating:%d \n",positionarray2[j] ,ratingarray2[j]);
        j = j + 1;

    }
    return;
}
