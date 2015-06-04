/*************************************************************************
         OggEncoder.cpp  -  encoder for Ogg/Vorbis data
                             -------------------
    begin                : Tue Sep 10 2002
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

#include "config.h"

#include <math.h>
#include <stdlib.h>

#include <QtCore/QList>
#include <QtCore/QSharedPointer>

#include <KI18n/KLocalizedString>
#include <kmimetype.h>
#include <kapplication.h>
#include <kglobal.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"

#include "OggCodecPlugin.h"
#include "OggEncoder.h"
#include "OggSubEncoder.h"
#include "OpusEncoder.h"
#include "VorbisEncoder.h"


/***************************************************************************/
Kwave::OggEncoder::OggEncoder()
    :Kwave::Encoder(), m_comments_map()
{
#ifdef HAVE_OGG_OPUS
    REGISTER_OGG_OPUS_MIME_TYPES;
    REGISTER_COMPRESSION_TYPE_OGG_OPUS;
#endif /* HAVE_OGG_OPUS */

#ifdef HAVE_OGG_VORBIS
    REGISTER_OGG_VORBIS_MIME_TYPES;
    REGISTER_COMPRESSION_TYPE_OGG_VORBIS;
#endif /* HAVE_OGG_VORBIS */
}

/***************************************************************************/
Kwave::OggEncoder::~OggEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::OggEncoder::instance()
{
    return new Kwave::OggEncoder();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::OggEncoder::supportedProperties()
{
    return m_comments_map.values();
}

/***************************************************************************/
bool Kwave::OggEncoder::encode(QWidget *widget, Kwave::MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    const Kwave::FileInfo info(meta_data);
    QSharedPointer<Kwave::OggSubEncoder> sub_encoder;

    // determine which codec (sub encoder) to use,
    // by examining the compression type
    const int compression = info.contains(Kwave::INF_COMPRESSION) ?
        info.get(Kwave::INF_COMPRESSION).toInt() : Kwave::Compression::NONE;

#ifdef HAVE_OGG_OPUS
    if (compression == Compression::OGG_OPUS) {
	qDebug("    OggEncoder: using Opus codec");
	sub_encoder =
	    QSharedPointer<Kwave::OpusEncoder>(new Kwave::OpusEncoder());
    }
#endif /* HAVE_OGG_OPUS */
#ifdef HAVE_OGG_VORBIS
    if (compression == Compression::OGG_VORBIS) {
	qDebug("    OggEncoder: using Vorbis codec");
	sub_encoder =
	    QSharedPointer<Kwave::VorbisEncoder>(new Kwave::VorbisEncoder());
    }
#endif /* HAVE_OGG_VORBIS */

    if (!sub_encoder) {
	qDebug("    OggEncoder: compression='%d'", compression);
	Kwave::MessageBox::error(widget, i18nc(
	    "error in Ogg encoder, no support for a compression type "
	    "(e.g. opus, vorbis etc)",
	    "Error: No Codec for '%1' available",
	    Kwave::Compression(compression).name()
	));
	return false;
    }

    if (!sub_encoder->open(widget, info, src))
	return false;

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving."));
	return false;
    }

    if (!sub_encoder->writeHeader(dst))
	return false;

    if (!sub_encoder->encode(src, dst))
	return false;

    // clean up and exit.
    sub_encoder->close();

    // the sub encoder will be deleted automatically (QSharedPointer)

    return true;
}

//***************************************************************************
//***************************************************************************
