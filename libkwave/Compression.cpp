/***************************************************************************
        Compression.cpp  -  Wrapper for a compression
                             -------------------
    begin                : Fri Jan 25 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#include <new>
#include <stdlib.h>

#include "libkwave/Compression.h"

/* static instance */
QMap<int, Kwave::Compression> Kwave::Compression::m_map;

//***************************************************************************
Kwave::Compression::Compression()
    :m_data()
{
}

//***************************************************************************
Kwave::Compression::Compression(int value)
    :m_data()
{
    if (m_map.isEmpty()) fillMap();

    if (m_map.contains(value)) {
	Kwave::Compression known_comp(m_map[value]);
	m_data = known_comp.m_data;
    }
}

//***************************************************************************
Kwave::Compression::Compression(const Kwave::Compression &other)
    :m_data(other.m_data)
{
}

//***************************************************************************
Kwave::Compression::Compression(
    int value,
    const QString &name,
    const QString &mime_type,
    const QList<Kwave::SampleFormat> &sample_formats,
    bool has_abr,
    bool has_vbr
)
    :m_data()
{
    if (m_map.contains(value)) return; // ignore duplicates

    m_data = new(std::nothrow) Kwave::Compression::CompressionInfo;
    Q_ASSERT(m_data);
    if (!m_data) return;

    m_data->m_as_int         = value;
    m_data->m_name           = name;
    m_data->m_mime_type      = mime_type;
    m_data->m_sample_formats = sample_formats;
    m_data->m_has_abr        = has_abr;
    m_data->m_has_vbr        = has_vbr;
}

//***************************************************************************
Kwave::Compression::~Compression()
{
}

//***************************************************************************
QString Kwave::Compression::name() const
{
    return (m_data) ? i18n(UTF8(m_data->m_name)) : QString();
}

//***************************************************************************
QString Kwave::Compression::preferredMimeType() const
{
    return (m_data) ? m_data->m_mime_type : QString();
}

//***************************************************************************
QList<Kwave::SampleFormat> Kwave::Compression::sampleFormats() const
{
    return (m_data) ? m_data->m_sample_formats : QList<Kwave::SampleFormat>();
}

//***************************************************************************
bool Kwave::Compression::hasABR() const
{
    return (m_data) ? m_data->m_has_abr : false;
}

//***************************************************************************
bool Kwave::Compression::hasVBR() const
{
    return (m_data) ? m_data->m_has_vbr : false;
}

//***************************************************************************
int Kwave::Compression::toInt() const
{
    return (m_data) ? m_data->m_as_int : -1;
}

//***************************************************************************
void Kwave::Compression::fillMap()
{
    QList<Kwave::SampleFormat> sfmt_none;

    QList<Kwave::SampleFormat> sfmt_int;
    sfmt_int += Kwave::SampleFormat(Kwave::SampleFormat::Signed);
    sfmt_int += Kwave::SampleFormat(Kwave::SampleFormat::Unsigned);

    QList<Kwave::SampleFormat> sfmt_all;
    sfmt_all += sfmt_int;
    sfmt_all += Kwave::SampleFormat(Kwave::SampleFormat::Float);
    sfmt_all += Kwave::SampleFormat(Kwave::SampleFormat::Double);

    /* no compression */

    m_map.insert(Kwave::Compression::NONE, Kwave::Compression(
	Kwave::Compression::NONE,
	_(I18N_NOOP("No Compression")),
	QString(),
	sfmt_all, false, false));

    /* types supported by OSS+ALSA record plugin and WAV codec */

    m_map.insert(Kwave::Compression::G711_ULAW, Kwave::Compression(
	Kwave::Compression::G711_ULAW,
	_(I18N_NOOP("CCITT G.711 u-law")),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::G711_ALAW, Kwave::Compression(
	Kwave::Compression::G711_ALAW,
	_(I18N_NOOP("CCITT G.711 A-law")),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::MS_ADPCM, Kwave::Compression(
	Kwave::Compression::MS_ADPCM,
	_(I18N_NOOP("MS ADPCM")),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::GSM, Kwave::Compression(
	Kwave::Compression::GSM,
	_(I18N_NOOP("GSM")),
	QString(),
	sfmt_int, false, false));

    /* compression types from libaudiofile (for display only, not supported) */

    static const struct {
	int compression;
	const char *name;
    } af_list[] = {
	{ AF_COMPRESSION_G722,       I18N_NOOP("G722")            },
	{ AF_COMPRESSION_APPLE_ACE2, I18N_NOOP("Apple ACE2")      },
	{ AF_COMPRESSION_APPLE_ACE8, I18N_NOOP("Apple ACE8")      },
	{ AF_COMPRESSION_APPLE_MAC3, I18N_NOOP("Apple MAC3")      },
	{ AF_COMPRESSION_APPLE_MAC6, I18N_NOOP("Apple MAC6")      },
	{ AF_COMPRESSION_G726,       I18N_NOOP("G726")            },
	{ AF_COMPRESSION_G728,       I18N_NOOP("G728")            },
	{ AF_COMPRESSION_DVI_AUDIO,  I18N_NOOP("DVI Audio / IMA") },
	{ AF_COMPRESSION_FS1016,     I18N_NOOP("FS1016")          },
	{ AF_COMPRESSION_DV,         I18N_NOOP("DV")              }
    };
    unsigned int af_count = sizeof(af_list) / sizeof(af_list[0]);
    for (unsigned int index = 0; index < af_count; ++index) {
	int id = af_list[index].compression;
	if (m_map.contains(id)) continue;
	QString name = QString::fromAscii(af_list[index].name);
	m_map.insert(id, Kwave::Compression(
	    id, name, QString(), sfmt_all, false, false));
    }

    /* MPEG layer I/II/III */
#ifdef HAVE_MP3
    m_map.insert(Kwave::Compression::MPEG_LAYER_I, Kwave::Compression(
	Kwave::Compression::MPEG_LAYER_I,
	_(I18N_NOOP("MPEG Layer I")),
	_("audio/x-mp3"),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::MPEG_LAYER_II, Kwave::Compression(
	Kwave::Compression::MPEG_LAYER_II,
	_(I18N_NOOP("MPEG Layer II")),
	_("audio/x-mp3"),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::MPEG_LAYER_III, Kwave::Compression(
	Kwave::Compression::MPEG_LAYER_III,
	_(I18N_NOOP("MPEG Layer III")),
	_("audio/x-mp3"),
	sfmt_none, true, false));
#endif /* HAVE_MP3 */

    /* FLAC */
#ifdef HAVE_FLAC
    m_map.insert(Kwave::Compression::FLAC, Kwave::Compression(
	Kwave::Compression::FLAC,
	_(I18N_NOOP("FLAC")),
	_("audio/x-flac"),
	sfmt_none, false, false));
#endif /* HAVE_FLAC */

    /* Ogg Vorbis */
#ifdef HAVE_OGG_VORBIS
    m_map.insert(Kwave::Compression::OGG_VORBIS, Kwave::Compression(
	Kwave::Compression::OGG_VORBIS,
	_(I18N_NOOP("Ogg Vorbis")),
	_("audio/ogg"),
	sfmt_none, true, true));
#endif /* HAVE_OGG_VORBIS */

    /* Ogg Opus */
#ifdef HAVE_OGG_OPUS
    m_map.insert(Kwave::Compression::OGG_OPUS, Kwave::Compression(
	Kwave::Compression::OGG_OPUS,
	_(I18N_NOOP("Ogg Opus")),
	_("audio/opus"),
	sfmt_none, true, false));
#endif /* HAVE_OGG_OPUS */

}

//***************************************************************************
//***************************************************************************
Kwave::Compression::CompressionInfo::CompressionInfo()
    :QSharedData(),
     m_as_int(-1),
     m_name(),
     m_has_abr(false),
     m_has_vbr(false)
{
}

//***************************************************************************
Kwave::Compression::CompressionInfo::CompressionInfo(
    const CompressionInfo &other
)
    :QSharedData(other),
     m_as_int(other.m_as_int),
     m_name(other.m_name),
     m_has_abr(other.m_has_abr),
     m_has_vbr(other.m_has_vbr)
{
}

//***************************************************************************
Kwave::Compression::CompressionInfo::~CompressionInfo()
{
}

//***************************************************************************
//***************************************************************************
