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

#include <id3/globals.h>
#include <id3/misc_support.h>
#include <id3/tag.h>

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

#include "ID3_QIODeviceReader.h"
#include "ID3_QIODeviceWriter.h"
#include "MP3CodecPlugin.h"
#include "MP3Encoder.h"
#include "MP3EncoderSettings.h"

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
QList<Kwave::FileProperty> Kwave::MP3Encoder::supportedProperties()
{
    return m_property_map.properties();
}

/***************************************************************************/
void Kwave::MP3Encoder::encodeID3Tags(const Kwave::MetaDataList &meta_data,
                                      ID3_Tag &tag)
{
    const Kwave::FileInfo info(meta_data);
    ID3_FrameInfo frameInfo;

    const QMap<Kwave::FileProperty, QVariant> properties(info.properties());
    QMap<Kwave::FileProperty, QVariant>::const_iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        const Kwave::FileProperty &property = it.key();
        const QVariant            &value    = it.value();

        ID3_FrameID id = m_property_map.findProperty(property);
        if ((property != Kwave::FileProperty::INF_ID3) &&
            (id == ID3FID_NOFRAME)) continue;

        if (info.contains(Kwave::INF_CD) && (property == Kwave::INF_CDS))
            continue; /* INF_CDS has already been handled by INF_CD */
        if (info.contains(Kwave::INF_TRACK) && (property == Kwave::INF_TRACKS))
            continue; /* INF_TRACKS has already been handled by INF_TRACK */

        ID3_Frame *frame = new(std::nothrow) ID3_Frame;
        Q_ASSERT(frame);
        if (!frame) break;

        QString str_val = value.toString();
//      qDebug("encoding ID3 tag #%02d, property='%s', value='%s'",
//          static_cast<int>(id),
//          DBG(info.name(property)),
//          DBG(str_val)
//      );

        // encode in UCS16
        frame->SetID(id);
        ID3_Field *field = frame->GetField(ID3FN_TEXT);
        if ((property != Kwave::FileProperty::INF_ID3) && !field) {
            qWarning("no field, frame id=%d", static_cast<int>(id));
            delete frame;
            continue;
        }

        bool ok = true;
        ID3_PropertyMap::Encoding encoding = m_property_map.encoding(id);
        switch (encoding) {
            case ID3_PropertyMap::ENC_TEXT_PARTINSET:
            {
                field->SetEncoding(ID3TE_UTF16);

                // if "number of CDs is available: append with "/"
                int cds = info.get(Kwave::INF_CDS).toInt();
                if (cds > 0)
                    str_val += _("/%1").arg(cds);

                field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                break;
            }
            case ID3_PropertyMap::ENC_TRACK_NUM:
            {
                // if "number of tracks is available: append with "/"
                int tracks = info.get(Kwave::INF_TRACKS).toInt();
                if (tracks > 0)
                    str_val += _("/%1").arg(tracks);

                field->SetEncoding(ID3TE_UTF16);
                field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                break;
            }
            case ID3_PropertyMap::ENC_TERMS_OF_USE:
                // the same as ENC_COMMENT, but without "Description"
                /* FALLTHROUGH */
            case ID3_PropertyMap::ENC_COMMENT:
            {
                // detect language at the start "[xxx] "
                if (str_val.startsWith(QLatin1Char('[')) &&
                    (str_val.at(4) == QLatin1Char(']'))) {
                    QString lang = str_val.mid(1,3);
                    str_val      = str_val.mid(5);
                    frame->GetField(ID3FN_DESCRIPTION)->Set("");
                    frame->GetField(ID3FN_LANGUAGE)->Set(
                        static_cast<const char *>(lang.toLatin1().data()));
                }
                /* frame->GetField(ID3FN_DESCRIPTION)->Set(""); */
                field->SetEncoding(ID3TE_UTF16);
                field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                break;
            }
            case ID3_PropertyMap::ENC_GENRE_TYPE:
            {
                int genre = Kwave::GenreType::fromID3(str_val);
                if (genre >= 0)
                    str_val = Kwave::GenreType::name(genre, false);
                // else: user defined genre type, take it as it is

                field->SetEncoding(ID3TE_UTF16);
                field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                break;
            }
            case ID3_PropertyMap::ENC_LENGTH:
            {
                // length in milliseconds
                const double         rate    = info.rate();
                const sample_index_t samples = info.length();
                if ((rate > 0) && samples) {
                    const sample_index_t ms = static_cast<sample_index_t>(
                        (static_cast<double>(samples) * 1E3) / rate);

                    str_val = QString::number(ms);

                    field->SetEncoding(ID3TE_UTF16);
                    field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                } else
                    ok = false;
                break;
            }
            case ID3_PropertyMap::ENC_TEXT_TIMESTAMP:
            {
                // ISO 8601 timestamp: "yyyy-MM-ddTHH:mm:ss"
                QString s = Kwave::string2date(str_val);

                // if failed, try "yyyy" format (year only)
                if (!s.length()) {
                    int year = str_val.toInt();
                    if ((year > 0) && (year < 9999)) {
                        frame->SetID(ID3FID_YEAR);
                        // -> re-get the field !
                        // it has become invalid through "SetID()"
                        field = frame->GetField(ID3FN_TEXT);
                        if (!field) {
                            qWarning("no field, frame id=%d",
                                     static_cast<int>(id));
                            ok = false;
                            break;
                        }
                        s = _("%1").arg(year, 4, 10, QLatin1Char('0'));
                    }
                }

                if (s.length()) {
                    field->SetEncoding(ID3TE_UTF16);
                    field->Set(static_cast<const unicode_t *>(s.utf16()));
                } else {
                    // date is invalid, unknown format
                    qWarning("MP3Encoder::encodeID3Tags(): invalid date: '%s'",
                             DBG(str_val));
                    ok = false;
                }
                break;
            }
            case ID3_PropertyMap::ENC_TEXT_SLASH: /* FALLTHROUGH */
            case ID3_PropertyMap::ENC_TEXT_URL:   /* FALLTHROUGH */
            case ID3_PropertyMap::ENC_TEXT:
                field->SetEncoding(ID3TE_UTF16);
                field->Set(static_cast<const unicode_t *>(str_val.utf16()));
                break;
            case ID3_PropertyMap::ENC_BINARY:
            {
                QStringList frames = value.toStringList();
                foreach (const QString &f, frames) {
                    delete frame;
                    frame = new(std::nothrow) ID3_Frame;
                    Q_ASSERT(frame != nullptr);
                    if (frame == nullptr) continue;

                    QByteArray raw = QByteArray::fromBase64(f.toLocal8Bit());
                    QBuffer buffer(&raw);
                    buffer.open(QIODevice::ReadOnly);
                    Kwave::ID3_QIODeviceReader reader(buffer);
                    if (!frame->Parse(reader)) {
                        qWarning("MP3Encoder::encodeID3Tags(): "
                        "parsing ENC_BINARY failed");
                        ok = false;
                        continue;
                    }
                    qDebug("attaching custom ID3 frame '%s'",
                           frame->GetDescription());
                    if (tag.AttachFrame(frame)) {
                        frame = nullptr;
                        ok = true;
                    } else {
                        qWarning("MP3Encoder::encodeID3Tags(): "
                        "attaching ENC_BINARY frame failed");
                        ok = false;
                        continue;
                    }
                }
                break;
            }
            case ID3_PropertyMap::ENC_NONE: /* FALLTHROUGH */
            default:
                ok = false;
                break; // ignore
        }

        if (frame && ok) tag.AttachFrame(frame);
        if (!ok) delete frame;
        frame = nullptr;
    }

    tag.Strip();
    tag.Update();
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
    ID3_Tag id3_tag;
    Kwave::MP3EncoderSettings settings;

    settings.load();

    ID3_TagType id3_tag_type = ID3TT_ID3V2;
    id3_tag.SetSpec(ID3V2_LATEST);

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

    // encode meta data into with id3lib
    ID3_QIODeviceWriter id3_writer(dst);
    encodeID3Tags(meta_data, id3_tag);

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

    // if a ID3v2 tag is requested, the tag comes at the start
    if (id3_tag_type == ID3TT_ID3V2)
        id3_tag.Render(id3_writer, id3_tag_type);

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

    // if a ID3v1 tag is requested, the tag comes at the end
    if (id3_tag_type != ID3TT_ID3V2)
        id3_tag.Render(id3_writer, id3_tag_type);

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
