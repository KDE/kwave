
#ifndef _FILE_FORMAT_H_
#define _FILE_FORMAT_H_

#include <sys/types.h>

// header format for reading wav files
typedef struct
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
}  wav_header_t;

typedef struct
{
    int16_t mode;              // Format tag: 1 = PCM
    int16_t channels;
    u_int32_t rate;
    u_int32_t AvgBytesPerSec;  // sample rate * block align
    int16_t BlockAlign;        // channels * bits/sample / 8
    int16_t bitspersample;
} wav_fmt_header_t;

#define WAV   0
#define ASCII 1

#endif /* _FILE_FORMAT_H_ */
