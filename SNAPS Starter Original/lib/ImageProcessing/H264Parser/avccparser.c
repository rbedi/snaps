#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../image.h"
#include "commonparser.h"
#include "../image.h"
#include "avccparser.h"

#ifdef STM32F4XX
    #include "ff.h"
#endif



int getavccdata(unsigned char *buffer, FM_FILE *file, unsigned char *sps1, unsigned char *pps1, unsigned int *stbloffsets)
{

    fm_seek(file, stbloffsets[7]-4, SEEK_SET);
    unsigned int bytes_read;

    bytes_read = 1;
    fm_read(buffer,4, bytes_read, file);
    unsigned char arr[4] = {buffer[0], buffer[1], buffer[2], buffer[3]};
    unsigned int length = arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];

    bytes_read = 1;
    fm_seek(file, stbloffsets[7]+10, SEEK_SET);
    fm_read(buffer,length, bytes_read, file);

    unsigned char arr2[4] = {buffer[0], buffer[1]};
    unsigned short spslength = arr2[0] << 8 | arr2[1];
    unsigned char arr3[4] = {buffer[spslength+3], buffer[spslength+4]};
    unsigned short ppslength = arr3[0] << 8 | arr3[1];


   // printf("\nSPS\n--------------------------------");
    //printf("\nLength: %d", spslength);
    //printf("\nFirst 10 Bytes: ");
    int i;
    for(i=2;i<spslength+2;i++)
    {
        sps1[i-2]=buffer[i];
        //printf(" %.2X  ", (int)sps1[i-2]);
    }

    //printf("\n\n\nPPS\n--------------------------------");
    //printf("\nLength: %d", ppslength);
    //printf("\nFirst 10 Bytes: ");
    int j;
    int w;
    for(j=spslength+5;j<ppslength+spslength+5;j++)
    {
        w = j-(spslength+5);
        pps1[w]=buffer[j];
        //printf(" %.2X  ", (int)pps1[w]);
    }
    //printf("\n \n\n");
    return 0;
}

int findmdataatom(unsigned int pos, unsigned char *buffer)
{
    int j;
    j = 0;
    while(1)
    {
        if(buffer[j] == 0x6D && buffer[j+1] == 0x64 && buffer[j+2] == 0x61 && buffer[j+3] == 0x74)
        {
            break;
        }

        j++;
        pos++;
    }
    return pos;
}


int getatomlength(unsigned char *buffer, int pos)
{
    unsigned char arr[4] = {buffer[pos-4], buffer[pos-3], buffer[pos-2], buffer[pos-1]};
	unsigned int size = arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];

	if(size == 1)
	{

        unsigned char array[8] = {buffer[pos+4], buffer[pos+5], buffer[pos+6], buffer[pos+7], buffer[pos+8], buffer[pos+9], buffer[pos+10], buffer[pos+11]};
        int64_t size = (int64_t)array[0] << 56 | (int64_t)array[1] << 48 | (int64_t)array[2] << 40 | (int64_t)array[3] << 32 | array[4] << 24 | array[5] << 16 | array[6] << 8 | array[7];
	    return size;
	}
    return size;

}

void findatomoffsets(unsigned char *buffer, FM_FILE *file, unsigned int mdatoffset, unsigned int mdatlength, unsigned int *stbloffsets, unsigned int *stbllengths)
{
    unsigned int sz;
    unsigned int currentposition;
    unsigned int bytes_read;



    #ifdef STM32F4XX
        sz = file->fsize;
    #else
        fm_seek(file, 0L, SEEK_END);
        sz = fm_tell(file);
    #endif

    int p;
    p = 0;
    //129776604

    currentposition = mdatlength + mdatoffset + 4;
    while(currentposition < sz)
   // while(p<90)
    {

        p = p + 1;
        if(p>60)
        {
            break;
        }
        bytes_read = 1;
        fm_seek(file, currentposition, SEEK_SET);
        fm_read(buffer,100, bytes_read, file);
        unsigned char arr[4] = {buffer[0], buffer[1], buffer[2], buffer[3]};
        unsigned int length = arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];
        unsigned char arr2[4] = {buffer[4], buffer[5], buffer[6], buffer[7]};
        unsigned int tag = arr2[0] << 24 | arr2[1] << 16 | arr2[2] << 8 | arr2[3];
        if(tag == moov || tag == trak || tag == mdia || tag == minf || tag == stbl)
        {
            currentposition = currentposition + 8;

        }
        else if(tag == stts)
        {

            stbloffsets[0] = currentposition;
            stbllengths[0] = length;
            currentposition = currentposition + length;
        }
        else if(tag == stss)
        {

            stbloffsets[1] = currentposition;
            stbllengths[1] = length;
            currentposition = currentposition + length;
        }
        else if(tag == stsd && stbloffsets[2]==0)
        {

            stbloffsets[2] = currentposition;
            stbllengths[2] = length;
            currentposition = currentposition + length;
            stbloffsets[7]=currentposition+106-length;

        }
        else if(tag == avcc)
        {

            stbloffsets[7] = currentposition;
            stbllengths[7] = length;
            currentposition = currentposition + length;
        }
        else if(tag == stsz && stbloffsets[3] == 0)
        {

            stbloffsets[3] = currentposition;
            stbllengths[3] = length;
            currentposition = currentposition + length;
        }
        else if(tag == stsc)
        {
            stbloffsets[4] = currentposition;
            stbllengths[4] = length;
            currentposition = currentposition + length;
        }
        else if(tag == stco && stbloffsets[6]==0)
        {
             stbloffsets[6] = currentposition;
             currentposition = currentposition + length;

        }
        else if(tag == stco)
        {
            stbloffsets[5] = currentposition;
            stbllengths[5] = length;
            currentposition = currentposition + length;
        }
        else if(tag == udta)
        {
            break;
        }

        else
        {
            currentposition = currentposition + length;
        }

    }
    bytes_read = 1;
    fm_seek(file, stbloffsets[3]+16, SEEK_SET);
    fm_read(buffer,10, bytes_read, file);
    return;
}

int readjump(unsigned char *buffer, FM_FILE *file, unsigned int *stbloffsets)
{
    unsigned int bytes_read;
    bytes_read = 1;
    fm_seek(file, stbloffsets[6]+12, SEEK_SET);
    fm_read(buffer,4, bytes_read, file);

    unsigned char arr[4] = {buffer[0], buffer[1], buffer[2], buffer[3]};
    unsigned int offset = arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];
    return offset;
}


int avccparser(unsigned char *sps, unsigned char *pps, unsigned int *numberofframes)
{
    unsigned int pos;
    unsigned char buffer[500];
    unsigned int atomsize;
    unsigned int currentposition;
    unsigned int startposition;
    unsigned int k;
    unsigned char sps1[10];
    unsigned char pps1[4];
    unsigned int stbloffsets[8] = {0};
    unsigned int stbllengths[8] = {0};
    unsigned int bytes_read;


    FM_FILE *file;
	file = fm_open(file_name, readmode);
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", "video");
		return 0;
	}

	pos = 0;
	bytes_read = 1;
    fm_seek(file, 0, SEEK_SET);
    fm_read(buffer,500, bytes_read, file);
    pos = findmdataatom(pos, buffer);
    atomsize = getatomlength(buffer, pos);
    pos = pos + 4;
    currentposition = pos;
    findatomoffsets(buffer, file, pos-4, atomsize, stbloffsets, stbllengths);
    getavccdata(buffer, file, sps1, pps1, stbloffsets);
    numberofframes[0] = readjump(buffer, file, stbloffsets);
    startposition = stbloffsets[6]+16;

    for(k=0;k<10;k++)
    {
             sps[k]=sps1[k];

    }

    for(k=0;k<4;k++)
    {

             pps[k]=pps1[k];
    }
    fm_close(file);
    return startposition;
}


