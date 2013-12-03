#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifdef STM32F4XX
  #include "ff.h"

  FIL* _fm_open(const uint8_t* filename, uint8_t mode);
  void _fm_close(FIL* file);


  #define FM_FILE FIL

  #define fm_open _fm_open
  #define fm_close _fm_close
  #define fm_write(buffer,size,written,file) f_write(file,buffer,size,&written)
  #define fm_read(buffer,size,read,file) f_read(file,buffer,size,&read)
  #define fm_flush(file) f_sync(file)
  #define fm_seek(file,distance,type) f_lseek(file,distance)

#else

  #define FM_FILE FILE

  #define fm_open fopen
  #define fm_write(buffer,size,written,file) fwrite(buffer,size,written,file)
  #define fm_seek fseeko64
  #define fm_tell ftell
  #define fm_close fclose
  #define fm_read fread
  #define fm_flush fflush

#endif
