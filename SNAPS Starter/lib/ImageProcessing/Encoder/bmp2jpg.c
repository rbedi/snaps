#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ejpgl.h"
#include "bmp2jpg.h"
#include "ColorConversion.h"
#include "dct.h"
#include "huffman.h"
#include "jpeg.h"
#include "zzq.h"
#include "../image.h"
#include "../Converter/fillpixelmatrix.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif
#include "../image.h"

JPEGHEADER _jpegheader;
FM_FILE* outfiles;
//static unsigned char buffer[MACRO_BLOCK_SIZE*3];

signed char YMatrix[MATRIX_SIZE][MATRIX_SIZE];
signed char CrMatrix[MATRIX_SIZE][MATRIX_SIZE];
signed char CbMatrix[MATRIX_SIZE][MATRIX_SIZE];

void writejpegfooter()
{
       unsigned char footer[2];
       unsigned int bytes_written;
       bytes_written = 1;
       footer[0] = 0xff;
       footer[1] = 0xd9;
       fm_write(footer,sizeof(footer),bytes_written,outfiles);
//       fm_flush(outfiles);
	return;

}

int openBMPJPG(int position)
{

    int jpegheadersize;
    char filename[50];
    sprintf(filename,"%d.jpg",position);
    outfiles = fm_open(filename, writemode);
    if (!outfiles)
    {
            printf("Error Saving File");
            exit(0);
    }


    	jpegheadersize = writejpegheader(&_jpegheader);
	if (jpegheadersize == 0) {
       	printf("\nerror in writing jpg header");
		exit(0);
		}
        unsigned int bytes_written;
        bytes_written = 1;
    	fm_write(&_jpegheader,jpegheadersize,bytes_written,outfiles);
//    	 fm_flush(outfiles);

  	return 1;

}


int closeBMPJPG() {
        printf("Encoding Success! \n \n");
     	writejpegfooter();
        fm_close(outfiles);
	 return 0;

}

void put_char(unsigned char c) {
    unsigned int bytes_written;
    bytes_written = 1;
	fm_write(&c, 1, bytes_written, outfiles);
//	 fm_flush(outfiles);

}


int main_encoder(int argc, int j)
{


    int sample;
    unsigned int col, cols,  rows;
    int row;

    int jpegheadersize;
    char filename[50];
    sprintf(filename,"%d.jpg",j);
    outfiles = fm_open(filename, writemode);
    if (!outfiles)
    {
            printf("Error Saving File");
            exit(0);
    }


    	jpegheadersize = writejpegheader(&_jpegheader);
	if (jpegheadersize == 0) {
       	printf("\nerror in writing jpg header");
		exit(0);
		}
        unsigned int bytes_written;
        bytes_written = 1;
    	fm_write(&_jpegheader,jpegheadersize,bytes_written,outfiles);
//    	 fm_flush(outfiles);


    unsigned char tempbuffer[768];

    signed char pixelmatrix[MACRO_BLOCK_SIZE][MACRO_BLOCK_SIZE*3];
    rows = 1088/MACRO_BLOCK_SIZE;
    cols = 1920/MACRO_BLOCK_SIZE;


    dct_init_start();
//    zzq_encode_init_start(compression);
    vlc_init_start();
    FM_FILE* rgbfile;
    rgbfile = fm_open("rgbs.dat", readmode);
    unsigned int bytes_read;


    for (row = 0; row < rows; row++)
    {


        for (col = 0; col < cols; col++)
        {
            fm_seek(rgbfile,92160*row+768*col, SEEK_SET);
            bytes_read=1;
            fm_read(tempbuffer,768, bytes_read, rgbfile);

            fillpixelmatrix((signed char*)pixelmatrix,col,tempbuffer);

            for(sample=0;sample<5;sample++)
            {
                if(sample<4)
                 {
                    RGB2YCrCb(pixelmatrix,YMatrix,CrMatrix,CbMatrix,sample);
                    dct(YMatrix,0);
                  }
                  else
                  {
                    dct(CrMatrix,1);
                    dct(CbMatrix,2);
                  }
            }
        }
    }


    fm_close(rgbfile);
	vlc_stop_done();

	//closeBMPJPG();




    printf("Encoding Success! \n \n");

    unsigned char footer[2];
    bytes_written = 1;
    footer[0] = 0xff;
    footer[1] = 0xd9;
    fm_write(footer,sizeof(footer),bytes_written,outfiles);
    //fm_flush(outfiles);

    fm_close(outfiles);

	return 0;

}


