/***************************************************************************
        WavFileFormat.h  -  format of a wav file
			     -------------------
    begin                : Mar 05 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _WAV_FILE_FORMAT_H_
#define _WAV_FILE_FORMAT_H_

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

/** @deprecated */
#define WAV   0

/** @deprecated */
#define ASCII 1

#endif /* _WAV_FILE_FORMAT_H_ */
//***************************************************************************
//***************************************************************************
