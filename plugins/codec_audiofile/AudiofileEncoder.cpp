/*************************************************************************
     AudiofileEncoder.h  -  export through libaudiofile
                             -------------------
    begin                : Tue Aug 18 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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

#include <audiofile.h>

#include <limits>
#include <new>

#include <QIODevice>
#include <QVarLengthArray>

#include <KLocalizedString>
#include <QtGlobal>

#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileEncoder.h"

#include <audiofile.h>

/***************************************************************************/
/**
 * look up the maximum support channels of a libaudiofile format via
 * fixed table, as there is no way to query this from the library
 *
 * @param format_id a libaudiofile format ID (e.g., AF_FILE_AIFF, AF_FILE_WAVE)
 * @return maximum number of channels
 */
static unsigned int maxChannelsOf(int format_id)
{
    switch (format_id) {
        case AF_FILE_SAMPLEVISION:
            // SMP header only supports mono (1 channel)
            return 1;

        case AF_FILE_AVR:
        case AF_FILE_IFF_8SVX:
        case AF_FILE_VOC:
            // Header supports only Mono or Stereo (1 or 2 channels)
            return 2;

        case AF_FILE_FLAC:
            // FLAC specification limits channel count to 8
            // (3 bits in STREAMINFO)
            return 8;

        case AF_FILE_AIFF:  /* FALLTHROUGH */
        case AF_FILE_AIFFC: /* FALLTHROUGH */
        case AF_FILE_BICSF: /* FALLTHROUGH */
            // 16-bit signed integer in COMM chunk
            return 32767;

        case AF_FILE_WAVE:
            // 16-bit unsigned integer in fmt chunk (nChannels)
            return 65535;

        case AF_FILE_NEXTSND:     /* FALLTHROUGH */
        case AF_FILE_CAF:         /* FALLTHROUGH */
        case AF_FILE_NIST_SPHERE: /* FALLTHROUGH */
        default:
            // 32-bit container formats
            return ((1ULL << 31ULL) - 1);
    }
}

/***************************************************************************/
Kwave::AudiofileEncoder::AudiofileEncoder(const char *name,
                                          const QString &description,
                                          const char *patterns,
                                          int format_id)
    :Kwave::Encoder(),
     m_format_id(format_id),
     m_misc_chunks()
{
    addMimeType(name, description, patterns);
}

/***************************************************************************/
Kwave::AudiofileEncoder::~AudiofileEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::AudiofileEncoder::instance()
{
    const CodecBase::MimeType m = mimeTypes().constFirst();
    return new(std::nothrow) Kwave::AudiofileEncoder(
        UTF8(m.name),
        m.description,
        UTF8(m.patterns.join(QStringLiteral("; "))),
        m_format_id
    );
}

/***************************************************************************/
QList<Kwave::SampleFormat::Format>
    Kwave::AudiofileEncoder::supportedSampleFormats(const FileInfo &info) const
{
    Q_UNUSED(info);

    switch (m_format_id) {
        /* (not handled here -> own codec)
        case AF_FILE_WAVE:
            // WAV supports 8-bit unsigned, 16/24/32-bit signed Int,
            // plus IEEE Float/Double
            return {
                Kwave::SampleFormat::Unsigned,
                Kwave::SampleFormat::Signed,
                Kwave::SampleFormat::Float,
                Kwave::SampleFormat::Double
            }; */
        case AF_FILE_AIFFC:   /* FALLTHROUGH */
        case AF_FILE_NEXTSND: /* FALLTHROUGH */
        case AF_FILE_BICSF:   /* FALLTHROUGH */
        case AF_FILE_CAF:
            // formats supporting signed int, float and double
            return {
                Kwave::SampleFormat::Signed,
                Kwave::SampleFormat::Float,
                Kwave::SampleFormat::Double
            };
        case AF_FILE_AIFF:     /* FALLTHROUGH */
        case AF_FILE_FLAC:     /* FALLTHROUGH */
        case AF_FILE_IFF_8SVX: /* FALLTHROUGH */
        case AF_FILE_NIST_SPHERE:
            // strict two's complement integer formats
            return {
                Kwave::SampleFormat::Signed
            };
        case AF_FILE_AVR: /* FALLTHROUGH */
        case AF_FILE_VOC: /* FALLTHROUGH */
        case AF_FILE_SAMPLEVISION:
            // legacy formats supporting unsigned/signed Int
            return {
                Kwave::SampleFormat::Unsigned,
                Kwave::SampleFormat::Signed
            };
        default:
            break;
    }
    return {};
}

/***************************************************************************/
QList<unsigned int> Kwave::AudiofileEncoder::supportedBitsPerSample(
    const FileInfo &info) const
{
    const Kwave::SampleFormat::Format format =
        static_cast<Kwave::SampleFormat::Format>(
            info.get(Kwave::INF_SAMPLE_FORMAT).toInt()
        );

    switch (m_format_id) {
        /* (not handled here -> own codec)
        case AF_FILE_WAVE:
            switch (format) {
                case Kwave::SampleFormat::Unsigned: return {8            };
                case Kwave::SampleFormat::Signed:   return {   16, 24, 32};
                case Kwave::SampleFormat::Float:    return {           32};
                default:                            return {8, 16, 24, 32};
            } */
        case AF_FILE_AIFF:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {8, 16, 24, 32};
                case Kwave::SampleFormat::Float:    return {             };
                case Kwave::SampleFormat::Unsigned: return {             };
                default:                            return {8, 16, 24, 32};
            }
        case AF_FILE_AIFFC:   /* FALLTHROUGH */
        case AF_FILE_NEXTSND: /* FALLTHROUGH */
        case AF_FILE_CAF:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {8, 16, 24, 32};
                case Kwave::SampleFormat::Float:    return {           32};
                case Kwave::SampleFormat::Unsigned: return {             };
                default:                            return {8, 16, 24, 32};
            }
        case AF_FILE_FLAC:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {8, 16, 24    };
                default:                            return {             };
            }
        case AF_FILE_BICSF:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {   16, 24, 32};
                case Kwave::SampleFormat::Float:    return {           32};
                default:                            return {   16, 24, 32};
            }

        case AF_FILE_NIST_SPHERE:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {   16, 24, 32};
                default:                            return {             };
        }
        case AF_FILE_IFF_8SVX:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {8            };
                default:                            return {             };
        }
        case AF_FILE_VOC:
            switch (format) {
                case Kwave::SampleFormat::Unsigned: return {8            };
                case Kwave::SampleFormat::Signed:   return {   16        };
                default:                            return {8, 16        };
            }
        case AF_FILE_AVR:
            switch (format) {
                case Kwave::SampleFormat::Unsigned: return {8            };
                case Kwave::SampleFormat::Signed:   return {8, 16        };
                default:                            return {8, 16        };
            }
        case AF_FILE_SAMPLEVISION:
            switch (format) {
                case Kwave::SampleFormat::Signed:   return {   16        };
                default:                            return {             };
            }
        case AF_FILE_RAWDATA:
        default:
            switch (format) {
                case Kwave::SampleFormat::Unsigned: return {8            };
                case Kwave::SampleFormat::Signed:   return {8, 16, 24, 32};
                case Kwave::SampleFormat::Float:    return {           32};
                default:                            return {8, 16, 24, 32};
            }
    }
}

/***************************************************************************/
unsigned long int Kwave::AudiofileEncoder::supportedMarkers() const
{
    if (afQueryLong(AF_QUERYTYPE_MARK, AF_QUERY_SUPPORTED,
                    m_format_id, 0, 0) == 0) return 0;

    long int count = afQueryLong(AF_QUERYTYPE_MARK, AF_QUERY_MAX_NUMBER,
                                 m_format_id, 0, 0);
    return (count < 1) ? 0 : count;
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::AudiofileEncoder::supportedProperties()
{
    switch (m_format_id) {
        /* (not handled here -> own codec)
        case AF_FILE_WAVE:
            return {
                Kwave::INF_NAME,
                Kwave::INF_AUTHOR,
                Kwave::INF_COPYRIGHT,
                Kwave::INF_COMMENTS,
                Kwave::INF_CREATION_DATE,
                Kwave::INF_SOFTWARE
            };
        */
        case AF_FILE_AIFF:
        case AF_FILE_AIFFC:
            return {
                Kwave::INF_NAME,
                Kwave::INF_AUTHOR,
                Kwave::INF_COPYRIGHT,
                Kwave::INF_ANNOTATION,
                Kwave::INF_COMMENTS
            };
        case AF_FILE_CAF:
            // CAF maps chunks internally to key-value pairs in the
            // info chunk
            return {
                Kwave::INF_NAME,
                Kwave::INF_AUTHOR,
                Kwave::INF_COPYRIGHT,
                Kwave::INF_COMMENTS
            };
        case AF_FILE_NEXTSND:
            // next/sun uses a single unstructured info text field
            // in the header
            return { Kwave::INF_ANNOTATION };
        case AF_FILE_NIST_SPHERE:
            // nist uses a text-based header for basic metadata
            return {
                Kwave::INF_NAME,
                Kwave::INF_COMMENTS
            };
        case AF_FILE_BICSF:
        case AF_FILE_AVR:
        case AF_FILE_VOC:
            // these formats do not support text metadata fields
            return {};
        default:
            return {};
    }
}

/***************************************************************************/
void Kwave::AudiofileEncoder::prepareMetaData(
    AFfilesetup &setup,
    const Kwave::MetaDataList &meta_data)
{
    const Kwave::FileInfo info(meta_data);

    /* --- meta data tags --- */

    m_misc_chunks.clear();
    const QList<Kwave::FileProperty> supported = supportedProperties();

    for (Kwave::FileProperty prop : supported) {
        if (!info.contains(prop)) continue;
        QString value = info.get(prop).toString().trimmed();
        if (value.isEmpty()) continue;

        int af_type = AF_MISC_UNRECOGNIZED;
        switch (prop) {
            case Kwave::INF_COPYRIGHT:     af_type = AF_MISC_COPY;    break;
            case Kwave::INF_AUTHOR:        af_type = AF_MISC_AUTH;    break;
            case Kwave::INF_NAME:          af_type = AF_MISC_NAME;    break;
            case Kwave::INF_ANNOTATION:    af_type = AF_MISC_ANNO;    break;
            case Kwave::INF_COMMENTS:      af_type = AF_MISC_COMMENT; break;
            case Kwave::INF_CREATION_DATE: af_type = AF_MISC_ICRD;    break;
            case Kwave::INF_SOFTWARE:      af_type = AF_MISC_ISFT;    break;
            default: break;
        }

        if (af_type != AF_MISC_UNRECOGNIZED) {
            QByteArray utf8_bytes = value.toUtf8();
            if (!utf8_bytes.isEmpty()) {
                utf8_bytes.append('\0');
                m_misc_chunks.append({af_type, utf8_bytes});
            }
        }
    }

    int nids = Kwave::toInt(m_misc_chunks.count());
    if (!nids || !setup) return;

    {
        QVarLengthArray<int, 16> ids(nids);
        for (int i = 0; i < nids; i++)
            ids[i] = i + 1;
        afInitMiscIDs(setup, ids.constData(), nids);
    }

    for (int i = 0; i < nids; ++i) {
        afInitMiscType(setup, i + 1, m_misc_chunks[i].type);
        afInitMiscSize(setup, i + 1, Kwave::toInt(
            m_misc_chunks[i].data.size()));
    }
}

/***************************************************************************/
void Kwave::AudiofileEncoder::prepareMarkers(
    AFfilesetup &setup, const Kwave::LabelList &labels)
{
    if (labels.empty()) return;

    unsigned int supported = Kwave::toUint(supportedMarkers());
    unsigned int used      = Kwave::toUint(labels.count());
    unsigned int count     = qMin(supported, used);
    if (!count) return;

    QVarLengthArray<int, 16> ids(count);
    unsigned int i = 0;
    for (const Label &label : labels) {
        sample_index_t pos = label.pos();
        if (pos > std::numeric_limits<AFframecount>::max())
            continue;
        ids[i] = i + 1;
        i++;
    }
    if (i < used) {
        qWarning("WARNING: %u labels have been skipped", used - i);
    }
    count = i;
    afInitMarkIDs(setup, AF_DEFAULT_TRACK, ids.constData(), count);

    i = 0;
    for (const Label &label : labels) {
        sample_index_t pos = label.pos();
        if (pos > std::numeric_limits<AFframecount>::max())
            continue;
        QString name = label.name();
        afInitMarkName(setup, AF_DEFAULT_TRACK, ids[i],
                       name.toUtf8().constData());
        i++;
    }
}

/***************************************************************************/
void Kwave::AudiofileEncoder::writeMetaData(AFfilehandle fh)
{
    if (m_misc_chunks.isEmpty() || !fh) return;

    int count = afGetMiscIDs(fh, nullptr);
    if (count != m_misc_chunks.size()) return;
    QVarLengthArray<int, 16> ids(count);
    afGetMiscIDs(fh, ids.data());

    for (int i = 0; i < count; ++i) {
        afWriteMisc(fh, ids[i],
                    m_misc_chunks[i].data.constData(),
                    Kwave::toUint(m_misc_chunks[i].data.size()));
    }
}

/***************************************************************************/
void Kwave::AudiofileEncoder::writeMarkers(
    AFfilehandle fh, const Kwave::LabelList &labels)
{
    if (labels.empty()) return;

    unsigned int supported = Kwave::toUint(supportedMarkers());
    unsigned int used      = Kwave::toInt(labels.count());
    unsigned int count     = qMin(supported, used);
    if (!count) return;

    for (unsigned int i = 0; i < count; i++) {
        const Label &label = labels[i];
        AFframecount pos = label.pos();
        afSetMarkPosition(fh, AF_DEFAULT_TRACK, i + 1, pos);
    }
}

/***************************************************************************/
bool Kwave::AudiofileEncoder::encode(QWidget *widget,
                                     Kwave::MultiTrackReader &src,
                                     QIODevice &dst,
                                     const Kwave::MetaDataList &meta_data)
{
    const Kwave::FileInfo  info(meta_data);
    const Kwave::LabelList labels(meta_data);

    int            tracks = info.tracks();
    unsigned int   bits   = info.bits();
    sample_index_t length = info.length();
    double         rate   = info.rate();

    if (bits == 0) bits = 16;
    if (tracks < 1) return false;

    // determine the maximum number of tracks we support
    long int max_tracks = maxChannelsOf(m_format_id);
    if (tracks > max_tracks) {
        if (max_tracks <= 1)
            Kwave::MessageBox::sorry(widget,
                i18n("This file format supports only mono, "
                    "%1 channels are not supported.", tracks));
        else
            Kwave::MessageBox::sorry(widget,
                i18n("This file format supports only up to %1 channels, "
                    "%2 channels are not supported.", max_tracks, tracks));
        return false;
    }

    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        Kwave::MessageBox::error(widget,
            i18n("Unable to open the destination device for writing."));
        return false;
    }

    // determine the sample format
    int af_sample_format = AF_SAMPFMT_TWOSCOMP;
    switch (info.get(Kwave::INF_SAMPLE_FORMAT).toInt()) {
        case Kwave::SampleFormat::Unsigned:
            af_sample_format = AF_SAMPFMT_UNSIGNED;
            break;
        case Kwave::SampleFormat::Float:
            af_sample_format = AF_SAMPFMT_FLOAT;
            break;
        case Kwave::SampleFormat::Double:
            af_sample_format = AF_SAMPFMT_DOUBLE;
            break;
        case Kwave::SampleFormat::Signed:
        default:
            af_sample_format = AF_SAMPFMT_TWOSCOMP;
            break;
    }

    // configure libaudiofile context
    AFfilesetup setup = afNewFileSetup();
    if (!setup) {
        dst.close();
        Kwave::MessageBox::error(widget, i18n("Out of memory"));
        return false;
    }

    afInitFileFormat(setup, m_format_id);
    afInitChannels(setup, AF_DEFAULT_TRACK, tracks);
    afInitSampleFormat(setup, AF_DEFAULT_TRACK,
                       af_sample_format,
                       static_cast<int>(bits));
    afInitRate(setup, AF_DEFAULT_TRACK, rate);

    // prepare metadata chunks and markers
    prepareMetaData(setup, meta_data);
    prepareMarkers(setup,  labels);

    // initialize libaudioFile wrapper
    Kwave::VirtualAudioFile virt_file(dst);
    virt_file.open(&virt_file, setup);
    afFreeFileSetup(setup);

    AFfilehandle fh = virt_file.handle();
    if (!fh) {
        dst.close();
        Kwave::MessageBox::error(widget,
            i18n("Unable to initialize libaudiofile encoder."));
        return false;
    }

    // write metadata payloads
    writeMetaData(fh);

    // set virtual input format to match Kwave's native sample_t format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
                             AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);
    const unsigned int block_len = src.blockSize();

    // target buffer for interleaved frames
    QVarLengthArray<sample_t, 4096> dst_buffer(block_len * tracks);

    // input buffer for each Kwave track
    Kwave::SampleArray buffer(block_len);

    sample_index_t rest   = length;
    bool           result = true;

    const unsigned int shift = (SAMPLE_STORAGE_BITS - SAMPLE_BITS);
    while ((rest > 0) && !src.isCanceled()) {
        const unsigned int len = static_cast<unsigned int>(
            qMin<sample_index_t>(block_len, rest));

        // read from tracks into a local buffer
        for (int track = 0; track < tracks; ++track) {
            Kwave::SampleReader *reader = src[track];
            if (!reader) continue;
            if (buffer.size() != len) buffer.resize(len);

            (*reader) >> buffer;

            const sample_t *b = buffer.constData();
            sample_t       *d = dst_buffer.data() + track;
            for (unsigned int i = 0; i < len; ++i) {
                sample_storage_t s = static_cast<sample_storage_t>(*b);
                if (shift) s <<= shift;
                *d = s;
                d += tracks;
                b += 1;
            }
        }

        // pass interleaved frames to libaudiofile
        int written = afWriteFrames(fh, AF_DEFAULT_TRACK,
                                    dst_buffer.constData(),
                                    static_cast<int>(len));
        if (written <= 0) {
            result = false;
            break;
        }

        rest -= written;
    }

    // write out markers
    writeMarkers(fh, labels);

    virt_file.close();
    dst.close();

    return result && !src.isCanceled();
}

/***************************************************************************/
/***************************************************************************/
