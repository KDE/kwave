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

typedef enum /* Here is the list of known format tags */
{
    WAVE_FORMAT_UNKNOWN             = 0x0000, /* Microsoft Corporation */
    WAVE_FORMAT_PCM                 = 0x0001, /* Microsoft PCM format */

    WAVE_FORMAT_MS_ADPCM            = 0x0002, /* Microsoft ADPCM */
    WAVE_FORMAT_IEEE_FLOAT          = 0x0003, /* Micrososft 32 bit float format */

    WAVE_FORMAT_IBM_CVSD            = 0x0005, /* IBM Corporation */
    WAVE_FORMAT_ALAW                = 0x0006, /* Microsoft Corporation */
    WAVE_FORMAT_MULAW               = 0x0007, /* Microsoft Corporation */
    WAVE_FORMAT_OKI_ADPCM           = 0x0010, /* OKI */
    WAVE_FORMAT_IMA_ADPCM           = 0x0011, /* Intel Corporation */
    WAVE_FORMAT_MEDIASPACE_ADPCM    = 0x0012, /* Videologic */
    WAVE_FORMAT_SIERRA_ADPCM        = 0x0013, /* Sierra Semiconductor Corp */
    WAVE_FORMAT_G723_ADPCM          = 0x0014, /* Antex Electronics Corporation */
    WAVE_FORMAT_DIGISTD             = 0x0015, /* DSP Solutions, Inc. */
    WAVE_FORMAT_DIGIFIX             = 0x0016, /* DSP Solutions, Inc. */
    WAVE_FORMAT_DIALOGIC_OKI_ADPCM  = 0x0017, /* Dialogic Corporation  */
    WAVE_FORMAT_MEDIAVISION_ADPCM   = 0x0018, /* Media Vision, Inc. */

    WAVE_FORMAT_YAMAHA_ADPCM        = 0x0020, /* Yamaha Corporation of America */
    WAVE_FORMAT_SONARC              = 0x0021, /* Speech Compression */
    WAVE_FORMAT_DSPGROUP_TRUESPEECH = 0x0022, /* DSP Group, Inc */
    WAVE_FORMAT_ECHOSC1             = 0x0023, /* Echo Speech Corporation */
    WAVE_FORMAT_AUDIOFILE_AF18      = 0x0024, /* Audiofile, Inc. */
    WAVE_FORMAT_APTX                = 0x0025, /* Audio Processing Technology */
    WAVE_FORMAT_AUDIOFILE_AF10      = 0x0026, /* Audiofile, Inc. */

    WAVE_FORMAT_DOLBY_AC2           = 0x0030, /* Dolby Laboratories */
    WAVE_FORMAT_GSM610              = 0x0031, /* Microsoft Corporation */
    WAVE_FORMAT_MSNAUDIO            = 0x0032, /* Microsoft Corporation */
    WAVE_FORMAT_ANTEX_ADPCME        = 0x0033, /* Antex Electronics Corporation */
    WAVE_FORMAT_CONTROL_RES_VQLPC   = 0x0034, /* Control Resources Limited */
    WAVE_FORMAT_DIGIREAL            = 0x0035, /* DSP Solutions, Inc. */
    WAVE_FORMAT_DIGIADPCM           = 0x0036, /* DSP Solutions, Inc. */
    WAVE_FORMAT_CONTROL_RES_CR10    = 0x0037, /* Control Resources Limited */
    WAVE_FORMAT_NMS_VBXADPCM        = 0x0038, /* Natural MicroSystems */
    WAVE_FORMAT_ROCKWELL_ADPCM      = 0x003B, /* Rockwell International */
    WAVE_FORMAT_ROCKWELL_DIGITALK   = 0x003C, /* Rockwell International */

    WAVE_FORMAT_G721_ADPCM          = 0x0040, /* Antex Electronics Corporation */
    WAVE_FORMAT_MPEG                = 0x0050, /* Microsoft Corporation */

    WAVE_FORMAT_MPEGLAYER3          = 0x0055, /* MPEG 3 Layer 1 */

    WAVE_FORMAT_IBM_MULAW           = 0x0101, /* IBM mu-law format */
    WAVE_FORMAT_IBM_ALAW            = 0x0102, /* IBM a-law format */
    WAVE_FORMAT_IBM_ADPCM           = 0x0103, /* IBM AVC Adaptive Differential PCM format */

    WAVE_FORMAT_CREATIVE_ADPCM      = 0x0200, /* Creative Labs, Inc */

    WAVE_FORMAT_FM_TOWNS_SND        = 0x0300, /* Fujitsu Corp. */
    WAVE_FORMAT_OLIGSM              = 0x1000, /* Ing C. Olivetti & C., S.p.A. */
    WAVE_FORMAT_OLIADPCM            = 0x1001, /* Ing C. Olivetti & C., S.p.A. */
    WAVE_FORMAT_OLICELP             = 0x1002, /* Ing C. Olivetti & C., S.p.A. */
    WAVE_FORMAT_OLISBC              = 0x1003, /* Ing C. Olivetti & C., S.p.A. */
    WAVE_FORMAT_OLIOPR              = 0x1004, /* Ing C. Olivetti & C., S.p.A. */

    WAVE_FORMAT_EXTENSIBLE          = 0xFFFE
} wav_format_id;

// header format for writing primitive canonical wav files
typedef struct {
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
} wav_header_t;

//typedef struct {
//    int16_t mode;               // Format tag: 1 = PCM
//    int16_t channels;
//    u_int32_t rate;
//    u_int32_t AvgBytesPerSec;   // sample rate * block align
//    int16_t BlockAlign;         // channels * bits/sample / 8
//    int16_t bitspersample;
//} wav_fmt_header_t;

typedef struct {
    int16_t format;
    int16_t channels;
    u_int32_t samplerate;
    u_int32_t bytespersec;
    int16_t blockalign;
    int16_t bitwidth;
} min_wav_header_t;

typedef struct {
    int16_t format;
    int16_t channels;
    u_int32_t samplerate;
    u_int32_t bytespersec;
    int16_t blockalign;
    int16_t bitwidth;
    int16_t extrabytes;
    int16_t dummy;
} wav_fmt_size20_header_t;

typedef struct {
    int16_t format;
    int16_t channels;
    u_int32_t samplerate;
    u_int32_t bytespersec;
    int16_t blockalign;
    int16_t bitwidth;
    int16_t extrabytes;
    int16_t samplesperblock;
    int16_t numcoeffs;
    struct {
	int16_t coeff1;
	int16_t coeff2;
    }
    coeffs [7];
} ms_adpcm_wav_header_t;

typedef struct {
    int16_t format;
    int16_t channels;
    u_int32_t samplerate;
    u_int32_t bytespersec;
    int16_t blockalign;
    int16_t bitwidth;
    int16_t extrabytes;
    int16_t samplesperblock;
} ima_adpcm_wav_header_t;

typedef struct {
    u_int32_t esf_field1;
    int16_t esf_field2;
    int16_t esf_field3;
    u_int8_t esf_field4 [8];
} ext_subformat_t;

typedef struct {
    int16_t format;
    int16_t channels;
    u_int32_t samplerate;
    u_int32_t bytespersec;
    int16_t blockalign;
    int16_t bitwidth;
    int16_t extrabytes;
    int16_t validbits;
    u_int32_t channelmask;
    ext_subformat_t esf;
} extensible_wav_header_t;

typedef union {
    int16_t format;
    min_wav_header_t        min;
    ima_adpcm_wav_header_t  ima;
    ms_adpcm_wav_header_t   msadpcm;
    extensible_wav_header_t ext;
    wav_fmt_size20_header_t size20;
    u_int8_t padding[512];
} wav_fmt_header_t;

typedef struct {
    u_int32_t samples;
} fact_chunk_t;


#define LOAD_MIME_TYPES { \
    /* included in KDE: */ \
addMimeType("audio/x-wav",    i18n("wav audio"), "*.wav"); \
    /* defined in RFC 2361 */ \
addMimeType("audio/vnd.wave", i18n("wav audio"), "*.wav"); \
    /* defined nowhere, but someone has used that */ \
addMimeType("audio/wav", i18n("wav audio"), "*.wav"); \
}

#endif /* _WAV_FILE_FORMAT_H_ */
//***************************************************************************
//***************************************************************************
