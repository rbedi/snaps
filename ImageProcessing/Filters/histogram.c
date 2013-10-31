#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../image.h"

int histogram(FRAMECHAR *CurrentFrame,unsigned char *verticalleft, unsigned char *verticalright, unsigned char *horizontalbottom,unsigned char *horizontaltop)
{

    int i,j;
    int red,green,blue;
    int red2,green2,blue2;
    int redl,greenl,bluel;
    int redr,greenr,bluer;
    int redt,greent,bluet;
    int redb,greenb,blueb;
    int culmred, culmblue, culmgreen;
    float avered, avegreen, aveblue;
    int width, height, pixels;
    int test;
    avered = 0;
    aveblue = 0;
    avegreen = 0;
    culmred = 0;
    culmblue = 0;
    culmgreen = 0;
    red = 0;
    green = 0;
    blue = 0;
    red2 = 0;
    blue2 = 0;
    green2 = 0;
    test = 0;
    width = CurrentFrame->width;
    height = CurrentFrame->height;
    pixels = width*height;

    for (i = 0; i < height; i ++)
    {

        for (j = 0; j < width/8; j ++) {
            red = red + CurrentFrame->framebits[i][(j*3)+2];
            green = green + CurrentFrame->framebits[i][(j*3)+1];
            blue = blue + CurrentFrame->framebits[i][(j*3)];
        }


        redl = red;
        bluel = blue;
        greenl = green;
        culmred = culmred + red;
        culmblue = culmblue + blue;
        culmgreen = culmgreen + green;
        red = 0;
        green = 0;
        blue = 0;

        for (j = (7*width/8)-1; j < width; j ++) {
            red = red + CurrentFrame->framebits[i][(j*3)+2];
            green = green + CurrentFrame->framebits[i][(j*3)+1];
            blue = blue + CurrentFrame->framebits[i][(j*3)];

        }

        redr = red;
        bluer = blue;
        greenr = green;
        culmred = culmred + red;
        culmblue = culmblue + blue;
        culmgreen = culmgreen + green;
        red = 0;
        green = 0;
        blue = 0;

        verticalleft[i]=8*(redl+bluel+greenl)/(3*Widths);
        verticalright[i]=8*(redr+bluer+greenr)/(3*Widths);


    }

    for (i = 0; i < width; i ++) {
        for (j = 0; j < height/8; j ++) {
            red = red + CurrentFrame->framebits[j][(i*3)+2];
            green = green + CurrentFrame->framebits[j][(i*3)+1];
            blue = blue + CurrentFrame->framebits[j][(i*3)];
            if(i>width/8 && i<7*width/8)
            {
                red2 = red2 + CurrentFrame->framebits[j][(i*3)+2];
                blue2 = blue2 + CurrentFrame->framebits[j][(i*3)];
                green2 = green2 + CurrentFrame->framebits[j][(i*3)+1];
            }
        }

        redt = red;
        bluet = blue;
        greent = green;
        culmred = culmred + red2;
        culmblue = culmblue + blue2;
        culmgreen = culmgreen + green2;
        red = 0;
        green = 0;
        blue = 0;
        red2 = 0;
        green2 = 0;
        blue2 = 0;

       for (j = 7*height/8; j < height; j ++) {
            red = red + CurrentFrame->framebits[j][(i*3)+2];
            green = green + CurrentFrame->framebits[j][(i*3)+1];
            blue = blue + CurrentFrame->framebits[j][(i*3)];


            if(i>width/8 && i<7*width/8)
            {
                red2 = red2 + CurrentFrame->framebits[j][(i*3)+2];
                blue2 = blue2 + CurrentFrame->framebits[j][(i*3)];
                green2 = green2 + CurrentFrame->framebits[j][(i*3)+1];

            }
        }
        redb = red;
        blueb = blue;
        greenb = green;
        culmred = culmred + red2;
        culmblue = culmblue + blue2;
        culmgreen = culmgreen + green2;
        red = 0;
        green = 0;
        blue = 0;
        red2 = 0;
        blue2 = 0;
        green2 = 0;
        //printf("%d %d %d \n", redt, bluet,greent);
        horizontaltop[i]=8*(redt+bluet+greent)/(3*Heights);
        horizontalbottom[i]=8*(redb+blueb+greenb)/(3*Heights);
    }


    for (i = height/8; i < 7*height/8; i ++) {

        for (j = width/8; j < 7*width/8; j ++) {
            red = red + CurrentFrame->framebits[i][(j*3)+2];
            green = green + CurrentFrame->framebits[i][(j*3)+1];
            blue = blue + CurrentFrame->framebits[i][(j*3)];
        }
        culmred = culmred + red;
        culmblue = culmblue + blue;
        culmgreen = culmgreen + green;
        red = 0;
        green = 0;
        blue = 0;
    }


  avered = 1.0*culmred/pixels;
  avegreen = 1.0*culmgreen/pixels;
  aveblue = 1.0*culmblue/pixels;
  printf("Red: %.2f   Green: %.2f   Blue: %.2f \n", avered,avegreen,aveblue);
  if(avegreen>GreenFilter && avered>RedFilter && aveblue>BlueFilter)
  {
    printf("Histogram: PASS! \n");
    return(10);
  }
  else
  {
    printf("Histogram: FAIL! \n");
    return(5);
  }
}
