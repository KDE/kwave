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

#ifndef WAV_FILE_FORMAT_H
#define WAV_FILE_FORMAT_H

#include <QList>

#include "libkwave/Compression.h"

namespace Kwave
{

    typedef enum /* Here is the list of known format tags */
    {
        /* Microsoft Corporation */
        WAVE_FORMAT_UNKNOWN             = 0x0000,

        /* Microsoft PCM format */
        WAVE_FORMAT_PCM                 = 0x0001,

        /* Microsoft ADPCM */
        WAVE_FORMAT_MS_ADPCM            = 0x0002,

        /* Micrososft 32 bit float format */
        WAVE_FORMAT_IEEE_FLOAT          = 0x0003,

        /* IBM Corporation */
        WAVE_FORMAT_IBM_CVSD            = 0x0005,

        /* Microsoft Corporation */
        WAVE_FORMAT_ALAW                = 0x0006,

        /* Microsoft Corporation */
        WAVE_FORMAT_MULAW               = 0x0007,

        /* OKI */
        WAVE_FORMAT_OKI_ADPCM           = 0x0010,

        /* Intel Corporation */
        WAVE_FORMAT_IMA_ADPCM           = 0x0011,

        /* Videologic */
        WAVE_FORMAT_MEDIASPACE_ADPCM    = 0x0012,

        /* Sierra Semiconductor Corp */
        WAVE_FORMAT_SIERRA_ADPCM        = 0x0013,

        /* Antex Electronics Corporation */
        WAVE_FORMAT_G723_ADPCM          = 0x0014,

        /* DSP Solutions, Inc. */
        WAVE_FORMAT_DIGISTD             = 0x0015,

        /* DSP Solutions, Inc. */
        WAVE_FORMAT_DIGIFIX             = 0x0016,

        /* Dialogic Corporation  */
        WAVE_FORMAT_DIALOGIC_OKI_ADPCM  = 0x0017,

        /* Media Vision, Inc. */
        WAVE_FORMAT_MEDIAVISION_ADPCM   = 0x0018,

        /* Yamaha Corporation of America */
        WAVE_FORMAT_YAMAHA_ADPCM        = 0x0020,

        /* Speech Compression */
        WAVE_FORMAT_SONARC              = 0x0021,

        /* DSP Group, Inc */
        WAVE_FORMAT_DSPGROUP_TRUESPEECH = 0x0022,

        /* Echo Speech Corporation */
        WAVE_FORMAT_ECHOSC1             = 0x0023,

        /* Audiofile, Inc. */
        WAVE_FORMAT_AUDIOFILE_AF18      = 0x0024,

        /* Audio Processing Technology */
        WAVE_FORMAT_APTX                = 0x0025,

        /* Audiofile, Inc. */
        WAVE_FORMAT_AUDIOFILE_AF10      = 0x0026,

        /* Dolby Laboratories */
        WAVE_FORMAT_DOLBY_AC2           = 0x0030,

        /* Microsoft Corporation */
        WAVE_FORMAT_GSM610              = 0x0031,

        /* Microsoft Corporation */
        WAVE_FORMAT_MSNAUDIO            = 0x0032,

        /* Antex Electronics Corporation */
        WAVE_FORMAT_ANTEX_ADPCME        = 0x0033,

        /* Control Resources Limited */
        WAVE_FORMAT_CONTROL_RES_VQLPC   = 0x0034,

        /* DSP Solutions, Inc. */
        WAVE_FORMAT_DIGIREAL            = 0x0035,

        /* DSP Solutions, Inc. */
        WAVE_FORMAT_DIGIADPCM           = 0x0036,

        /* Control Resources Limited */
        WAVE_FORMAT_CONTROL_RES_CR10    = 0x0037,

        /* Natural MicroSystems */
        WAVE_FORMAT_NMS_VBXADPCM        = 0x0038,

        /* Rockwell International */
        WAVE_FORMAT_ROCKWELL_ADPCM      = 0x003B,

        /* Rockwell International */
        WAVE_FORMAT_ROCKWELL_DIGITALK   = 0x003C,

        /* Antex Electronics Corporation */
        WAVE_FORMAT_G721_ADPCM          = 0x0040,

        /* Microsoft Corporation */
        WAVE_FORMAT_MPEG                = 0x0050,

        /* MPEG 3 Layer 1 */
        WAVE_FORMAT_MPEGLAYER3          = 0x0055,

        /* IBM mu-law format */
        WAVE_FORMAT_IBM_MULAW           = 0x0101,

        /* IBM a-law format */
        WAVE_FORMAT_IBM_ALAW            = 0x0102,

        /* IBM AVC Adaptive Differential PCM format */
        WAVE_FORMAT_IBM_ADPCM           = 0x0103,

        /* Creative Labs, Inc */
        WAVE_FORMAT_CREATIVE_ADPCM      = 0x0200,

        /* Fujitsu Corp. */
        WAVE_FORMAT_FM_TOWNS_SND        = 0x0300,

        /* Ing C. Olivetti & C., S.p.A. */
        WAVE_FORMAT_OLIGSM              = 0x1000,

        /* Ing C. Olivetti & C., S.p.A. */
        WAVE_FORMAT_OLIADPCM            = 0x1001,

        /* Ing C. Olivetti & C., S.p.A. */
        WAVE_FORMAT_OLICELP             = 0x1002,

        /* Ing C. Olivetti & C., S.p.A. */
        WAVE_FORMAT_OLISBC              = 0x1003,

        /* Ing C. Olivetti & C., S.p.A. */
        WAVE_FORMAT_OLIOPR              = 0x1004,

        WAVE_FORMAT_EXTENSIBLE          = 0xFFFE
    } wav_format_id;

    // header format for writing primitive canonical wav files
    typedef struct {
        qint8   riffid[4];
        quint32 filelength;
        qint8   wavid[4];
        qint8   fmtid[4];
        quint32 fmtlength;
        qint16  mode;
        qint16  channels;
        quint32 rate;
        quint32 AvgBytesPerSec;
        qint16  BlockAlign;
        qint16  bitspersample;
    } wav_header_t;

    //typedef struct {
    //    qint16 mode;               // Format tag: 1 = PCM
    //    qint16 channels;
    //    quint32 rate;
    //    quint32 AvgBytesPerSec;   // sample rate * block align
    //    qint16 BlockAlign;         // channels * bits/sample / 8
    //    qint16 bitspersample;
    //} wav_fmt_header_t;

    typedef struct {
        qint16  format;
        qint16  channels;
        quint32 samplerate;
        quint32 bytespersec;
        qint16  blockalign;
        qint16  bitwidth;
    } min_wav_header_t;

//     typedef struct {
//      qint16  format;
//      qint16  channels;
//      quint32 samplerate;
//      quint32 bytespersec;
//      qint16  blockalign;
//      qint16  bitwidth;
//      qint16  extrabytes;
//      qint16  dummy;
//     } wav_fmt_size20_header_t;

//     typedef struct {
//      qint16  format;
//      qint16  channels;
//      quint32 samplerate;
//      quint32 bytespersec;
//      qint16  blockalign;
//      qint16  bitwidth;
//      qint16  extrabytes;
//      qint16  samplesperblock;
//      qint16  numcoeffs;
//      struct {
//          qint16 coeff1;
//          qint16 coeff2;
//      }
//      coeffs [7];
//     } ms_adpcm_wav_header_t;

//     typedef struct {
//      qint16  format;
//      qint16  channels;
//      quint32 samplerate;
//      quint32 bytespersec;
//      qint16  blockalign;
//      qint16  bitwidth;
//      qint16  extrabytes;
//      qint16  samplesperblock;
//     } ima_adpcm_wav_header_t;

//     typedef struct {
//      quint32 esf_field1;
//      qint16  esf_field2;
//      qint16  esf_field3;
//      quint8  esf_field4 [8];
//     } ext_subformat_t;

//     typedef struct {
//      qint16  format;
//      qint16  channels;
//      quint32 samplerate;
//      quint32 bytespersec;
//      qint16  blockalign;
//      qint16  bitwidth;
//      qint16  extrabytes;
//      qint16  validbits;
//      quint32 channelmask;
//      Kwave::ext_subformat_t esf;
//     } extensible_wav_header_t;

    typedef union {
//      qint16 format;
        Kwave::min_wav_header_t        min;
//      Kwave::ima_adpcm_wav_header_t  ima;
//      Kwave::ms_adpcm_wav_header_t   msadpcm;
//      Kwave::extensible_wav_header_t ext;
//      Kwave::wav_fmt_size20_header_t size20;
        quint8 padding[512];
    } wav_fmt_header_t;

//     typedef struct {
//      quint32 samples;
//     } fact_chunk_t;

    /**
     * Returns a list with all compression types supported by libaudiofile
     */
    QList<Kwave::Compression::Type> audiofileCompressionTypes();

}

/* defined in RFC 2361 and other places */
#define REGISTER_MIME_TYPES                       \
    addMimeType(                                  \
        "audio/x-wav, audio/vnd.wave, audio/wav", \
        i18n("WAV audio"),                        \
        "*.wav"                                   \
    );

#define REGISTER_COMPRESSION_TYPES \
    foreach (Kwave::Compression::Type c, Kwave::audiofileCompressionTypes()) \
        addCompression(c);

#endif /* WAV_FILE_FORMAT_H */

//***************************************************************************
//***************************************************************************
