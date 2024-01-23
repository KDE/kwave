/***************************************************************************
       WavFormatMap.cpp  -  list of known wav file formats
                             -------------------
    begin                : Apr 28 2002
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

#include "config.h"

#include <QString>

#include "libkwave/String.h"

#include "WavFormatMap.h"

//***************************************************************************
Kwave::WavFormatMap::WavFormatMap()
{
#define FMT(x,y) insert(WAVE_FORMAT_##x,_(y));
    FMT(UNKNOWN             ," Unknown; Microsoft Corporation ")
    FMT(PCM                 ," Microsoft PCM format ")
    FMT(MS_ADPCM            ," Microsoft ADPCM ")
    FMT(IEEE_FLOAT          ," Microsoft 32 bit float format ")
    FMT(IBM_CVSD            ," IBM Corporation ")
    FMT(ALAW                ," A-Law; Microsoft Corporation ")
    FMT(MULAW               ," U-Law; Microsoft Corporation ")
    FMT(OKI_ADPCM           ," ADPCM; OKI ")
    FMT(IMA_ADPCM           ," ADPCM; Intel Corporation ")
    FMT(MEDIASPACE_ADPCM    ," ADPCM; Videologic ")
    FMT(SIERRA_ADPCM        ," ADPCM; Sierra Semiconductor Corp ")
    FMT(G723_ADPCM          ," G723 ADPCM; Antex Electronics Corporation ")
    FMT(DIGISTD             ," DIGISTD; DSP Solutions, Inc. ")
    FMT(DIGIFIX             ," DIGIFIX; DSP Solutions, Inc. ")
    FMT(DIALOGIC_OKI_ADPCM  ," OKI ADPCM; Dialogic Corporation  ")
    FMT(MEDIAVISION_ADPCM   ," ADPCM; Media Vision, Inc. ")
    FMT(YAMAHA_ADPCM        ," ADPCM; Yamaha Corporation of America ")
    FMT(SONARC              ," SONARC; Speech Compression ")
    FMT(DSPGROUP_TRUESPEECH ," Truespeech; DSP Group, Inc ")
    FMT(ECHOSC1             ," SC1; Echo Speech Corporation ")
    FMT(AUDIOFILE_AF18      ," AF18; Audiofile, Inc. ")
    FMT(APTX                ," APTX; Audio Processing Technology ")
    FMT(AUDIOFILE_AF10      ," AF10; Audiofile, Inc. ")
    FMT(DOLBY_AC2           ," Dolby AC2; Dolby Laboratories ")
    FMT(GSM610              ," GSM610; Microsoft Corporation ")
    FMT(MSNAUDIO            ," MSN audio; Microsoft Corporation ")
    FMT(ANTEX_ADPCME        ," ADPCME; Antex Electronics Corporation ")
    FMT(CONTROL_RES_VQLPC   ," RES VQLPC; Control Resources Limited ")
    FMT(DIGIREAL            ," DigiReal; DSP Solutions, Inc. ")
    FMT(DIGIADPCM           ," DigiADPCM; DSP Solutions, Inc. ")
    FMT(CONTROL_RES_CR10    ," Res CR10; Control Resources Limited ")
    FMT(NMS_VBXADPCM        ," VBCADPCM; Natural MicroSystems ")
    FMT(ROCKWELL_ADPCM      ," ADPCM; Rockwell International ")
    FMT(ROCKWELL_DIGITALK   ," DigiTalk; Rockwell International ")
    FMT(G721_ADPCM          ," G721 ADPCM; Antex Electronics Corporation ")
    FMT(MPEG                ," MPEG; Microsoft Corporation ")
    FMT(MPEGLAYER3          ," MPEG 3 Layer 1 ")
    FMT(IBM_MULAW           ," IBM mu-law format ")
    FMT(IBM_ALAW            ," IBM a-law format ")
    FMT(IBM_ADPCM           ," IBM AVC Adaptive Differential PCM format ")
    FMT(CREATIVE_ADPCM      ," ADPCM; Creative Labs, Inc ")
    FMT(FM_TOWNS_SND        ," FM TOWNS SND; Fujitsu Corp. ")
    FMT(OLIGSM              ," GSM; Ing C. Olivetti & C., S.p.A. ")
    FMT(OLIADPCM            ," ADPCM; Ing C. Olivetti & C., S.p.A. ")
    FMT(OLICELP             ," CELP; Ing C. Olivetti & C., S.p.A. ")
    FMT(OLISBC              ," SBC; Ing C. Olivetti & C., S.p.A. ")
    FMT(OLIOPR              ," OPR; Ing C. Olivetti & C., S.p.A. ")
    FMT(EXTENSIBLE          ," Extensible ")
#undef FMT
}

//***************************************************************************
const QString &Kwave::WavFormatMap::findName(unsigned int id)
{
    if (!contains(static_cast<Kwave::wav_format_id>(id)))
        return (*this)[WAVE_FORMAT_UNKNOWN];
    return (*this)[static_cast<Kwave::wav_format_id>(id)];
}

//***************************************************************************
//***************************************************************************
