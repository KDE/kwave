/*************************************************************************
         MP3Encoder.cpp  -  export of MP3 data via "lame"
                             -------------------
    begin                : Sat May 19 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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
#include <new>

#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/unknownframe.h>

#include <QBuffer>
#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QLatin1Char>
#include <QList>
#include <QMap>

#include <KLocalizedString>

#include "libkwave/FileInfo.h"
#include "libkwave/GenreType.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MixerMatrix.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "MP3CodecPlugin.h"
#include "MP3Encoder.h"
#include "MP3EncoderSettings.h"
#include "TagLib_PropertyMap.h"

/***************************************************************************/
Kwave::MP3Encoder::MP3Encoder()
    :Kwave::Encoder(),
     m_property_map(),
     m_lock(),
     m_dst(nullptr),
     m_process(this),
     m_program(),
     m_params()
{
    REGISTER_MIME_TYPES
    REGISTER_COMPRESSION_TYPES

    connect(&m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(dataAvailable()));
}

/***************************************************************************/
Kwave::MP3Encoder::~MP3Encoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::MP3Encoder::instance()
{
    return new(std::nothrow) MP3Encoder();
}

/***************************************************************************/
QList<Kwave::SampleFormat::Format> Kwave::MP3Encoder::supportedSampleFormats(
    const FileInfo &info) const
{
    Q_UNUSED(info);
    // return empty list -> configured via MP3 settings dialog
    return QList<Kwave::SampleFormat::Format>();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::MP3Encoder::supportedProperties()
{
    return m_property_map.properties();
}

/***************************************************************************/
void Kwave::MP3Encoder::encodeID3Tags(const Kwave::MetaDataList &meta_data,
                                      TagLib::ID3v2::Tag &tag)
{
    // encode meta data with taglib
    const Kwave::FileInfo info(meta_data);
    const QMap<Kwave::FileProperty, QVariant> props = info.properties();

    for (auto it = props.constBegin(); it != props.constEnd(); ++it)
    {
        const Kwave::FileProperty prop = it.key();

        // skip separate total track count if already merged into TRCK
        if ((prop == Kwave::INF_TRACKS) && info.contains(Kwave::INF_TRACK))
            continue;

        // skip separate total cd count if already merged into TPOS
        if ((prop == Kwave::INF_CDS) && info.contains(Kwave::INF_CD))
            continue;

        const TagLib::ByteVector frame_id = m_property_map.findProperty(prop);
        if (frame_id.isEmpty())
            continue;

        const TagLib_PropertyMap::Encoding enc =
            m_property_map.encoding(frame_id);
        switch (enc)
        {
            case TagLib_PropertyMap::ENC_COMMENT:
            {
                // create comment frame with language code
                const QStringList list = it.value().toStringList();
                for (QString c : list)
                {
                    if (c.isEmpty())
                        continue;

                    TagLib::ByteVector lang("eng", 3);
                    if ((c.length() > 5) &&
                        (c.at(0) == QLatin1Char('[')) &&
                        (c.at(4) == QLatin1Char(']')))
                    {
                        const QString l = c.mid(1, 3);
                        lang = TagLib::ByteVector(l.toLatin1().constData(), 3);
                        c = c.mid(5).trimmed();
                    }

                    TagLib::ID3v2::CommentsFrame *frame =
                    new(std::nothrow) TagLib::ID3v2::CommentsFrame(
                        TagLib::String::UTF8);
                    if (!frame)
                        continue;

                    frame->setLanguage(lang);
                    frame->setText(
                        TagLib::String(c.toUtf8().constData(),
                                       TagLib::String::UTF8));
                    tag.addFrame(frame);
                }
                break;
            }
            case TagLib_PropertyMap::ENC_TRACK_NUM:
            {
                // format track number as "track/total" if total tracks count
                QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                if ((prop == Kwave::INF_TRACK) && info.contains(Kwave::INF_TRACKS))
                {
                    const QString total = info.get(Kwave::INF_TRACKS).toString();
                    if (!total.isEmpty())
                        val += QLatin1Char('/') + total;
                }

                TagLib::ID3v2::TextIdentificationFrame *frame =
                new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                    frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                frame->setText(
                    TagLib::String(val.toUtf8().constData(),
                                   TagLib::String::UTF8));
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_PARTINSET:
            {
                // format disc number as "disc/total" if total discs count
                QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                if ((prop == Kwave::INF_CD) && info.contains(Kwave::INF_CDS))
                {
                    const QString total = info.get(Kwave::INF_CDS).toString();
                    if (!total.isEmpty())
                        val += QLatin1Char('/') + total;
                }

                TagLib::ID3v2::TextIdentificationFrame *frame =
                new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                    frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                frame->setText(
                    TagLib::String(val.toUtf8().constData(),
                                   TagLib::String::UTF8));
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_GENRE_TYPE:
            {
                // format genre name from numerical id if known
                QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                const int genre = Kwave::GenreType::fromID3(val);
                if (genre >= 0)
                    val = Kwave::GenreType::name(genre, false);

                TagLib::ID3v2::TextIdentificationFrame *frame =
                new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                    frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                frame->setText(
                    TagLib::String(val.toUtf8().constData(),
                                   TagLib::String::UTF8));
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_TIMESTAMP:
            {
                // format timestamp or year
                const QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                QString s = Kwave::string2date(val);
                if (s.isEmpty())
                {
                    const int year = val.toInt();
                    if ((year > 0) && (year < 9999))
                        s = _("%1").arg(year, 4, 10, QLatin1Char('0'));
                }
                if (s.isEmpty())
                    break;

                TagLib::ID3v2::TextIdentificationFrame *frame =
                new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                    frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                frame->setText(
                    TagLib::String(s.toUtf8().constData(),
                                   TagLib::String::UTF8));
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT_LIST:      /* FALLTHROUGH */
            case TagLib_PropertyMap::ENC_TEXT_SLASH:
            {
                // create text identification frame with list of strings
                const QStringList list = it.value().toStringList();
                if (list.isEmpty())
                    break;

                TagLib::ID3v2::TextIdentificationFrame *frame =
                new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                    frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                TagLib::StringList taglib_list;
                for (const QString &item : list)
                {
                    if (!item.isEmpty())
                        taglib_list.append(
                            TagLib::String(item.toUtf8().constData(),
                                           TagLib::String::UTF8));
                }
                if (taglib_list.isEmpty())
                {
                    delete frame;
                    break;
                }

                frame->setText(taglib_list);
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_TEXT:           /* FALLTHROUGH */
            case TagLib_PropertyMap::ENC_TEXT_URL:
            {
                // create text identification frame
                const QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                TagLib::ID3v2::TextIdentificationFrame *frame =
                    new(std::nothrow) TagLib::ID3v2::TextIdentificationFrame(
                        frame_id, TagLib::String::UTF8);
                if (!frame)
                    break;

                frame->setText(
                    TagLib::String(val.toUtf8().constData(),
                                   TagLib::String::UTF8));
                tag.addFrame(frame);
                break;
            }
            case TagLib_PropertyMap::ENC_BINARY:
            {
                // add raw binary data frame(s) from base64 string list
                const QStringList frames = it.value().toStringList();
                for (const QString &f : frames)
                {
                    const QByteArray data = QByteArray::fromBase64(f.toLatin1());
                    if (data.isEmpty())
                        continue;

                    TagLib::ID3v2::UnknownFrame *frame =
                        new(std::nothrow) TagLib::ID3v2::UnknownFrame(frame_id);
                    if (!frame)
                        continue;

                    const unsigned int len =
                        static_cast<unsigned int>(data.size());
                    frame->setData(TagLib::ByteVector(data.constData(), len));
                    tag.addFrame(frame);
                }
                break;
            }
            case TagLib_PropertyMap::ENC_TERMS_OF_USE:
            {
                // add terms of use text frame
                const QString val = it.value().toString();
                if (val.isEmpty())
                    break;

                TagLib::ID3v2::UnknownFrame *frame =
                    new(std::nothrow) TagLib::ID3v2::UnknownFrame(frame_id);
                if (!frame)
                    break;

                const QByteArray utf8 = val.toUtf8();
                const unsigned int len =
                    static_cast<unsigned int>(utf8.size());
                TagLib::ByteVector payload;
                payload.append('\x03');
                payload.append(TagLib::ByteVector("eng", 3));
                payload.append(TagLib::ByteVector(utf8.constData(), len));
                frame->setData(payload);
                tag.addFrame(frame);
                break;
            }
            default:
                break;
        }
    }
}

#define OPTION(__field__) \
    if (settings.__field__.length()) m_params.append(settings.__field__)

#define OPTION_P(__field__, __value__) \
    if (settings.__field__.length()) \
        m_params.append( \
            QString(settings.__field__.arg(__value__)).split(QLatin1Char(' ')))

/***************************************************************************/
bool Kwave::MP3Encoder::encode(QWidget *widget, Kwave::MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    bool result = true;
    TagLib::ID3v2::Tag id3_tag;
    Kwave::MP3EncoderSettings settings;

    settings.load();

    const Kwave::FileInfo info(meta_data);

    // get info: tracks, sample rate
    const unsigned int tracks     = src.tracks();
    const sample_index_t length   = src.last() - src.first() + 1;
    unsigned int       bits       = qBound(8U, ((info.bits() + 7) & ~0x7), 32U);
    const double       rate       = info.rate();
    const unsigned int out_tracks = qMin(tracks, 2U);

    // when encoding track count > 2, show a warning that we will mix down
    // to stereo
    if (tracks > 2) {
        if (Kwave::MessageBox::warningContinueCancel(
            widget,
            i18n("The file format you have chosen supports only mono or "
                 "stereo. This file will be mixed down to stereo when "
                 "saving."),
            QString(), QString(), QString(),
            _("mp3_accept_down_mix_on_export")) != KMessageBox::Continue)
        {
            return false;
        }
    }

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        Kwave::MessageBox::error(widget,
            i18n("Unable to open the file for saving!"));
        return false;
    }

    m_dst  = &dst;
    m_params.clear();

    // encode meta data with taglib and write id3v2 header to stream
    encodeID3Tags(meta_data, id3_tag);
    const TagLib::ByteVector tag_data = id3_tag.render();
    if (!tag_data.isEmpty())
        dst.write(tag_data.data(), tag_data.size());

    OPTION(m_flags.m_prepend);          // optional parameters at the very start

    // mandantory audio input format and encoding options
    OPTION(m_input.m_raw_format);       // input is raw audio
    OPTION(m_input.m_byte_order);       // byte swapping
    OPTION(m_input.m_signed);           // signed sample format

    // supported sample rates [kHz]
    // 8 / 11.025 / 12 / 16 / 22.05 / 24 /32 / 44.1 / 48
    // if our rate is not supported, lame automatically resamples with the
    // next higher supported rate
    if (settings.m_format.m_sample_rate.length()) {
        QString str = settings.m_format.m_sample_rate;
        if (str.contains(_("[%khz]"))) {
            str = str.replace(_("[%khz]"),
                _("%1")).arg(rate / 1000.0, 1, 'f', 2);
            m_params.append(str.split(QLatin1Char(' ')));
        } else {
            m_params.append(str.arg(rate).split(QLatin1Char(' ')));
        }
    }

    // bits per sample, supported by Kwave are: 8 / 16 / 24 / 32
    if (!settings.m_format.m_bits_per_sample.contains(QLatin1Char('%'))) {
        // bits/sample are not selectable => use default=16bit
        bits = 16;
        OPTION(m_format.m_bits_per_sample);
    } else {
        OPTION_P(m_format.m_bits_per_sample, bits);
    }

    // encode one track as "mono" and two tracks as "joint-stereo"
    if (tracks == 1) {
        OPTION(m_format.m_channels.m_mono);
    } else {
        OPTION(m_format.m_channels.m_stereo);
    }

    // nominal / lower / upper bitrate
    int bitrate_min =   8;
    int bitrate_max = 320;
    int bitrate_nom = 128;
    if (info.contains(Kwave::INF_BITRATE_NOMINAL)) {
        // nominal bitrate => use ABR mode
        bitrate_nom = info.get(Kwave::INF_BITRATE_NOMINAL).toInt() / 1000;
        bitrate_nom = qBound(bitrate_min, bitrate_nom, bitrate_max);
        OPTION_P(m_quality.m_bitrate.m_avg, bitrate_nom);
    }
    if (info.contains(Kwave::INF_BITRATE_LOWER)) {
        int bitrate = info.get(Kwave::INF_BITRATE_LOWER).toInt() / 1000;
        bitrate_min = qBound(bitrate_min, bitrate, bitrate_nom);
        OPTION_P(m_quality.m_bitrate.m_min, bitrate_min);
    }
    if (info.contains(Kwave::INF_BITRATE_UPPER)) {
        int bitrate = info.get(Kwave::INF_BITRATE_UPPER).toInt() / 1000;
        bitrate_max = qBound(bitrate_nom, bitrate, bitrate_max);
        OPTION_P(m_quality.m_bitrate.m_max, bitrate_max);
    }
    //  Kwave::INF_MPEG_LAYER,          /**< MPEG Layer, I/II/III */
    //  Kwave::INF_MPEG_MODEEXT,        /**< MPEG mode extension */
    //  Kwave::INF_MPEG_VERSION,        /**< MPEG version */

    /* MPEG emphasis mode */
    if (info.contains(Kwave::INF_MPEG_EMPHASIS)) {
        int emphasis = info.get(Kwave::INF_MPEG_EMPHASIS).toInt();
        switch (emphasis) {
            case  1:
                OPTION(m_encoding.m_emphasis.m_50_15ms);  // 1 = 50/15ms
                break;
            case  3:
                OPTION(m_encoding.m_emphasis.m_ccit_j17); // 3 = CCIT J.17
                break;
            case  0: /* FALLTHROUGH */
            default:
                OPTION(m_encoding.m_emphasis.m_none);     // 0 = none
                break;
        }
    }

    OPTION(m_encoding.m_noise_shaping); // noise shaping settings
    OPTION(m_encoding.m_compatibility); // compatibility options

    if (info.contains(Kwave::INF_COPYRIGHTED) &&
        info.get(Kwave::INF_COPYRIGHTED).toBool()) {
        OPTION(m_flags.m_copyright);     // copyrighted
    }

    if ( info.contains(Kwave::INF_ORIGINAL) &&
        !info.get(Kwave::INF_ORIGINAL).toBool()) {
        OPTION(m_flags.m_original);     // original
    }

    OPTION(m_flags.m_protect);          // CRC protection
    OPTION(m_flags.m_append);           // optional parameters at the end

    m_params.append(_("-")); // infile  = stdin
    m_params.append(_("-")); // outfile = stdout

    m_program = settings.m_path;

    qDebug("MP3Encoder::encode(): %s %s",
           DBG(m_program), DBG(m_params.join(_(" ")))
    );

    m_process.setReadChannel(QProcess::StandardOutput);

    m_process.start(m_program, m_params);
    if (!m_process.waitForStarted()) {
        qWarning("cannot start program '%s'", DBG(m_program));
        m_process.waitForFinished();
        result = false;
    }

    // MP3 supports only mono and stereo, prepare a mixer matrix
    // (not used in case of tracks <= 2)
    Kwave::MixerMatrix mixer(tracks, out_tracks);

    // read in from the sample readers
    const unsigned int buf_len = sizeof(m_write_buffer);
    const int bytes_per_sample = bits / 8;

    sample_index_t rest = length;
    Kwave::SampleArray in_samples(tracks);
    Kwave::SampleArray out_samples(tracks);

    while (result && rest && (m_process.state() != QProcess::NotRunning)) {
        unsigned int x;
        unsigned int y;

        // merge the tracks into the sample buffer
        quint8 *dst_buffer = &(m_write_buffer[0]);
        unsigned int count = buf_len / (bytes_per_sample * tracks);
        if (rest < count) count = Kwave::toUint(rest);

        unsigned int written = 0;
        for (written = 0; written < count; written++) {
            const sample_t *src_buf = nullptr;

            // fill input buffer with samples
            for (x = 0; x < tracks; ++x) {
                in_samples[x] = 0;
                Kwave::SampleReader *stream = src[x];
                Q_ASSERT(stream);
                if (!stream) continue;

                if (!stream->eof()) (*stream) >> in_samples[x];
            }

            if (tracks > 2) {
                // multiply matrix with input to get output
                const Kwave::SampleArray &in = in_samples;
                for (y = 0; y < out_tracks; ++y) {
                    double sum = 0;
                    for (x = 0; x < tracks; ++x)
                        sum += static_cast<double>(in[x]) * mixer[x][y];
                    out_samples[y] = static_cast<sample_t>(sum);
                }

                // use output of the matrix
                src_buf = out_samples.constData();
            } else {
                // use input buffer directly
                src_buf = in_samples.constData();
            }

            // sample conversion from 24bit to raw PCM, native endian
            for (y = 0; y < out_tracks; ++y) {
                sample_t s = *(src_buf++);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                // big endian
                if (bits >= 8)
                    *(dst_buffer++) = static_cast<quint8>(s >> 16);
                if (bits > 8)
                    *(dst_buffer++) = static_cast<quint8>(s >> 8);
                if (bits > 16)
                    *(dst_buffer++) = static_cast<quint8>(s & 0xFF);
                if (bits > 24)
                    *(dst_buffer++) = 0x00;
#else
                // little endian
                if (bits > 24)
                    *(dst_buffer++) = 0x00;
                if (bits > 16)
                    *(dst_buffer++) = static_cast<quint8>(s & 0xFF);
                if (bits > 8)
                    *(dst_buffer++) = static_cast<quint8>(s >> 8);
                if (bits >= 8)
                    *(dst_buffer++) = static_cast<quint8>(s >> 16);
#endif
            }
        }

        // write out to the stdin of the external process
        qint64 bytes_written = m_process.write(
            reinterpret_cast<char *>(&(m_write_buffer[0])),
            written * (bytes_per_sample * tracks)
        );

        // break if eof reached or disk full
        if (!bytes_written) break;

        // wait for write to take all data...
        m_process.waitForBytesWritten();

        // abort if the user pressed cancel
        // --> this would leave a corrupted file !!!
        if (src.isCanceled()) break;

        Q_ASSERT(rest >= written);
        rest -= written;
    }

    // flush and close the write channel
    m_process.closeWriteChannel();

    // wait until the process has finished
    qDebug("wait for finish of the process");
    while (m_process.state() != QProcess::NotRunning) {
        m_process.waitForFinished(100);
        if (src.isCanceled()) break;
    }

    int exit_code = m_process.exitCode();
    qDebug("exit code=%d", exit_code);
    if (!result || (exit_code != 0)) {
        result = false;
        QString stdError = QString::fromLocal8Bit(
            m_process.readAllStandardError());
        qWarning("stderr output: %s", DBG(stdError));

        Kwave::MessageBox::error(widget,
            i18nc("%1=name of the external program, %2=stderr of the program",
            "An error occurred while calling the external encoder '%1':\n\n%2",
           m_program, stdError
        ));
    }

    {
        QMutexLocker _lock(&m_lock);
        m_dst = nullptr;
        dst.close();
    }

    return result;
}

/***************************************************************************/
void Kwave::MP3Encoder::dataAvailable()
{
    while (m_process.bytesAvailable()) {
        qint64 len = m_process.read(&(m_read_buffer[0]), sizeof(m_read_buffer));
        if (len) {
            QMutexLocker _lock(&m_lock);
            if (m_dst) m_dst->write(&(m_read_buffer[0]), len);
        }
    }
}

/***************************************************************************/
/***************************************************************************/

#include "moc_MP3Encoder.cpp"
