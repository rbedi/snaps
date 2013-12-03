#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../image.h"
#include "iframeparser.h"
#ifdef STM32F4XX
    #include "ff.h"
#endif
#include "../image.h"

static uint32_t swap32(uint32_t x)
{
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
    return x;
}

void iframeparser(unsigned char* buffer, unsigned int j, unsigned int offsetarraylocation, unsigned int *iframelength)
{
    uint32_t offset, length;
    unsigned int bytes_read;

    FM_FILE *file;
	file = fm_open(file_name, readmode);
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", "iframe");
		return;
	}

    bytes_read = 1;
    fm_seek(file, offsetarraylocation+(j*4), SEEK_SET);
    fm_read(&offset,4, bytes_read, file);
    offset = swap32(offset);

    bytes_read = 1;
    fm_seek(file, offset, SEEK_SET);
    fm_read(&length,4, bytes_read, file);
    length = swap32(length) +3;
    *iframelength = length;

    bytes_read = 1;
    fm_seek(file, offset+4, SEEK_SET);
    fm_read(buffer,length,bytes_read , file);

    fm_close(file);
    return;
}
