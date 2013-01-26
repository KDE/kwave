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

#include <ogg/ogg.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QTime>

#include <klocale.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kaboutdata.h>

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
    :Kwave::Encoder(), m_sub_encoder(0), m_comments_map()
{
    REGISTER_COMMON_MIME_TYPES;

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
    delete m_sub_encoder;
    m_sub_encoder = 0;
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

    ogg_stream_state os; // take physical pages, weld into a logical stream of packets
    ogg_page         og; // one Ogg bitstream page.  Vorbis packets are inside
    ogg_packet       op; // one raw packet of data for decode

    // get rid of the previous sub decoder
    if (m_sub_encoder) {
	delete m_sub_encoder;
	m_sub_encoder = 0;
    }

    // determine which codec (sub encoder) to use,
    // by examining the compression type
    const int compression = info.contains(Kwave::INF_COMPRESSION) ?
        info.get(Kwave::INF_COMPRESSION).toInt() : Kwave::CompressionType::NONE;

#ifdef HAVE_OGG_OPUS
    if (compression == CompressionType::OGG_OPUS) {
	qDebug("    OggEncoder: using Opus codec");
	m_sub_encoder =
	    new Kwave::OpusEncoder(os, og, op);
    }
#endif /* HAVE_OGG_OPUS */
#ifdef HAVE_OGG_VORBIS
    if (compression == CompressionType::OGG_VORBIS) {
	qDebug("    OggEncoder: using Vorbis codec");
	m_sub_encoder = new Kwave::VorbisEncoder(os, og, op);
    }
#endif /* HAVE_OGG_VORBIS */

    if (!m_sub_encoder) {
	qDebug("    OggEncoder: compression='%d'", compression);
	Kwave::MessageBox::error(widget, i18nc(
	    "error in Ogg encoder, no support for a compression type "
	    "(e.g. opus, vorbis etc)",
	    "Error: No Codec for '%1' available",
	    Kwave::Compression(compression).name()
	));
	return false;
    }

    if (!m_sub_encoder->open(widget, info, src))
	return false;

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving."));
	return false;
    }

    // set up our packet->stream encoder
    // pick a random serial number; that way we can more likely build
    // chained streams just by concatenation
    srand(QTime::currentTime().msec());
    ogg_stream_init(&os, rand());

    if (!m_sub_encoder->writeHeader(dst))
	return false;

    if (!m_sub_encoder->encode(src, dst))
	return false;

    // clean up and exit.
    m_sub_encoder->close();

    // delete the sub encoder, it's not needed any more
    delete m_sub_encoder;
    m_sub_encoder = 0;

    return true;
}

//***************************************************************************
//***************************************************************************
