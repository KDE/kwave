
#include <sys/types.h>

struct wavheader
	    // header format for reading wav files
{
    int8_t riffid[4];
    u_int32_t filelength;
    int8_t wavid[4];
    int8_t fmtid[4];
    u_int32_t fmtlength;
    int16_t mode;
    int16_t channels;
    u_int32_t rate;
    u_int32_t AvgBytesPerSec;
    int16_t BlockAlign;
    int16_t bitspersample;
};

#define sample_t int32_t
