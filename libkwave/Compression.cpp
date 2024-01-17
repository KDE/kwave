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

#include <audiofile.h>

#include "libkwave/Compression.h"

/* static instance */
QMap<int, Kwave::Compression::Info> Kwave::Compression::m_map;

//***************************************************************************
Kwave::Compression::Compression()
    :m_type(Kwave::Compression::NONE)
{
    fillMap();
}

//***************************************************************************
Kwave::Compression::Compression(const Type value)
    :m_type(value)
{
    fillMap();
}

//***************************************************************************
QString Kwave::Compression::name() const
{
    return (m_map.contains(m_type)) ?
	m_map[m_type].m_name.toString() : QString();
}

//***************************************************************************
QString Kwave::Compression::preferredMimeType() const
{
    return (m_map.contains(m_type)) ?
	m_map[m_type].m_mime_type : QString();
}

//***************************************************************************
QList<Kwave::SampleFormat> Kwave::Compression::sampleFormats() const
{
    return (m_map.contains(m_type)) ?
	m_map[m_type].m_sample_formats : QList<Kwave::SampleFormat>();
}

//***************************************************************************
bool Kwave::Compression::hasABR() const
{
    return (m_map.contains(m_type)) ? m_map[m_type].m_has_abr : false;
}

//***************************************************************************
bool Kwave::Compression::hasVBR() const
{
    return (m_map.contains(m_type)) ? m_map[m_type].m_has_vbr : false;
}

//***************************************************************************
Kwave::Compression::Type Kwave::Compression::fromInt(int i)
{
    fillMap();
    return (m_map.contains(static_cast<Kwave::Compression::Type>(i))) ?
	static_cast<Kwave::Compression::Type>(i) : Kwave::Compression::NONE;
}

//***************************************************************************
void Kwave::Compression::fillMap()
{
    if (!m_map.isEmpty()) return; // bail out if already filled

    QList<Kwave::SampleFormat> sfmt_none;

    QList<Kwave::SampleFormat> sfmt_int;
    sfmt_int += Kwave::SampleFormat(Kwave::SampleFormat::Signed);
    sfmt_int += Kwave::SampleFormat(Kwave::SampleFormat::Unsigned);

    QList<Kwave::SampleFormat> sfmt_all;
    sfmt_all += sfmt_int;
    sfmt_all += Kwave::SampleFormat(Kwave::SampleFormat::Float);
    sfmt_all += Kwave::SampleFormat(Kwave::SampleFormat::Double);

    /* no compression */

    m_map.insert(Kwave::Compression::NONE, Kwave::Compression::Info(
	kli18n("No Compression"),
	QString(),
	sfmt_all, false, false));

    /* types supported by OSS+ALSA record plugin and WAV codec */

    m_map.insert(Kwave::Compression::G711_ULAW, Kwave::Compression::Info(
	kli18n("CCITT G.711 u-law"),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::G711_ALAW, Kwave::Compression::Info(
	kli18n("CCITT G.711 A-law"),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::MS_ADPCM, Kwave::Compression::Info(
	kli18n("MS ADPCM"),
	QString(),
	sfmt_int, false, false));
    m_map.insert(Kwave::Compression::GSM, Kwave::Compression::Info(
	kli18n("GSM"),
	QString(),
	sfmt_int, false, false));

    /* compression types from libaudiofile (for display only, not supported) */
    m_map.insert(Kwave::Compression::G722, Kwave::Compression::Info(
	kli18n("G722"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::APPLE_ACE2, Kwave::Compression::Info(
	kli18n("Apple ACE2"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::APPLE_ACE8, Kwave::Compression::Info(
	kli18n("Apple ACE8"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::APPLE_MAC3, Kwave::Compression::Info(
	kli18n("Apple MAC3"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::APPLE_MAC6, Kwave::Compression::Info(
	kli18n("Apple MAC6"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::G726, Kwave::Compression::Info(
	kli18n("G726"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::DVI_AUDIO, Kwave::Compression::Info(
	kli18n("DVI Audio / IMA"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::FS1016, Kwave::Compression::Info(
	kli18n("FS1016"),
	QString(),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::DV, Kwave::Compression::Info(
	kli18n("DV"),
	QString(),
	sfmt_none, true, false));

    /* MPEG layer I/II/III */
#ifdef HAVE_MP3
    m_map.insert(Kwave::Compression::MPEG_LAYER_I, Kwave::Compression::Info(
	kli18n("MPEG Layer I"),
	_("audio/x-mp3"),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::MPEG_LAYER_II, Kwave::Compression::Info(
	kli18n("MPEG Layer II"),
	_("audio/x-mp3"),
	sfmt_none, true, false));
    m_map.insert(Kwave::Compression::MPEG_LAYER_III, Kwave::Compression::Info(
	kli18n("MPEG Layer III"),
	_("audio/x-mp3"),
	sfmt_none, true, false));
#endif /* HAVE_MP3 */

    /* FLAC */
#ifdef HAVE_FLAC
    m_map.insert(Kwave::Compression::FLAC, Kwave::Compression::Info(
	kli18n("FLAC"),
	_("audio/x-flac"),
	sfmt_none, false, false));
#endif /* HAVE_FLAC */

    /* Ogg Vorbis */
#ifdef HAVE_OGG_VORBIS
    m_map.insert(Kwave::Compression::OGG_VORBIS, Kwave::Compression::Info(
	kli18n("Ogg Vorbis"),
	_("audio/ogg"),
	sfmt_none, true, true));
#endif /* HAVE_OGG_VORBIS */

    /* Ogg Opus */
#ifdef HAVE_OGG_OPUS
    m_map.insert(Kwave::Compression::OGG_OPUS, Kwave::Compression::Info(
	kli18n("Ogg Opus"),
	_("audio/opus"),
	sfmt_none, true, false));
#endif /* HAVE_OGG_OPUS */

}

//***************************************************************************
int Kwave::Compression::toAudiofile(Kwave::Compression::Type compression)
{
    int af_compression = AF_COMPRESSION_UNKNOWN;

    fillMap();
    switch (compression)
    {
	case Kwave::Compression::NONE:
	    af_compression = AF_COMPRESSION_NONE;
	    break;
	case Kwave::Compression::G722:
	    af_compression = AF_COMPRESSION_G722;
	    break;
	case Kwave::Compression::G711_ULAW:
	    af_compression = AF_COMPRESSION_G711_ULAW;
	    break;
	case Kwave::Compression::G711_ALAW:
	    af_compression = AF_COMPRESSION_G711_ALAW;
	    break;
	case Kwave::Compression::APPLE_ACE2:
	    af_compression = AF_COMPRESSION_APPLE_ACE2;
	    break;
	case Kwave::Compression::APPLE_ACE8:
	    af_compression = AF_COMPRESSION_APPLE_ACE8;
	    break;
	case Kwave::Compression::APPLE_MAC3:
	    af_compression = AF_COMPRESSION_APPLE_MAC3;
	    break;
	case Kwave::Compression::APPLE_MAC6:
	    af_compression = AF_COMPRESSION_APPLE_MAC6;
	    break;
	case Kwave::Compression::G726:
	    af_compression = AF_COMPRESSION_G726;
	    break;
	case Kwave::Compression::G728:
	    af_compression = AF_COMPRESSION_G728;
	    break;
	case Kwave::Compression::DVI_AUDIO:
	    af_compression = AF_COMPRESSION_DVI_AUDIO;
	    break;
	case Kwave::Compression::GSM:
	    af_compression = AF_COMPRESSION_GSM;
	    break;
	case Kwave::Compression::FS1016:
	    af_compression = AF_COMPRESSION_FS1016;
	    break;
	case Kwave::Compression::DV:
	    af_compression = AF_COMPRESSION_DV;
	    break;
	case Kwave::Compression::MS_ADPCM:
	    af_compression = AF_COMPRESSION_MS_ADPCM;
	    break;
#ifdef HAVE_AF_COMPRESSION_FLAC
	case Kwave::Compression::FLAC:
	    af_compression = AF_COMPRESSION_FLAC;
	    break;
#endif /* HAVE_AF_COMPRESSION_FLAC */
#ifdef HAVE_AF_COMPRESSION_ALAC
	case Kwave::Compression::ALAC:
	    af_compression = AF_COMPRESSION_ALAC;
	    break;
#endif /* HAVE_AF_COMPRESSION_ALAC */
	default:
	    af_compression = AF_COMPRESSION_UNKNOWN;
	    break;
    }

    return af_compression;
}

//***************************************************************************
Kwave::Compression::Type Kwave::Compression::fromAudiofile(int af_compression)
{
    Kwave::Compression::Type compression_type;

    fillMap();
    switch (af_compression)
    {
	case AF_COMPRESSION_NONE :
	    compression_type = Kwave::Compression::NONE;
	    break;
	case AF_COMPRESSION_G722:
	    compression_type = Kwave::Compression::G722;
	    break;
	case AF_COMPRESSION_G711_ULAW:
	    compression_type = Kwave::Compression::G711_ULAW;
	    break;
	case AF_COMPRESSION_G711_ALAW:
	    compression_type = Kwave::Compression::G711_ALAW;
	    break;
	case AF_COMPRESSION_APPLE_ACE2:
	    compression_type = Kwave::Compression::APPLE_ACE2;
	    break;
	case AF_COMPRESSION_APPLE_ACE8:
	    compression_type = Kwave::Compression::APPLE_ACE8;
	    break;
	case AF_COMPRESSION_APPLE_MAC3:
	    compression_type = Kwave::Compression::APPLE_MAC3;
	    break;
	case AF_COMPRESSION_APPLE_MAC6:
	    compression_type = Kwave::Compression::APPLE_MAC6;
	    break;
	case AF_COMPRESSION_G726:
	    compression_type = Kwave::Compression::G726;
	    break;
	case AF_COMPRESSION_G728:
	    compression_type = Kwave::Compression::G728;
	    break;
	case AF_COMPRESSION_DVI_AUDIO:
	    compression_type = Kwave::Compression::DVI_AUDIO;
	    break;
	case AF_COMPRESSION_GSM:
	    compression_type = Kwave::Compression::GSM;
	    break;
	case AF_COMPRESSION_FS1016:
	    compression_type = Kwave::Compression::FS1016;
	    break;
	case AF_COMPRESSION_DV:
	    compression_type = Kwave::Compression::DV;
	    break;
	case AF_COMPRESSION_MS_ADPCM:
	    compression_type = Kwave::Compression::MS_ADPCM;
	    break;
#ifdef HAVE_AF_COMPRESSION_FLAC
	case AF_COMPRESSION_FLAC:
	    compression_type = Kwave::Compression::FLAC;
	    break;
#endif /* HAVE_AF_COMPRESSION_FLAC */
#ifdef HAVE_AF_COMPRESSION_ALAC
	case AF_COMPRESSION_ALAC:
	    compression_type = Kwave::Compression::ALAC;
	    break;
#endif /* HAVE_AF_COMPRESSION_ALAC */
	default:
	    compression_type = Kwave::Compression::NONE;
	    break;
    }

    return compression_type;
}

//***************************************************************************
//***************************************************************************
Kwave::Compression::Info::Info()
    :m_name(),
     m_mime_type(),
     m_sample_formats(),
     m_has_abr(false),
     m_has_vbr(false)
{
}

//***************************************************************************
Kwave::Compression::Info::Info(const Kwave::Compression::Info &other)
    :m_name(other.m_name),
     m_mime_type(other.m_mime_type),
     m_sample_formats(other.m_sample_formats),
     m_has_abr(other.m_has_abr),
     m_has_vbr(other.m_has_vbr)
{
}

//***************************************************************************
Kwave::Compression::Info::~Info()
{
}

//***************************************************************************
Kwave::Compression::Info::Info(
    const KLazyLocalizedString &name,
    const QString &mime_type,
    const QList<Kwave::SampleFormat> &sample_formats,
    bool has_abr,
    bool has_vbr
)
    :m_name(name),
     m_mime_type(mime_type),
     m_sample_formats(sample_formats),
     m_has_abr(has_abr),
     m_has_vbr(has_vbr)
{
}

//***************************************************************************
//***************************************************************************
