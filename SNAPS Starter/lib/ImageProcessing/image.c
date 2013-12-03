//
// SNAPS Image Processing
// Requires 4Mb Memory
//

#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "H264Parser/iframeparser.h"
#include "H264Parser/avccparser.h"
#include "H264Decoder/lud.h"
#include "Encoder/bmp2jpg.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
FRAMECHAR CurrentFrame; //Main Memory Allocation

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
FRAMECONTAINER Frames[7500];

#ifdef STM32F4XX
    #pragma location = "SRAMSection" // Place the next variable in SRAM
#endif
unsigned char iframebuffer[522240];

int image_main(void)
{
    unsigned int total[1];
    total[0] = 0;
    unsigned char pps[4]; // PPS NAL Unit Data
    unsigned char sps[10]; // SPS NAL Unit Data
    unsigned int iframelength[1]; // Length of I-Frame NAL Unit
    unsigned int numberofframes[1]; //Number of I-Frames
    int iframearrayposition;  //Location of array with I-Frame Offsets
    int j;
    int images;
    iframearrayposition = avccparser(sps, pps, numberofframes); // 'Demux' Video, retrieve SPS, PPS, and Offset Data
    images = numberofframes[0]; // Set number of Iframes to Process
    printf("(image.c) Number of Frames %d \n",images);

    for(j=1;j<5;j++)
    {
        iframeparser(iframebuffer, j, iframearrayposition, iframelength); //Retrieve I-Frame NAL Unit
        lud(iframebuffer,iframelength, sps, pps,j); //Decode I-frame into RGB image
        edgedetect(j,total);
        printf("(image.c) Encoding Frame %i\n", j);

        // Only encode JPEG images that have corners
        if (total[0])
        {
            main_encoder(2,j);
        }

        printf(" \n\n(image.c) Final Count per Image: %d ",total[0]);
    }
    //printf(" \n\n(image.c) Final Count: %d ",total[0]);
    return (0);

}
