#include "filemanager.h"

#ifdef STM32F4XX
  #include "ff.h"

  #define MAX_FILE_POINTERS 16
#endif


#ifdef STM32F4XX
  static struct {
    FIL filepointer;
    uint8_t inuse;
  } _filepointers[MAX_FILE_POINTERS];


  FIL* _fm_open(const uint8_t* filename, uint8_t mode)
  {
    int i;
    for(i = 0; i < MAX_FILE_POINTERS; i++)
    {
      if(!_filepointers[i].inuse)
      {
        FRESULT res = f_open(&_filepointers[i].filepointer, filename, mode);
        if(res == FR_OK)
        {
          _filepointers[i].inuse = 1;
          return &_filepointers[i].filepointer;
        }
        else
          return NULL;
      }
    }

    return NULL;
  }

  void _fm_close(FIL* file)
  {
    f_close(file);

    int i;
    for(i = 0; i < MAX_FILE_POINTERS; i++)
    {
      if(&_filepointers[i].filepointer == file)
      {
        _filepointers[i].inuse = 0;
        break;
      }
    }
  }
#endif
