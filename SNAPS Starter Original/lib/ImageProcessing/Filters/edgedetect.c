#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../image.h"
#include "edgedetect.h"

#include <string.h>

#include "imageio.h"
#include "fast-edge.h"

struct image img, img_gauss, img_out; //img_scratch, img_scratch2,

void readpicture(unsigned char* buffy, int j)
{
    unsigned char filename[50];
    FILE* reader;
    sprintf(filename,"SatImages/%d.pgm",4);
    reader = fopen(filename,"rb");
    fseek(reader,56,SEEK_SET);
    fread(buffy,522240,1,reader);
    fclose(reader);
    return;

}

int canny(int j)
{

	int w, h, i,u;
    unsigned char yo[522240];

	u = 0;

	if(u==1)
	{
        readpicture(yo,j);
        img.pixel_data = yo;
	}
	else
	{
       //img.pixel_data =  CurrentFrame.framebits;
	}

    w = 960;
    h = 544;
    img.width = w;
    img.height = h;

    img_out.width = 960;
    img_out.height = 544;

    unsigned char *img_gauss_data = malloc(w * h * sizeof(char));
    img_gauss.pixel_data = img_gauss_data;
    gaussian_noise_reduce(&img, &img_gauss);
    //printf("*** performing morphological closing ***\n");
    //morph_close(&img, &img_scratch, &img_scratch2, &img_gauss);
    canny_edge_detect(&img_gauss, &img);
    write_pgm_image(&img,j);
    free(img_gauss_data);
	return(1);
}

void savegreyscale(int q)
{
  int i;
  int j;

  unsigned char header[19];
  header[0] = 80;
  header[1] = 53;
  header[2] = 13;
  header[3] = 10;

  header[4] = 57;
  header[5] = 54;
  header[6] = 48;

  header[7] = 13;
  header[8] = 10;
  header[9] = 53;
  header[10] = 52;
  header[11] = 52;

  header[12] = 13;
  header[13] = 10;
  header[14] = 50;
  header[15] = 53;
  header[16] = 53;
  header[17] = 13;
  header[18] = 10;

  FILE* testfile;
  char filename[50];
  sprintf(filename,"Greyscale/%d.pgm",q);
  testfile = fopen(filename,"wb");
  fwrite(header,19,1,testfile);
  fwrite((unsigned char*)CurrentFrame.framebits,522240,1,testfile);
  printf("GreyScale Image Created!\n");
  fclose(testfile);
  return;
}



int edgedetect(int j,unsigned int* total)
{
  int i = 0;
  int k = 0;
  int averow = 0;
  int avecol = 0;
  int diffrow = 0;
  int diffcol = 0;
  int confirmed = 0;
  int confirmed2 = 0;
  int position = 0;

  unsigned char rows[100];
  unsigned char cols[100];
  unsigned char rows2[100];
  unsigned char cols2[100];

  unsigned char numberofhits[1] = {0};
  //savegreyscale(j);

  canny(j);

  for(i=0;i<59;i++)
  {
      for(k=0;k<33;k++)
      {
          if(numberofhits[0]==100)
          {
              return 1;
          }

          cornerdetect(img.pixel_data,i,k,numberofhits,rows,cols);
      }
  }

  if(numberofhits[0]>0)
  {
      for(i=0;i<numberofhits[0];i++)
      {
            averow = averow + rows[i];
            avecol = avecol + cols[i];
      }

      averow = averow/numberofhits[0];
      avecol = avecol/numberofhits[0];

      for(i=0;i<numberofhits[0];i++)
      {
          diffrow = abs(averow-rows[i]);
          diffcol = abs(avecol-cols[i]);
          if(diffcol < 6 && diffrow < 6)
          {
              rows2[position] = rows[i];
              cols2[position] = cols[i];
              confirmed2 = confirmed2 + 1;
              position = position + 1;

          }
      }

      numberofhits[0] = confirmed2;
  }

  averow = 0;
  avecol = 0;

  if(numberofhits[0]>0)
  {
      for(i=0;i<numberofhits[0];i++)
      {
            averow = averow + rows2[i];
            avecol = avecol + cols2[i];
            printf(" %d %d ",rows2[i],cols2[i]);
      }

      averow = averow/numberofhits[0];
      avecol = avecol/numberofhits[0];

      for(i=0;i<numberofhits[0];i++)
      {
          diffrow = abs(averow-rows2[i]);
          diffcol = abs(avecol-cols2[i]);
          if(diffcol < 4 && diffrow < 4)
          {
              confirmed = confirmed + 1;
              total[0] = total[0] + 1;
          }
      }

      numberofhits[0] = confirmed;
  }


  printf("\nResults\n\nHits: %d \nConfirmed: %d \n\n",numberofhits[0],confirmed);
  return 0;
}
