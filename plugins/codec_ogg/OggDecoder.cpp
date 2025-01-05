/*************************************************************************
        OggDecoder.cpp  -  decoder for Ogg/Vorbis data
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
#include <new>

#include <QDate>
#include <QIODevice>
#include <QLatin1Char>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"

#include "OggCodecPlugin.h"
#include "OggDecoder.h"
#include "OggSubDecoder.h"
#include "OpusDecoder.h"
#include "VorbisDecoder.h"

//***************************************************************************
Kwave::OggDecoder::OggDecoder()
    :Kwave::Decoder(), m_sub_decoder(nullptr), m_source(nullptr)
{
#ifdef HAVE_OGG_OPUS
    REGISTER_OGG_OPUS_MIME_TYPES
    REGISTER_COMPRESSION_TYPE_OGG_OPUS
#endif /* HAVE_OGG_OPUS */

#ifdef HAVE_OGG_VORBIS
    REGISTER_OGG_VORBIS_MIME_TYPES
    REGISTER_COMPRESSION_TYPE_OGG_VORBIS
#endif /* HAVE_OGG_VORBIS */

    /* Ogg audio, as per RFC5334 */
    addMimeType("audio/ogg",         i18n("Ogg audio"),        "*.oga");
    addMimeType("application/ogg",   i18n("Ogg audio"),        "*.ogx");

}

//***************************************************************************
Kwave::OggDecoder::~OggDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Kwave::Decoder *Kwave::OggDecoder::instance()
{
    return new(std::nothrow) Kwave::OggDecoder();
}

//***************************************************************************
int Kwave::OggDecoder::parseHeader(QWidget *widget)
{
    // grab some data at the head of the stream.  We want the first page
    // (which is guaranteed to be small and only contain the Vorbis
    // stream initial header) We need the first page to get the stream
    // serialno.

    // submit a 4k block to libvorbis' Ogg layer
    char *buffer = ogg_sync_buffer(&m_oy, 4096);
    Q_ASSERT(buffer);
    if (!buffer) return -1;

    long int bytes = static_cast<long int>(m_source->read(buffer, 4096));
    if ((bytes <= 0) && (!m_source->pos())) {
        Kwave::MessageBox::error(widget, i18n(
            "Ogg bitstream has zero-length."));
        return -1;
    }
    ogg_sync_wrote(&m_oy, bytes);

    // Get the first page.
    if (ogg_sync_pageout(&m_oy, &m_og) != 1) {
        // have we simply run out of data?  If so, we're done.
        if (bytes < 4096) return 0;

        // error case. seems not be Vorbis data
        Kwave::MessageBox::error(widget, i18n(
             "Input does not appear to be an Ogg bitstream."));
        return -1;
    }

    // Get the serial number and set up the rest of decode.
    // serialno first; use it to set up a logical stream
    ogg_stream_init(&m_os, ogg_page_serialno(&m_og));

    // get the first packet
    if (ogg_stream_pagein(&m_os, &m_og) < 0) {
        // error; stream version mismatch perhaps
        Kwave::MessageBox::error(widget, i18n(
            "Error reading first page of the Ogg bitstream data."));
        return -1;
    }

    if ((ogg_stream_packetout(&m_os, &m_op) != 1) || (m_op.bytes < 8)) {
        // no page? must not be vorbis
        Kwave::MessageBox::error(widget, i18n(
            "Error reading initial header packet."));
        return -1;
    }

    // get rid of the previous sub decoder
    if (m_sub_decoder) {
        delete m_sub_decoder;
        m_sub_decoder = nullptr;
    }

    Kwave::FileInfo info(metaData());

    // ---------------------------------
    // auto-detect the sub decoder
#ifdef HAVE_OGG_OPUS
    if (memcmp(m_op.packet, "OpusHead", 8) == 0) {
        qDebug("    OggDecoder: detected Opus codec");
        m_sub_decoder = new(std::nothrow)
            Kwave::OpusDecoder(m_source, m_oy, m_os, m_og, m_op);
        info.set(Kwave::INF_MIMETYPE, _("audio/opus"));
    }
#endif /* HAVE_OGG_OPUS */
#ifdef HAVE_OGG_VORBIS
    if (memcmp(m_op.packet + 1, "vorbis", 6) == 0) {
        qDebug("    OggDecoder: detected Vorbis codec");
        m_sub_decoder = new(std::nothrow)
            Kwave::VorbisDecoder(m_source, m_oy, m_os, m_og, m_op);
        info.set(Kwave::INF_MIMETYPE, _("audio/x-vorbis+ogg"));
    }
#endif /* HAVE_OGG_VORBIS */

    if (!m_sub_decoder) {
        qDebug("--- dump of the first 8 bytes of the packet: ---");
        for (unsigned int i = 0; i < 8; i++)
            qDebug("%2u: 0x%02X - '%c'", i, m_op.packet[i], m_op.packet[i]);

        Kwave::MessageBox::error(widget, i18n(
            "Error: Codec not supported"));
        return -1;
    }

    info.setLength(0);         // use streaming
    info.setBits(SAMPLE_BITS); // use Kwave's internal resolution
    if (m_sub_decoder->open(widget, info) < 0)
        return -1;

    metaData().replace(Kwave::MetaDataList(info));
    return 1;
}

//***************************************************************************
bool Kwave::OggDecoder::open(QWidget *widget, QIODevice &src)
{
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("OggDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning("failed to open source !");
        return false;
    }

    // take over the source
    m_source = &src;

    /********** Decode setup ************/
    qDebug("--- OggDecoder::open() ---");
    ogg_sync_init(&m_oy); // Now we can read pages

    // read the header the first time
    if (parseHeader(widget) < 0)
        return false;

    return true;
}

//***************************************************************************
bool Kwave::OggDecoder::decode(QWidget *widget, Kwave::MultiWriter &dst)
{
    int eos = 0;

    Q_ASSERT(m_source);
    Q_ASSERT(m_sub_decoder);
    if (!m_source || !m_sub_decoder) return false;

    // we repeat if the bitstream is chained
    while (!dst.isCanceled()) {
        // The rest is just a straight decode loop until end of stream
        while (!eos) {
            while (!eos) {
                int result = ogg_sync_pageout(&m_oy, &m_og);
                if (result == 0) break; // need more data
                if (result < 0) {
                    // missing or corrupt data at this page position
                    Kwave::MessageBox::error(widget, i18n(
                        "Corrupt or missing data in bitstream. Continuing."
                        ));
                } else {
                    // can safely ignore errors at this point
                    ogg_stream_pagein(&m_os, &m_og);
                    while (1) {
                        result = ogg_stream_packetout(&m_os, &m_op);

                        if (result == 0) break; // need more data
                        if (result < 0) {
                            // missing or corrupt data at this page position
                            // no reason to complain; already complained above
                        } else {
                            result = m_sub_decoder->decode(dst);
                            if (result < 0)
                                break;

                            // signal the current position
                            emit sourceProcessed(m_source->pos());
                        }
                    }
                    if (ogg_page_eos(&m_og) || dst.isCanceled()) eos = 1;
                }
            }

            if (!eos) {
                char *buffer = ogg_sync_buffer(&m_oy, 4096);
                int bytes = Kwave::toInt(m_source->read(buffer, 4096));
                ogg_sync_wrote(&m_oy, bytes);
                if (!bytes) eos = 1;
            }
        }

        // clean up this logical bitstream; before exit we see if we're
        // followed by another [chained]
        ogg_stream_clear(&m_os);
        m_sub_decoder->reset();

        // parse the next header, maybe we parse a stream or chain...
        if (eos || (parseHeader(widget) < 1)) break;
    }

    // OK, clean up the framer
    ogg_sync_clear(&m_oy);

    // signal the current position
    emit sourceProcessed(m_source->pos());

    Kwave::FileInfo info(metaData());
    m_sub_decoder->close(info);
    metaData().replace(Kwave::MetaDataList(info));

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void Kwave::OggDecoder::close()
{
    m_source = nullptr;
    delete m_sub_decoder;
    m_sub_decoder = nullptr;
}

//***************************************************************************
//***************************************************************************
