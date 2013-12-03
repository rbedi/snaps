#include "ejpgl.h"

extern int zzq_encode_init_start(int compression);
extern int zzq_encode_stop_done();
extern void zzq_encode(signed short pixelmatrix[MATRIX_SIZE][MATRIX_SIZE], int color);

