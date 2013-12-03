#include "global.h"
#include "util.h"

static char _uniqueIDChar[25];
const char* unique_id_char;

static uint8_t nibbleToAsciiChar(uint8_t nibble);

void populateUniqueID()
{
  unique_id_char = _uniqueIDChar;
  
  char* uid = (char*)UNIQUE_ID;
  int i;
  for(i = 0; i < 12; i++)
  {
    _uniqueIDChar[2*i] = nibbleToAsciiChar(uid[i] >> 4);
    _uniqueIDChar[2*i + 1] = nibbleToAsciiChar(uid[i]);
  }
}

static uint8_t nibbleToAsciiChar(uint8_t nibble)
{
  nibble &= 0x0F;
  if(nibble >= 0x0A)
    return (nibble - 0x0A) + 'A';
  else
    return nibble + '0';
}
