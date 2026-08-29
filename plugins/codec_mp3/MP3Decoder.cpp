/*************************************************************************
         MP3Decoder.cpp  -  decoder for MP3 data
                             -------------------
    begin                : Wed Aug 07 2002
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

#include <cstdint>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include <taglib/commentsframe.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/mpegheader.h>
#include <taglib/mpegproperties.h>
#include <taglib/taglib.h>
#include <taglib/textidentificationframe.h>
#include <taglib/tiostream.h>

#include <QBuffer>
#include <QDate>
#include <QDateTime>
#include <QIODevice>
#include <QLatin1Char>
#include <QStringView>
#include <QTime>

#include "libkwave/Compression.h"
#include "libkwave/GenreType.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"

#include "TagLib_QIODeviceStream.h"
#include "MP3CodecPlugin.h"
#include "MP3Decoder.h"

//***************************************************************************
static inline QString parseId3Frame2String(const TagLib::ID3v2::Frame *frame)
{
    return QString::fromUtf8(frame->toString().to8Bit(true)).trimmed();
}

//***************************************************************************
/**
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
namespace Kwave {
    typedef struct {
        mad_fixed_t error[3];
        mad_fixed_t random;
    } audio_dither;
}

//***************************************************************************
Kwave::MP3Decoder::MP3Decoder()
    :Kwave::Decoder(),
     m_property_map(),
     m_source(nullptr),
     m_dest(nullptr),
     m_buffer(nullptr),
     m_buffer_size(0),
     m_prepended_bytes(0),
     m_appended_bytes(0),
     m_failures(0),
     m_parent_widget(nullptr)
{
    REGISTER_MIME_TYPES
    REGISTER_COMPRESSION_TYPES
}

//***************************************************************************
Kwave::MP3Decoder::~MP3Decoder()
{
    if (m_source) close();
    if (m_buffer) free(m_buffer);
}

//***************************************************************************
Kwave::Decoder *Kwave::MP3Decoder::instance()
{
    return new(std::nothrow) MP3Decoder();
}

//***************************************************************************
bool Kwave::MP3Decoder::parseMp3Header(const TagLib::MPEG::Header &header,
                                       QWidget *widget)
{
    Kwave::FileInfo info(metaData());

    /* MPEG layer */
    switch (header.layer()) {
        case 1:
            info.set(Kwave::INF_COMPRESSION,
                     QVariant(Kwave::Compression::MPEG_LAYER_I));
            info.set(Kwave::INF_MPEG_LAYER, QVariant(1));
            break;
        case 2:
            info.set(Kwave::INF_COMPRESSION,
                     QVariant(Kwave::Compression::MPEG_LAYER_II));
            info.set(Kwave::INF_MPEG_LAYER, QVariant(2));
            break;
        case 3:
            info.set(Kwave::INF_COMPRESSION,
                     QVariant(Kwave::Compression::MPEG_LAYER_III));
            info.set(Kwave::INF_MPEG_LAYER, QVariant(3));
            break;
        default:
            qWarning("unknown MPEG layer '%d'", header.layer());
            break;
    }

    /* MPEG version */
    switch (header.version()) {
        case TagLib::MPEG::Header::Version1:
            info.set(Kwave::INF_MPEG_VERSION, QVariant(1));
            break;
        case TagLib::MPEG::Header::Version2:
            info.set(Kwave::INF_MPEG_VERSION, QVariant(2));
            break;
        case TagLib::MPEG::Header::Version2_5:
            info.set(Kwave::INF_MPEG_VERSION, QVariant(2.5));
            break;
        default:
            qWarning("unknown MPEG version '%d'", header.version());
            break;
    }

    /* bit rate */
    if (header.bitrate() > 0) info.set(Kwave::INF_BITRATE_NOMINAL,
        QVariant(header.bitrate()));

    /* channel mode */
    unsigned int tracks = 0;
    switch (header.channelMode()) {
        case TagLib::MPEG::Header::SingleChannel:
            tracks = 1;
            break;
        case TagLib::MPEG::Header::Stereo:      /* FALLTHROUGH */
        case TagLib::MPEG::Header::JointStereo: /* FALLTHROUGH */
        case TagLib::MPEG::Header::DualChannel:
            tracks = 2;
            break;
        default:
            if (Kwave::MessageBox::warningContinueCancel(widget,
                i18n("The file contains an invalid channel mode (%1).\n"
                      "Assuming Mono...", int(header.channelMode())))
                     != KMessageBox::Continue) return false;
            tracks = 1;
            break;
    }
    info.setTracks(tracks);

    if (header.isCopyrighted()) info.set(Kwave::INF_COPYRIGHTED, true);
    if (header.isOriginal())    info.set(Kwave::INF_ORIGINAL, true);

    info.setBits(SAMPLE_BITS);      // fake Kwave's default resolution
    info.setRate(header.sampleRate());

    metaData().replace(Kwave::MetaDataList(info));

    return true;
}

//***************************************************************************
bool Kwave::MP3Decoder::parseID3Tags(TagLib::ID3v2::Tag *tag)
{
    if (!tag || tag->isEmpty()) return true; // nothing to do

    QDate creation_date;
    QTime creation_time;
    int year  = -1;
    int month = -1;
    int day   = -1;

    Kwave::FileInfo info(metaData());
    QVariantList custom_frames;

    const TagLib::ID3v2::FrameList &frames = tag->frameList();
    for (const TagLib::ID3v2::Frame *frame : frames) {
        if (!frame)
            continue;

        TagLib::ByteVector          id        = frame->frameID();
        Kwave::FileProperty         property  = m_property_map.property(id);
        TagLib_PropertyMap::Encoding encoding = m_property_map.encoding(id);

        switch (encoding) {
            case TagLib_PropertyMap::ENC_TEXT_PARTINSET:
            {
                QString s = parseId3Frame2String(frame);
                int cd  = 0;
                int cds = 0;
                if (s.contains(QLatin1Char('/'))) {
                    qsizetype i = s.indexOf(QLatin1Char('/'));
                    cd = s.left(i).toInt();
                    cds = s.mid(i + 1).toInt();
                } else {
                    cd = s.toInt();
                }
                if (cd  > 0) info.set(Kwave::INF_CD , QVariant(cd));
                if (cds > 0) info.set(Kwave::INF_CDS, QVariant(cds));
                break;
            }
            case TagLib_PropertyMap::ENC_TRACK_NUM:
            {
                QString s = parseId3Frame2String(frame);
                int track  = 0;
                int tracks = 0;
                if (s.contains(QLatin1Char('/'))) {
                    qsizetype i = s.indexOf(QLatin1Char('/'));
                    track = s.left(i).toInt();
                    tracks = s.mid(i + 1).toInt();
                } else {
                    track = s.toInt();
                }
                if (track  > 0) info.set(Kwave::INF_TRACK , QVariant(track));
                if (tracks > 0) info.set(Kwave::INF_TRACKS, QVariant(tracks));
                break;
            }
            case TagLib_PropertyMap::ENC_TERMS_OF_USE: /* FALLTHROUGH */
            case TagLib_PropertyMap::ENC_COMMENT:
            {
                // list of strings
                QString s = parseId3Frame2String(frame);

                // optionally prepend language
                const auto *comm_frame =
                dynamic_cast<const TagLib::ID3v2::CommentsFrame *>(frame);
                if (comm_frame) {
                    TagLib::ByteVector lang_bv = comm_frame->language();
                    if (!lang_bv.isEmpty()) {
                        QString lang = QString::fromLatin1(lang_bv.data(),
                                                           lang_bv.size());
                        if (!lang.isEmpty())
                            s = _("[") + lang + _("] ") + s;
                    }
                }

                QStringList list;
                if (info.contains(property))
                    list = info.get(property).toStringList();
                list.append(s);
                info.set(property, QVariant(list));
                break;
            }
            case TagLib_PropertyMap::ENC_GENRE_TYPE:
            {
                QString s = parseId3Frame2String(frame);
                int genre = Kwave::GenreType::fromID3(s);
                if (genre >= 0)
                    s = Kwave::GenreType::name(genre, false);
                info.set(property, QVariant(s));
                break;
            }
            case TagLib_PropertyMap::ENC_LENGTH:
            {
                // length in ms -> convert this to samples
                QString      s    = parseId3Frame2String(frame);
                const double rate = info.rate();
                bool         ok   = false;
                const double ms   = s.toDouble(&ok) + 0.5;
                if (ok && (rate > 0)) {
                    // NOTE: this overwrites the length found in the header!
                    sample_index_t length = static_cast<sample_index_t>(
                        (rate * ms) / 1000.0);
                    info.setLength(length);
                }
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_TIMESTAMP:
            {
                if (!creation_date.isValid()) {
                    QString s = parseId3Frame2String(frame);
                    if (id == "TDRC") {
                        // should be an iso 8601 timestamp or similar
                        s = Kwave::string2date(s);
                        if (s.length())
                            creation_date = QDate::fromString(s, Qt::ISODate);
                    } else if (id == "TDAT") {
                        // ddmm
                        unsigned int ddmm = s.toUInt();
                        day   = ddmm / 100;
                        month = ddmm % 100;
                    }
                    else if ((id == "TYER") || (id == "TORY"))
                        year = s.toUInt();
                }

                if (creation_time.isValid() && (id == "TIME"))
                    creation_time = QTime::fromString(_("hhmm"));
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_SLASH:
            {
                // append to already existing tag, separated by a slash
                QString s = parseId3Frame2String(frame);
                if (info.contains(property))
                    s = info.get(property).toString() + _(" / ") + s;
                info.set(property, QVariant(s));
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_URL: /* FALLTHROUGH */
            case TagLib_PropertyMap::ENC_TEXT:
                info.set(property, QVariant(parseId3Frame2String(frame)));
                break;
            case TagLib_PropertyMap::ENC_NONE: /* FALLTHROUGH */
            default:
            {
                qWarning("unsupported ID3 frame: %.*s",
                    static_cast<int>(id.size()), id.data());

                TagLib::ByteVector data = frame->render();
                QByteArray qdata(data.data(), static_cast<int>(data.size()));

                qDebug("-> storing, buffer size = %td bytes",
                        static_cast<ptrdiff_t>(qdata.size()));
                custom_frames.append(QVariant(
                    QString::fromLocal8Bit(qdata.toBase64())));
                break;
            }
        }
    }

    /* add custom (unsupported) ID3 frames 1:1 (just store internally) */
    if (!custom_frames.isEmpty())
        info.set(Kwave::INF_ID3, QVariant(custom_frames));

    /*
     * try to build a valid creation date/time
     */
    if (!creation_date.isValid()) {
        // no complete creation date - try to reassemble from found y/m/d
        creation_date = QDate(year, month, day);
    }
    if (creation_date.isValid() && creation_time.isValid()) {
        // full date + time
        QDateTime dt(creation_date, creation_time);
        info.set(Kwave::INF_CREATION_DATE, dt.toString(
            _("yyyy-MM-ddTHH:mm:ss")));
    } else if (creation_date.isValid()) {
        // date without time
        info.set(Kwave::INF_CREATION_DATE, creation_date.toString(
            _("yyyy-MM-dd")));
    } else if (year > 0) {
        // only year
        creation_date = QDate(year, 1, 1);
        info.set(Kwave::INF_CREATION_DATE, creation_date.toString(_("yyyy")));
    }

    metaData().replace(Kwave::MetaDataList(info));

    return true;
}

//***************************************************************************
bool Kwave::MP3Decoder::open(QWidget *widget, QIODevice &src)
{
    qDebug("MP3Decoder::open()");
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("MP3Decoder::open(), already open !");

    /* open the file in readonly mode with seek enabled */
    if (src.isSequential()) return false;
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning("unable to open source in read-only mode!");
        return false;
    }

    if (!src.seek(0))
        return false;

    // wrap QIODevice for TagLib
    Kwave::TagLib_QIODeviceStream stream(src);

    TagLib::MPEG::File mpeg_file(&stream);
    if (!mpeg_file.isValid())
        return false;

    // retrieve header directly from mpeg file
    TagLib::offset_t first_offset = mpeg_file.firstFrameOffset();
    if (first_offset < 0)
        return false;

    // save offset of first frame as prepended bytes (ID3v2 header size)
    m_prepended_bytes = (first_offset > 0) ?
        static_cast<size_t>(first_offset) : 0;

    TagLib::MPEG::Header mp3hdr(&mpeg_file, first_offset);
    if (!mp3hdr.isValid())
        return false;

    if (!parseMp3Header(mp3hdr, widget))
        return false;

    // get the estimated length by the time in ms and the sample rate
    if (mpeg_file.audioProperties()) {
        const TagLib::MPEG::Properties *properties = mpeg_file.audioProperties();
        const double rate = properties->sampleRate();
        const double ms   = properties->lengthInMilliseconds();
        if (rate > 0) {
            Kwave::FileInfo info(metaData());
            sample_index_t length = static_cast<sample_index_t>(
                (rate * ms) / 1000.0);
            info.set(Kwave::INF_ESTIMATED_LENGTH, static_cast<quint64>(length));
            metaData().replace(Kwave::MetaDataList(info));
        }
    }
    if (mpeg_file.hasID3v2Tag())
        parseID3Tags(mpeg_file.ID3v2Tag());

    if (mpeg_file.lastFrameOffset() > 0)
    {
        // calculate appended bytes after last mpeg frame
        // (e.g. ID3v1 or APETag)
        TagLib::offset_t last_frame_end = mpeg_file.lastFrameOffset() +
            mp3hdr.frameLength();
        if ((last_frame_end > 0) && (last_frame_end < src.size()))
            m_appended_bytes = static_cast<size_t>(src.size() - last_frame_end);
    }

    // read missing data directly from first MPEG frame header
    Kwave::FileInfo info(metaData());
    {
        unsigned char hdr_buf[MAD_BUFFER_MDLEN];
        if (src.seek(first_offset))
        {
            qint64 bytes_read = src.read(reinterpret_cast<char *>(hdr_buf),
                                         sizeof(hdr_buf));
            if (bytes_read > 0)
            {
                struct mad_stream hdr_stream;
                struct mad_header header;

                mad_stream_init(&hdr_stream);
                mad_header_init(&header);

                mad_stream_buffer(&hdr_stream, hdr_buf,
                                  static_cast<unsigned long int>(bytes_read));
                if (mad_header_decode(&header, &hdr_stream) == 0)
                {
                    // private bit
                    if (header.private_bits)
                        info.set(Kwave::INF_PRIVATE, true);

                    /* Emphasis mode */
                    // 0 = none
                    // 1 = 50/15ms
                    // 2 = reserved
                    // 3 = CCIT J.17
                    // if (header.emphasis() > 0)
                    // /* emphasis mode */
                    int emphasis = static_cast<int>(header.emphasis);
                    if (emphasis > 0)
                        info.set(Kwave::INF_MPEG_EMPHASIS, emphasis);

                    /* MPEG Mode Extension */
                    // only in "Joint Stereo" mode, then depends on Layer
                    //
                    // Layer I+II          |  Layer III
                    //                     |  Intensity stereo MS Stereo
                    //--------------------------------------------------
                    // 0 - bands  4 to 31  |  off              off  -> 4
                    // 1 - bands  8 to 31  |  on               off  -> 5
                    // 2 - bands 12 to 31  |  off              on   -> 6
                    // 3 - bands 16 to 31  |  on               on   -> 7
                    /* read mode extension */
                    int modeext = static_cast<int>(header.mode_extension);
                    if ((header.mode == MAD_MODE_JOINT_STEREO) &&
                        (header.layer == MAD_LAYER_III))
                        modeext += 4;
                    info.set(Kwave::INF_MPEG_MODEEXT, modeext);
                }

                mad_header_finish(&header);
                mad_stream_finish(&hdr_stream);
            }
        }
    }

    /* accept the source */
    m_source = &src;
    info.set(Kwave::INF_MIMETYPE, _("audio/mpeg"));
    metaData().replace(Kwave::MetaDataList(info));

    // allocate a transfer buffer with 128 kB
    if (m_buffer) free(m_buffer);
    m_buffer_size = (128 << 10);

    m_buffer = static_cast<unsigned char *>(malloc(m_buffer_size));
    if (!m_buffer) return false; // out of memory :-(

    return true;
}

//***************************************************************************
static enum mad_flow _input_adapter(void *data, struct mad_stream *stream)
{
    Kwave::MP3Decoder *decoder = reinterpret_cast<Kwave::MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ? decoder->fillInput(stream) : MAD_FLOW_STOP;
}

//***************************************************************************
static enum mad_flow _output_adapter(void *data,
                                     struct mad_header const *header,
                                     struct mad_pcm *pcm)
{
    Kwave::MP3Decoder *decoder = reinterpret_cast<Kwave::MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ?
        decoder->processOutput(data, header, pcm) : MAD_FLOW_STOP;
}

//***************************************************************************
static enum mad_flow _error_adapter(void *data, struct mad_stream *stream,
                                    struct mad_frame *frame)
{
    Kwave::MP3Decoder *decoder = reinterpret_cast<Kwave::MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ?
        decoder->handleError(data, stream, frame) : MAD_FLOW_BREAK;
}

//***************************************************************************
enum mad_flow Kwave::MP3Decoder::handleError(void *data,
    struct mad_stream *stream, struct mad_frame *frame)
{
    Q_UNUSED(data);
    Q_UNUSED(frame);

    if (m_failures >= 2) return MAD_FLOW_CONTINUE; // ignore errors
    if (stream->error == MAD_ERROR_NONE) return MAD_FLOW_CONTINUE; // ???

    QString error;
    switch (stream->error) {
        case MAD_ERROR_BUFLEN:
        case MAD_ERROR_BUFPTR:
        case MAD_ERROR_NOMEM:
            error = i18n("Out of memory");
            break;
        case MAD_ERROR_BADCRC:
            error = i18n("Checksum error");
            break;
        case MAD_ERROR_LOSTSYNC:
            error = i18n("Synchronization lost");
            break;
        case MAD_ERROR_BADLAYER:
        case MAD_ERROR_BADBITRATE:
        case MAD_ERROR_BADSAMPLERATE:
        case MAD_ERROR_BADEMPHASIS:
        case MAD_ERROR_BADBITALLOC:
        case MAD_ERROR_BADSCALEFACTOR:
        case MAD_ERROR_BADFRAMELEN:
        case MAD_ERROR_BADBIGVALUES:
        case MAD_ERROR_BADBLOCKTYPE:
        case MAD_ERROR_BADSCFSI:
        case MAD_ERROR_BADDATAPTR:
        case MAD_ERROR_BADPART3LEN:
        case MAD_ERROR_BADHUFFTABLE:
        case MAD_ERROR_BADHUFFDATA:
        case MAD_ERROR_BADSTEREO:
            error = i18n("File contains invalid data");
            break;
        default:
            QString err_hex = QString::number(
                static_cast<int>(stream->error), 16).toUpper();
            error = i18n("Unknown error 0x%1. Damaged file?", err_hex);
            break;
    }

    long unsigned int pos = stream->this_frame - m_buffer;
    error = i18n("An error occurred while decoding the file:\n'%1',\n"
                 "at position %2.", error, pos);
    if (!m_failures) {
        m_failures = 1;
        int result = Kwave::MessageBox::warningContinueCancel(m_parent_widget,
                 error + _("\n") + i18n("Do you still want to continue?"));
        if (result != KMessageBox::Continue) return MAD_FLOW_BREAK;
    } else if (m_failures == 1) {
        int result = Kwave::MessageBox::warningYesNo(m_parent_widget,
            error + _("\n") +
            i18n("Do you want to continue and ignore all following errors?"));
        m_failures++;
        if (result != KMessageBox::PrimaryAction) return MAD_FLOW_BREAK;
    }

    return MAD_FLOW_CONTINUE;
}

//***************************************************************************
enum mad_flow Kwave::MP3Decoder::fillInput(struct mad_stream *stream)
{
    Q_ASSERT(m_source);
    if (!m_source) return MAD_FLOW_STOP;

    // check if the user pressed cancel
    if (m_dest->isCanceled()) return MAD_FLOW_STOP;

    // preserve the remaining bytes from the last pass
    size_t rest = stream->bufend - stream->next_frame;
    if (rest) memmove(m_buffer, stream->next_frame, rest);

    // clip source at "eof-appended_bytes"
    size_t bytes_to_read = m_buffer_size - rest;
    if (m_source->pos() + bytes_to_read > m_source->size() - m_appended_bytes)
        bytes_to_read = Kwave::toUint(
            m_source->size() - m_appended_bytes - m_source->pos());

    // abort if nothing more to read, even if there are
    // some "left-overs" from the previous pass
    if (!bytes_to_read) return MAD_FLOW_STOP;

    // read from source to fill up the buffer
    size_t size = rest;
    size += m_source->read(
        reinterpret_cast<char *>(m_buffer) + rest, bytes_to_read);
    if (!size) return MAD_FLOW_STOP; // no more data

    // buffer is filled -> process it
    mad_stream_buffer(stream, m_buffer, size);

    // signal the current position
    emit sourceProcessed(m_source->pos());

    return MAD_FLOW_CONTINUE;
}

//***************************************************************************
/**
 * 32-bit pseudo-random number generator
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
static inline quint32 prng(quint32 state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

//***************************************************************************
/**
 * generic linear sample quantize and dither routine
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
static inline qint32 audio_linear_dither(unsigned int bits,
    mad_fixed_t sample, Kwave::audio_dither *dither)
{
    unsigned int scalebits;
    mad_fixed_t output, mask, random;

    enum {
        MIN = -MAD_F_ONE,
        MAX =  MAD_F_ONE - 1
    };

    /* noise shape */
    sample += dither->error[0] - dither->error[1] + dither->error[2];

    dither->error[2] = dither->error[1];
    dither->error[1] = dither->error[0] / 2;

    /* bias */
    output = sample + mad_fixed_t(1L << (MAD_F_FRACBITS + 1 - bits - 1));

    scalebits = MAD_F_FRACBITS + 1 - bits;
    mask = mad_fixed_t(1L << scalebits) - 1;

    /* dither */
    random  = static_cast<mad_fixed_t>(prng(dither->random));
    output += (random & mask) - (dither->random & mask);

    dither->random = random;

    /* clip */
    if (output > MAX) {
        output = MAX;
        if (sample > MAX) sample = MAX;
    } else if (output < MIN) {
        output = MIN;
        if (sample < MIN) sample = MIN;
    }

    /* quantize */
    output &= ~mask;

    /* error feedback */
    dither->error[0] = sample - output;

    /* scale */
    return output >> scalebits;
}

//***************************************************************************
enum mad_flow Kwave::MP3Decoder::processOutput(
    void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
    static Kwave::audio_dither dither;
    qint32 sample;
    Kwave::SampleArray buffer(pcm->length);

    Q_UNUSED(data);
    Q_UNUSED(header);

    // loop over all tracks
    const unsigned int tracks = m_dest->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
        unsigned int nsamples = pcm->length;
        mad_fixed_t const *p = pcm->samples[track];
        unsigned int ofs = 0;

        // and render samples into Kwave's internal format
        while (nsamples--) {
            sample = static_cast<qint32>(audio_linear_dither(SAMPLE_BITS,
                     static_cast<mad_fixed_t>(*p++), &dither));
            buffer[ofs++] = static_cast<sample_t>(sample);
        }
        *(*m_dest)[track] << buffer;
    }

    return MAD_FLOW_CONTINUE;
}

//***************************************************************************
bool Kwave::MP3Decoder::decode(QWidget *widget, Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_source);
    if (!m_source) return false;
    m_source->seek(m_prepended_bytes); // skip id3v2 tag

    // set target of the decoding
    m_dest = &dst;
    m_failures = 0;
    m_parent_widget = widget;

    // setup the decoder
    struct mad_decoder decoder;
    mad_decoder_init(&decoder, this,
                     _input_adapter,
                     nullptr /* header */,
                     nullptr /* filter */,
                     _output_adapter,
                     _error_adapter,
                     nullptr /* message */);

    // decode through libmad...
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    // release the decoder
    mad_decoder_finish(&decoder);

    return (result == 0);
}

//***************************************************************************
void Kwave::MP3Decoder::close()
{
    m_source = nullptr;
}

//***************************************************************************
//***************************************************************************
