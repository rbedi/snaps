extern int vlc_init_start();
extern int vlc_stop_done();
extern void ConvertDCMagnitudeC(unsigned char magnitude,unsigned short int *out, unsigned short int *lenght);
extern void ConvertACMagnitudeC(unsigned char magnitude,unsigned short int *out, unsigned short int *lenght);
extern void ConvertDCMagnitudeY(unsigned char magnitude,unsigned short int *out, unsigned short int *lenght);
extern void ConvertACMagnitudeY(unsigned char magnitude,unsigned short int *out, unsigned short int *lenght);
extern char Extend (char additional, unsigned char magnitude);
extern void ReverseExtend (char value, unsigned char *magnitude, unsigned char *bits);
extern void WriteRawBits16(unsigned char amount_bits, unsigned int bits);
extern void HuffmanEncodeFinishSend();
extern void HuffmanEncodeUsingDCTable(unsigned char magnitude);
extern void HuffmanEncodeUsingACTable(unsigned char mag);
extern char EncodeDataUnit(signed char dataunit[64], unsigned int color);

