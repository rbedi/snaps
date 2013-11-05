#define heightbit1 0x04
#define heightbit2 0x40
#define widthbit1 0x07
#define widthbit2 0x80
#define Heights  1088
#define Widths 1920
#define macroblockcolumns 120
#define macroblockrows 68
#define GreenFilter 0.1
#define RedFilter 0.25
#define BlueFilter 0.5
#define whitefilterblue 170
#define whitefilterred 170
#define whitefiltergreen 170
#define whitefilterthreshold 0.04
#define circledetectmatrixsize 16
#define file_name "HHD00017.MOV"

//#define file_name "test.txt"
// test comment

#include "filemanager.h"

#ifdef STM32F4XX
    #define writemode (FA_WRITE | FA_OPEN_ALWAYS)
    #define readmode FA_READ
#else
    #define image_main main
    #define writemode "wb"
    #define readmode "rb"
#endif


#ifndef MAIN_H_
#define MAIN_H_
typedef struct {
    unsigned int position;                       /* Position of Frame in Video      */
    unsigned int width,height;                   /* Width and height of Frame */
    unsigned char framebits[544][960];  /* RGB Matrix    */
} FRAMECHAR;

typedef struct
{
    unsigned short framenumber;
    unsigned char rating;
    unsigned char hq_avail;
} FRAMECONTAINER;


#endif /* MAIN_H_ */

extern int image_main(void);
extern FRAMECHAR CurrentFrame;
extern unsigned int averagepixelintensity;

