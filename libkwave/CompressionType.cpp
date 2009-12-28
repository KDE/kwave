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

#include "CompressionType.h"

//***************************************************************************
CompressionType::CompressionType()
    :TypesMap<int,int>()
{
}

//***************************************************************************
CompressionType::~CompressionType()
{
}

//***************************************************************************
void CompressionType::fill()
{
    int i=0;

    /* supported compression schemes */
    append(i++, AF_COMPRESSION_NONE,
           i18n("No Compression"), 0);
    append(i++, AF_COMPRESSION_G722,
           i18n("G722"), 0);
    append(i++, AF_COMPRESSION_G711_ULAW,
           i18n("G711 ULAW"), 0);
    append(i++, AF_COMPRESSION_G711_ALAW,
           i18n("G711 ALAW"), 0);

    /* Apple proprietary AIFF-C compression schemes (not supported) */
    append(i++, AF_COMPRESSION_APPLE_ACE2,
           i18n("Apple ACE2"), 0);
    append(i++, AF_COMPRESSION_APPLE_ACE8,
           i18n("Apple ACE8"), 0);
    append(i++, AF_COMPRESSION_APPLE_MAC3,
           i18n("Apple MAC3"), 0);
    append(i++, AF_COMPRESSION_APPLE_MAC6,
           i18n("Apple MAC6"), 0);

    append(i++, AF_COMPRESSION_G726,
           i18n("G726"), 0);
    append(i++, AF_COMPRESSION_G728,
           i18n("G728"), 0);
    append(i++, AF_COMPRESSION_DVI_AUDIO,
           i18n("DVI Audio / IMA"), 0);
    append(i++, AF_COMPRESSION_GSM,
           i18n("GSM"), 0);
    append(i++, AF_COMPRESSION_FS1016,
           i18n("FS1016"), 0);
    append(i++, AF_COMPRESSION_DV,
           i18n("DV"), 0);
    append(i++, AF_COMPRESSION_MS_ADPCM,
           i18n("MS ADPCM"), 0);

    append(i++, MPEG_LAYER_I,
           i18n("MPEG Layer I"), 0);
    append(i++, MPEG_LAYER_II,
           i18n("MPEG Layer II"), 0);
    append(i++, MPEG_LAYER_III,
           i18n("MPEG Layer III"), 0);

    append(i++, OGG_VORBIS,
           i18n("Ogg Vorbis"), 0);

    append(i++, FLAC,
           i18n("FLAC"), 0);

}

//***************************************************************************
//***************************************************************************
