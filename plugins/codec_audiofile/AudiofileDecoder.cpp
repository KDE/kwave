/*************************************************************************
   AudiofileDecoder.cpp  -  import through libaudiofile
                             -------------------
    begin                : Tue May 28 2002
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
#include <stdlib.h>

#include <new>

#include <audiofile.h>

#include <QIODevice>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"
#include "libkwave/Writer.h"

#include "AudiofileDecoder.h"

//***************************************************************************
Kwave::AudiofileDecoder::AudiofileDecoder()
    :Kwave::Decoder(), m_source(nullptr), m_src_adapter(nullptr)
{
    /* defined in RFC 1521 */
    addMimeType("audio/basic",
                i18n("NeXT, Sun Audio"),
                "*.au; *.snd");

    /* some others, mime types might be wrong (I found no RFC or similar)  */
    addMimeType("audio/x-8svx",
                i18n("Amiga IFF/8SVX Sound File Format"),
                "*.iff; *.8svx");
    addMimeType("audio/x-aifc",
                i18n("Compressed Audio Interchange Format"),
                "*.aifc");
    addMimeType("audio/x-aiff", /* included in KDE */
                i18n("Audio Interchange Format"),
                "*.aif; *.aiff");
    addMimeType("audio/x-avr",
                i18n("Audio Visual Research File Format"),
                "*.avr");
    addMimeType("audio/x-caf",
                i18n("Core Audio File Format"),
                "*.caf");
    addMimeType("audio/x-ircam",
                i18n("Berkeley, IRCAM, Carl Sound Format"),
                "*.sf");
    addMimeType("audio/x-nist",
                i18n("NIST SPHERE Audio File Format"),
                "*.nist");
    addMimeType("audio/x-smp",
                i18n("Sample Vision Format"),
                "*.smp");
    addMimeType("audio/x-voc",
                i18n("Creative Voice"),
                "*.voc");
}

//***************************************************************************
Kwave::AudiofileDecoder::~AudiofileDecoder()
{
    if (m_source) close();
    delete m_src_adapter;
    m_src_adapter = nullptr;
}

//***************************************************************************
Kwave::Decoder *Kwave::AudiofileDecoder::instance()
{
    return new(std::nothrow) Kwave::AudiofileDecoder();
}

//***************************************************************************
bool Kwave::AudiofileDecoder::open(QWidget *widget, QIODevice &src)
{
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("AudiofileDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning("AudiofileDecoder::open(), failed to open source !");
        return false;
    }

    // source successfully opened
    m_source = &src;
    m_src_adapter = new(std::nothrow) Kwave::VirtualAudioFile(*m_source);
    Q_ASSERT(m_src_adapter);
    if (!m_src_adapter) return false;

    m_src_adapter->open(m_src_adapter, nullptr);

    AFfilehandle fh = m_src_adapter->handle();
    if (!fh || (m_src_adapter->lastError() >= 0)) {
        QString reason;

        switch (m_src_adapter->lastError()) {
            case AF_BAD_NOT_IMPLEMENTED:
                reason = i18n("Format or function is not implemented");
                break;
            case AF_BAD_MALLOC:
                reason = i18n("Out of memory");
                break;
            case AF_BAD_HEADER:
                reason = i18n("File header is damaged");
                break;
            case AF_BAD_CODEC_TYPE:
                reason = i18n("Invalid codec type");
                break;
            case AF_BAD_OPEN:
                reason = i18n("Opening the file failed");
                break;
            case AF_BAD_READ:
                reason = i18n("Read access failed");
                break;
            case AF_BAD_SAMPFMT:
                reason = i18n("Invalid sample format");
                break;
            default:
                reason = reason.number(m_src_adapter->lastError());
        }

        QString text= i18n("An error occurred while opening the "\
            "file:\n'%1'", reason);
        Kwave::MessageBox::error(widget, text);

        return false;
    }

    AFframecount length = afGetFrameCount(fh, AF_DEFAULT_TRACK);
    unsigned int tracks = qMax(afGetVirtualChannels(fh, AF_DEFAULT_TRACK), 0);
    unsigned int bits = 0;
    double       rate = 0.0;
    int af_sample_format;
    afGetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, &af_sample_format,
        reinterpret_cast<int *>(&bits));
    Kwave::SampleFormat::Format fmt;
    switch (af_sample_format)
    {
        case AF_SAMPFMT_TWOSCOMP:
            fmt = Kwave::SampleFormat::Signed;
            break;
        case AF_SAMPFMT_UNSIGNED:
            fmt = Kwave::SampleFormat::Unsigned;
            break;
        case AF_SAMPFMT_FLOAT:
            fmt = Kwave::SampleFormat::Float;
            break;
        case AF_SAMPFMT_DOUBLE:
            fmt = Kwave::SampleFormat::Double;
            break;
        default:
            fmt = Kwave::SampleFormat::Unknown;
            break;
    }

    // get sample rate, with fallback to 8kHz
    rate = afGetRate(fh, AF_DEFAULT_TRACK);
    if (rate < 1.0) {
        qWarning("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"\
                 "WARNING: file has no sample rate!\n"\
                 "         => using 8000 samples/sec as fallback\n"\
                 "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        rate = 8000.0;
    }

    Kwave::SampleFormat::Map sf;
    QString sample_format_name = sf.description(Kwave::SampleFormat(fmt), true);

    if (static_cast<signed int>(bits) < 0) bits = 0;

    int af_compression = afGetCompression(fh, AF_DEFAULT_TRACK);
    const Kwave::Compression compression(
        Kwave::Compression::fromAudiofile(af_compression)
    );

    Kwave::FileInfo info(metaData());
    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(tracks);
    info.setLength(length);
    info.set(INF_SAMPLE_FORMAT, Kwave::SampleFormat(fmt).toInt());
    info.set(Kwave::INF_COMPRESSION, compression.toInt());
    metaData().replace(Kwave::MetaDataList(info));
    qDebug("-------------------------");
    qDebug("info:");
    qDebug("compression = %d", af_compression);
    qDebug("channels    = %u", info.tracks());
    qDebug("rate        = %0.0f", info.rate());
    qDebug("bits/sample = %u", info.bits());
    qDebug("length      = %lu samples",
           static_cast<unsigned long int>(info.length()));
    qDebug("format      = %d (%s)", af_sample_format,
                                    DBG(sample_format_name));
    qDebug("-------------------------");

    // set up libaudiofile to produce Kwave's internal sample format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
        AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    return true;
}

//***************************************************************************
bool Kwave::AudiofileDecoder::decode(QWidget */*widget*/,
                                     Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_src_adapter);
    Q_ASSERT(m_source);
    if (!m_source) return false;
    if (!m_src_adapter) return false;

    AFfilehandle fh = m_src_adapter->handle();
    Q_ASSERT(fh);
    if (!fh) return false;

    unsigned int frame_size = Kwave::toUint(
        afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 1));

    // allocate a buffer for input data
    const unsigned int buffer_frames = (8 * 1024);
    sample_storage_t *buffer =
        static_cast<sample_storage_t *>(malloc(buffer_frames * frame_size));
    Q_ASSERT(buffer);
    if (!buffer) return false;

    // read in from the audiofile source
    const unsigned int tracks = Kwave::FileInfo(metaData()).tracks();
    sample_index_t rest = Kwave::FileInfo(metaData()).length();
    while (rest) {
        unsigned int frames = buffer_frames;
        if (frames > rest) frames = Kwave::toUint(rest);
        int buffer_used = afReadFrames(fh,
            AF_DEFAULT_TRACK, reinterpret_cast<char *>(buffer), frames);

        // break if eof reached
        if (buffer_used <= 0) break;
        rest -= buffer_used;

        // split into the tracks
        const sample_storage_t *p = buffer;
        unsigned int count = buffer_used;
        while (count) {
            for (unsigned int track = 0; track < tracks; track++) {
                sample_storage_t s = *p++;

                // adjust precision
                if (SAMPLE_STORAGE_BITS != SAMPLE_BITS) {
                    s /= (1 << (SAMPLE_STORAGE_BITS - SAMPLE_BITS));
                }

                // the following cast is only necessary if
                // sample_t is not equal to a quint32
                *(dst[track]) << static_cast<sample_t>(s);
            }
            --count;
        }

        // abort if the user pressed cancel
        if (dst.isCanceled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    if (buffer) free(buffer);
    return true;
}

//***************************************************************************
void Kwave::AudiofileDecoder::close()
{
    if (m_src_adapter) delete m_src_adapter;
    m_src_adapter = nullptr;
    m_source = nullptr;
}

//***************************************************************************
//***************************************************************************
