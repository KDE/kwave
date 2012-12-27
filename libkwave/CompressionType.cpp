/***************************************************************************
    CompressionType.cpp  -  Map for all known compression types
                             -------------------
    begin                : Mon Jul 29 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "libkwave/CompressionType.h"
#include "libkwave/String.h"

//***************************************************************************
Kwave::CompressionType::CompressionType()
    :Kwave::TypesMap<int,int>()
{
    fill();
}

//***************************************************************************
Kwave::CompressionType::~CompressionType()
{
}

//***************************************************************************
void Kwave::CompressionType::fill()
{
    int i = 0;

    /* supported compression schemes */
    append(i++, AF_COMPRESSION_NONE,       _(0), i18n("No Compression"));
    append(i++, AF_COMPRESSION_G722,       _(0), i18n("G722"));
    append(i++, AF_COMPRESSION_G711_ULAW,  _(0), i18n("G711 ULAW"));
    append(i++, AF_COMPRESSION_G711_ALAW,  _(0), i18n("G711 ALAW"));

    /* Apple proprietary AIFF-C compression schemes (not supported) */
    append(i++, AF_COMPRESSION_APPLE_ACE2, _(0), i18n("Apple ACE2"));
    append(i++, AF_COMPRESSION_APPLE_ACE8, _(0), i18n("Apple ACE8"));
    append(i++, AF_COMPRESSION_APPLE_MAC3, _(0), i18n("Apple MAC3"));
    append(i++, AF_COMPRESSION_APPLE_MAC6, _(0), i18n("Apple MAC6"));

    append(i++, AF_COMPRESSION_G726,       _(0), i18n("G726"));
    append(i++, AF_COMPRESSION_G728,       _(0), i18n("G728"));
    append(i++, AF_COMPRESSION_DVI_AUDIO,  _(0), i18n("DVI Audio / IMA"));
    append(i++, AF_COMPRESSION_GSM,        _(0), i18n("GSM"));
    append(i++, AF_COMPRESSION_FS1016,     _(0), i18n("FS1016"));
    append(i++, AF_COMPRESSION_DV,         _(0), i18n("DV"));
    append(i++, AF_COMPRESSION_MS_ADPCM,   _(0), i18n("MS ADPCM"));

    append(i++, MPEG_LAYER_I,              _(0), i18n("MPEG Layer I"));
    append(i++, MPEG_LAYER_II,             _(0), i18n("MPEG Layer II"));
    append(i++, MPEG_LAYER_III,            _(0), i18n("MPEG Layer III"));

    append(i++, OGG_OPUS,                  _(0), i18n("Ogg Opus"));
    append(i++, OGG_VORBIS,                _(0), i18n("Ogg Vorbis"));

    append(i++, FLAC,                      _(0), i18n("FLAC"));

}

//***************************************************************************
//***************************************************************************
