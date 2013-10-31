/***************************************************************************
 *                                                                         *
 *     Copyright (C) 2008  ludrao.net                                      *
 *     ludh264@ludrao.net                                                  *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "ludh264.h"

#define WB_SIZE 2048*1024
uint8_t streamBuffer[WB_SIZE+4];
#define min(a, b) ((a)<(b) ? (a) : (b))
#define max(a, b) ((a)>(b) ? (a) : (b))

//#define BENCHMARK

void write_pic(Picture* pic, FILE* out)
{
#ifndef BENCHMARK
  unsigned int written;
  written = fwrite(pic->Y, 1, pic->image_buffer_size, out);
  if (written != pic->image_buffer_size)
  {
    printf("Failed while writing output file\n");
    exit(-1);
  }
#endif // BENCHMARK

}

int test_bench()
{
#include "motion_compensation_types.h"
  void luma_sample_interpolation_abc_4xH(int xFrac,
      const pixel_t* src, int src_dy, pixel_t* dst, int dst_dy, int h);


  unsigned long nb, i, j;
  pixel_t* src0;
  pixel_t* src1;
  pixel_t* dst0;
  pixel_t* dst1;

#define WIDTH 512
#define HEIGTH 512
#define SIZE  (WIDTH * HEIGTH)
  src0 = malloc(SIZE);
  src1 = malloc(SIZE);
  dst0 = malloc(SIZE);
  dst1 = malloc(SIZE);
  unsigned int src_dy = WIDTH;
  unsigned int dst_dy = WIDTH;
  int o;

  srand(101);

  for (nb = 0; nb < SIZE; ++nb)
  {
    src0[nb] = rand();
    src1[nb] = rand();
  }


/*
  nb = 0;
  for (j = 0; j < HEIGTH; ++j)
  {
    for (i = 0; i < WIDTH; ++i)
    {
      src0[nb] = src1[nb] = j;
      nb++;
    }
  }
*/
/*

  nb = 0;
  for (j = 0; j < HEIGTH; ++j)
  {
    for (i = 0; i < WIDTH; ++i)
    {
      src0[nb] = src1[nb] = i;
      nb++;
    }
  }
*/
  memset(dst0, 0, SIZE);
  memset(dst1, 0, SIZE);


#if 1
  extern const qpel_func_t qpel_func_sse2[4][4][4]; // [partWidth4-1][xFrac][yFrac]
  extern const qpel_func_t qpel_func_c[4][4][4]; // [partWidth4-1][xFrac][yFrac]
  extern const hpel_func_t hpel_func_sse2[3][4][2][2]; // [cfidc-1][partWidth4-1][xFrac!=0][yFrac!=0]
  extern const hpel_func_t hpel_func_c[3][4][2][2]; // [cfidc-1][partWidth4-1][xFrac!=0][yFrac!=0]

  int w, h, xFrac, yFrac;
  for (w = 1; w <= 4; ++w)
  {
    for (h = max(1, w - 1); h <= min(w + 1, 4); ++h)
    {
      if (w==3 || h == 3)
        continue;

      for (xFrac = 0; xFrac < 4; ++xFrac)
      {
        for (yFrac = 0; xFrac < 4; ++xFrac)
        {
          printf("Doing: w:%d, h:%d, xFrac:%d, yFrac:%d\n", w, h, xFrac, yFrac);

          for (nb = 0; nb < 1000; ++nb)
          {
            int x, y;
            x = rand() & 7;
            y = rand() & 7;
            o = rand() % 512;
            //o = 0;
            o += WIDTH * 2 + 2;

            //qpel_interpolation_21_16xH(4, src0+o, src_dy, dst0, dst_dy-16, 0);
            hpel_func_c[0][w-1][xFrac!=0][yFrac!=0](h, xFrac, yFrac, src0 + o, src_dy, dst0, dst_dy - 16);
            hpel_func_sse2[0][w-1][xFrac!=0][yFrac!=0](h, xFrac, yFrac, src0 + o, src_dy, dst1, dst_dy - 16);

            int c = 0;

            while (c < SIZE && dst0[c] == dst1[c])
              c++;

            if (c < SIZE)
            {
              printf("Failed: iteration:%d, count=%d, v0:%d, v1:%d\n", nb, c, dst0[c], dst1[c]);
              return 0;
            }
          }
        }
      }
    }

  }

  printf("Done\n");
  return 0;
#endif

  for (nb = 0; nb < 50000000; ++nb)
  //for (nb = 0; nb < 100000; ++nb)
  {
    o = (rand() % ((WIDTH-16) * (HEIGTH-16)));

    //luma_sample_interpolation_20_16xH(src0+o, src_dy, dst0, dst_dy, 4);
    //luma_sample_interpolation_20_16xH_comp(src0+o, src_dy, dst0, dst_dy, 4);
    //qpel_func[3][0](4, 1, src0, src_dy, dst1, dst_dy, 255);
    //qpel_func[3][2](1, 2, src0+o, src_dy, dst0, dst_dy, 255);
    qpel_interpolation_00_16xH(4, src0+o, src_dy, dst0, dst_dy, 0);
  }

  return 0;
}


int main(int argc, char *argv[])
{

  //return test_bench();

  FILE* in;
  FILE* out;
  struct timeval prev;
  struct timeval curr;
  uint32_t t;

  int nb_of_nalu=0;

 // argc = argc;
 // argv = argv;


  // Test P/B mv pred:
  // CVBS3_Sony_C.jsv
  // BA3_SVA_C.264
  // SL1_SVA_B.264
  // NL3_SVA_E.264
  // cvmp_mot_frm0_full_B.26l
  // cvmp_mot_fld0_full_B.26l
  // cvmp_mot_picaff0_full_B.26l
  // cvmp_mot_mbaff0_full_B.26l

  // CVMA1_TOSHIBA_B.264
  // CVMANL1_TOSHIBA_B.264
  // CVMANL2_TOSHIBA_B.264
  // CVMAQP2_Sony_G.jsv
  // CVMAQP3_Sony_D.jsv
  // CVMAPAQP3_Sony_E.jsv
  // CVMP_MOT_FRM_L31_B.26l
  // CVWP1_TOSHIBA_E.264
  // CVWP2_TOSHIBA_E.264
  // CVWP3_TOSHIBA_E.264
  // CVWP5_TOSHIBA_E.264
  // MR8_BT_B.h264
  // MR6_BT_B.h264

  // CABAC
  // CABA1_Sony_D.jsv


  unsigned int max_size=0, max_duration=0;
  unsigned int total_dur = 0;
  char* filename;

  if (argc <= 1)
  {
    //filename = "../../samples/iamalegend_1080p.h264";
    //filename = "../../samples/all/FM1_BT_B.h264";
    //filename = "../..samples/all/FVDO_Girl_720p.264";
    //filename = "../../samples/all/MR6_BT_B.h264";
    filename = "video_track1.h264";
  }
  else
  {
    filename = argv[1];
  }
  in = fopen("video_track1.h264", "rb");

  if (!in)
  {
    printf("File %s not found\n", filename);
    exit(-1);
  }

  out = fopen("out2.yuv", "wb");
  assert(out);

  ludh264_init();

  Picture* pic;
  uint32_t consumed, read;
  uint32_t remaining_bytes_in_file;
  uint32_t remaining_bytes_in_buffer=0;
  uint32_t to_read;

  fseek(in, 0, SEEK_END);
  remaining_bytes_in_file = ftell(in);
  fseek(in, 0, SEEK_SET);



  do
  {
    if (remaining_bytes_in_file)
    {
      to_read = min(WB_SIZE - remaining_bytes_in_buffer, remaining_bytes_in_file);
      read = fread(streamBuffer + remaining_bytes_in_buffer, 1, to_read, in);
      remaining_bytes_in_file -= read;
      remaining_bytes_in_buffer += read;
      LUD_DEBUG_ASSERT(to_read == read);

      if (!remaining_bytes_in_file)
      {
        // end of file, add a last startcode in order to correctly detect the last NALU
        streamBuffer[remaining_bytes_in_buffer] = 0;
        streamBuffer[remaining_bytes_in_buffer + 1] = 0;
        streamBuffer[remaining_bytes_in_buffer + 2] = 0;
        streamBuffer[remaining_bytes_in_buffer + 3] = 1;
        remaining_bytes_in_buffer += 4;
      }
    }

    consumed = remaining_bytes_in_buffer;

    gettimeofday(&prev, NULL);
    ludh264_get_frame(&pic, streamBuffer, &consumed);
    gettimeofday(&curr, NULL);
    LUD_DEBUG_ASSERT(remaining_bytes_in_buffer>consumed);

    remaining_bytes_in_buffer -= consumed;

    // move remaining data to the beginning of the buffer
    if (consumed)
      memmove(streamBuffer, streamBuffer + consumed, remaining_bytes_in_buffer);

    nb_of_nalu++;

    if (pic)
    {
      write_pic(pic, out);
      ludh264_free_frame(pic);
    }

    t = (int)((curr.tv_sec-prev.tv_sec)*1000000 + (curr.tv_usec-prev.tv_usec));
    max_size = max(max_size, consumed);
    max_duration = max(max_duration, t);
    total_dur+=t;

  } while (remaining_bytes_in_buffer + remaining_bytes_in_file > 6);

  do
  {
    ludh264_get_frame(&pic, 0, 0);
    if (pic)
    {
      write_pic(pic, out);
      ludh264_free_frame(pic);
    }
  } while (pic);









/*
  consumed = WB_SIZE;
  do
  {
    if (remaining_bytes_in_file)
    {
      to_read = im_min(consumed, remaining_bytes_in_file);
      read = fread( streamBuffer+WB_SIZE-consumed, 1, to_read, in );
      remaining_bytes_in_file -= read;
      remaining_bytes_in_buffer += read;
      LUD_DEBUG_ASSERT(to_read == read);

      if (!remaining_bytes_in_file)
      {
        // end of file, add a last startcode in order to correctly the last NALU
        streamBuffer[WB_SIZE-consumed+read] = 0;
        streamBuffer[WB_SIZE-consumed+read+1] = 0;
        streamBuffer[WB_SIZE-consumed+read+2] = 0;
        streamBuffer[WB_SIZE-consumed+read+3] = 1;
        remaining_bytes_in_buffer+=4;
      }
    }

    consumed = remaining_bytes_in_buffer;

    gettimeofday(&prev, NULL);
    r = decode_nalu(streamBuffer, &consumed);
    gettimeofday(&curr, NULL);

    LUD_DEBUG_ASSERT(SUCCESS == r);
    // move remaining data to the beginning of the buffer
    memmove(streamBuffer, streamBuffer+consumed, WB_SIZE-consumed);
    LUD_DEBUG_ASSERT(remaining_bytes_in_buffer>=consumed);
    remaining_bytes_in_buffer -= consumed;

    nb_of_nalu++;

    t = (int)((curr.tv_sec-prev.tv_sec)*1000000 + (curr.tv_usec-prev.tv_usec));
    max_size = im_max(max_size, consumed);
    max_duration = im_max(max_duration, t);
    total_dur+=t;
  } while (remaining_bytes_in_buffer+remaining_bytes_in_file>6);*/


  ludh264_destroy();
  fclose(in);
  fclose(out);
  printf("Parsed %d NALU in %d ms. max_dur=%d, max_len=%d\n", nb_of_nalu, total_dur/1000, max_duration, max_size);


 return 0;
}

