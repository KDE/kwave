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

#include <QFutureSynchronizer>
#include <QIODevice>
#include <QtConcurrentRun>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"
#include "libkwave/Writer.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"

#define addAfMimeType(m, d, e, t) addMimeType(m, d, e)

#define MAX_META_DATA_LEN   1024 /**< max length of one meta data entry */
#define MAX_META_DATA_COUNT 1024 /**< max number of meta data entries   */

//***************************************************************************
Kwave::AudiofileDecoder::AudiofileDecoder()
    :Kwave::Decoder(), m_source(nullptr), m_src_adapter(nullptr)
{
    REGISTER_MIME_TYPES
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
    decodeFileInfo(fh, info);
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

    // get the cue list (aka "markers" / "labels")
    decodeMarkers(fh);

    return true;
}

//***************************************************************************
void Kwave::AudiofileDecoder::decodeFileInfo(AFfilehandle fh,
                                             Kwave::FileInfo &info)
{
    int count = afGetMiscIDs(fh, NULL);
    if (count < 1) return;

    QVarLengthArray<int, 16> ids(count);
    afGetMiscIDs(fh, ids.data());
    for (int i = 0; i < count; i++) {
        if (i > MAX_META_DATA_COUNT) break;
        int misctype = afGetMiscType(fh, ids[i]);
        int datasize = afGetMiscSize(fh, ids[i]);
        if (datasize < 1) continue;
        if (datasize > MAX_META_DATA_LEN) continue;
        QByteArray data(datasize, Qt::Uninitialized);
        int res = afReadMisc(fh, ids[i], data.data(), datasize);
        if (res != datasize) continue;

        Kwave::FileProperty prop(INF_UNKNOWN);
        switch (misctype) {
            case AF_MISC_COPY:    prop = Kwave::INF_COPYRIGHT;     break;
            case AF_MISC_AUTH:    prop = Kwave::INF_AUTHOR;        break;
            case AF_MISC_NAME:    prop = Kwave::INF_NAME;          break;
            case AF_MISC_ANNO:    prop = Kwave::INF_ANNOTATION;    break;
            case AF_MISC_COMMENT: prop = Kwave::INF_COMMENTS;      break;
            case AF_MISC_ICRD:    prop = Kwave::INF_CREATION_DATE; break;
            case AF_MISC_ISFT:    prop = Kwave::INF_SOFTWARE;      break;
            default: break;
        }
        if (prop != INF_UNKNOWN)
        {
            QString value = QString::fromUtf8(data.constData()).trimmed();
            if (!value.isEmpty())
                info.set(prop, value);
        }
    }
}

//***************************************************************************
void Kwave::AudiofileDecoder::decodeMarkers(AFfilehandle fh)
{
    int num_labels = afGetMarkIDs(fh, AF_DEFAULT_TRACK, NULL);
    if (num_labels < 1) return;

    Kwave::LabelList labels;
    QVarLengthArray<int, 16> ids(num_labels);
    afGetMarkIDs(fh, AF_DEFAULT_TRACK, ids.data());

    for (int i = 0; i < num_labels; i++) {
        const char *n = afGetMarkName(fh, AF_DEFAULT_TRACK, ids[i]);
        QString name = QString::fromUtf8(n);

        const char *c = afGetMarkComment(fh, AF_DEFAULT_TRACK, ids[i]);
        QString comment = QString::fromUtf8(c);

        AFframecount p = afGetMarkPosition(fh, AF_DEFAULT_TRACK, ids[i]);
        sample_index_t pos = static_cast<sample_index_t>(p);

        QString txt = comment;
        if (txt.isEmpty()) txt = name;
        labels.append(Kwave::Label(pos, txt.trimmed()));
    }

    metaData().replace(labels.toMetaDataList());
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
    const unsigned int buffer_frames = (64 * 1024);
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

        // parallel deinterleaving and writing directly per track
        unsigned int step  = tracks;
        QFutureSynchronizer<void> synchronizer;

        for (unsigned int track = 0; track < tracks; ++track) {
            Kwave::Writer          *writer = dst[track];
            const sample_storage_t *in     = buffer + track;
            if (!writer) continue;

            synchronizer.addFuture(QtConcurrent::run(
                [in, buffer_used, step, writer]() {
                    unsigned int remaining = buffer_used;
                    const sample_storage_t *src = in;
                    while (remaining-- > 0) {
                        sample_storage_t s = *src;
                        src += step;

                        // adjust precision
                        if (SAMPLE_STORAGE_BITS != SAMPLE_BITS) {
                            s /= (1 << (SAMPLE_STORAGE_BITS - SAMPLE_BITS));
                        }

                        // the following cast is only necessary if
                        // sample_t is not equal to a quint32
                        *(writer) << static_cast<sample_t>(s);
                    }
                }
            ));
        }
        synchronizer.waitForFinished();

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
