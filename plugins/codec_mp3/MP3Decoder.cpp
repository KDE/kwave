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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include <id3/globals.h>
#include <id3/misc_support.h>
#include <id3/tag.h>

#include <QDate>
#include <QDateTime>
#include <QLatin1Char>
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

#include "ID3_QIODeviceReader.h"
#include "MP3CodecPlugin.h"
#include "MP3Decoder.h"

//***************************************************************************
Kwave::MP3Decoder::MP3Decoder()
    :Kwave::Decoder(),
     m_property_map(),
     m_source(Q_NULLPTR),
     m_dest(Q_NULLPTR),
     m_buffer(Q_NULLPTR),
     m_buffer_size(0),
     m_prepended_bytes(0),
     m_appended_bytes(0),
     m_failures(0),
     m_parent_widget(Q_NULLPTR)
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
bool Kwave::MP3Decoder::parseMp3Header(const Mp3_Headerinfo &header,
                                       QWidget *widget)
{
    Kwave::FileInfo info(metaData());

    /* first of all check CRC, it might be senseless if the file is broken */
    qDebug("crc = 0x%08X", header.crc);
    if ((header.crc == MP3CRC_MISMATCH) || (header.crc == MP3CRC_ERROR_SIZE)) {

	if (header.layer == MPEGLAYER_II) {
	    qWarning("WARNING: file is MPEG layer II, CRC calculation "
	             "in id3lib is probably wrong - CRC check skipped");
	} else {
	    if (Kwave::MessageBox::warningContinueCancel(widget,
		i18n("The file has an invalid checksum.\n"
		    "Do you still want to continue?"),
		    QString(), QString(), QString(),
		    _("accept_mp3_invalid_checksum"))
			!= KMessageBox::Continue) return false;
	}
    }

    /* MPEG layer */
    switch (header.layer) {
	case MPEGLAYER_I:
	    info.set(Kwave::INF_COMPRESSION,
	             QVariant(Kwave::Compression::MPEG_LAYER_I));
	    info.set(Kwave::INF_MPEG_LAYER, QVariant(1));
	    break;
	case MPEGLAYER_II:
	    info.set(Kwave::INF_COMPRESSION,
	             QVariant(Kwave::Compression::MPEG_LAYER_II));
	    info.set(Kwave::INF_MPEG_LAYER, QVariant(2));
	    break;
	case MPEGLAYER_III:
	    info.set(Kwave::INF_COMPRESSION,
	             QVariant(Kwave::Compression::MPEG_LAYER_III));
	    info.set(Kwave::INF_MPEG_LAYER, QVariant(3));
	    break;
	default:
	    qWarning("unknown mpeg layer '%d'", header.layer);
    }

    /* MPEG version */
    switch (header.version) {
	case MPEGVERSION_1:
	    info.set(Kwave::INF_MPEG_VERSION, QVariant(1));
	    break;
	case MPEGVERSION_2:
	    info.set(Kwave::INF_MPEG_VERSION, QVariant(2));
	    break;
	case MPEGVERSION_2_5:
	    info.set(Kwave::INF_MPEG_VERSION, QVariant(2.5));
	    break;
	default:
	    qWarning("unknown mpeg version '%d'", header.version);
    }

    /* bit rate */
    if (header.bitrate > 0) info.set(Kwave::INF_BITRATE_NOMINAL,
        QVariant(header.bitrate));
    // NOTE: this is an enum value in libid3, but can also be treated
    // as unsigned integer without problems!

    /* channel mode */
    unsigned int tracks = 0;
    switch (header.channelmode) {
	case MP3CHANNELMODE_SINGLE_CHANNEL:
	     tracks = 1;
	     break;
	case MP3CHANNELMODE_STEREO:
	     tracks = 2;
	     break;
	case MP3CHANNELMODE_JOINT_STEREO:
	     tracks = 2;
	     break;
	case MP3CHANNELMODE_DUAL_CHANNEL:
	     tracks = 2;
	     break;
	default:
	    QString mode;
	    mode = mode.setNum(header.channelmode, 16);
	    if (Kwave::MessageBox::warningContinueCancel(widget,
	        i18n("The file contains an invalid channel mode 0x"
	             "%1\nAssuming Mono...", mode))
	             != KMessageBox::Continue) return false;
    }
    info.setTracks(tracks);

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
    if (header.channelmode == MP3CHANNELMODE_JOINT_STEREO) {
	int modeext = header.modeext;
	if (header.layer >= 3) modeext += 4;
	info.set(Kwave::INF_MPEG_MODEEXT, modeext);
    } else {
	int modeext = header.modeext;
	info.set(Kwave::INF_MPEG_MODEEXT, modeext);
    }

    /* Emphasis mode */
    // 0 = none
    // 1 = 50/15ms
    // 2 = reserved
    // 3 = CCIT J.17
    if (header.emphasis > 0)
        info.set(Kwave::INF_MPEG_EMPHASIS, header.emphasis);

    if (header.privatebit)  info.set(Kwave::INF_PRIVATE, header.privatebit);
    if (header.copyrighted) info.set(Kwave::INF_COPYRIGHTED, header.copyrighted);
    if (header.original)    info.set(Kwave::INF_ORIGINAL, header.original);

    info.setRate(header.frequency); // sample rate
    info.setBits(SAMPLE_BITS);      // fake Kwave's default resolution
    info.set(INF_ESTIMATED_LENGTH, header.time * header.frequency);

    metaData().replace(Kwave::MetaDataList(info));

    return true;
}

//***************************************************************************
bool Kwave::MP3Decoder::parseID3Tags(ID3_Tag &tag)
{
    if (tag.NumFrames() < 1) return true; // no tags, nothing to do

    QDate creation_date;
    QTime creation_time;
    int year  = -1;
    int month = -1;
    int day   = -1;

    ID3_Tag::Iterator *it = tag.CreateIterator();
    ID3_Frame *frame = Q_NULLPTR;
    Kwave::FileInfo info(metaData());
    while (it && (frame = it->GetNext())) {
	const ID3_FrameID id = frame->GetID();
	const Kwave::FileProperty property = m_property_map.property(id);
	const ID3_PropertyMap::Encoding encoding = m_property_map.encoding(id);
	switch (encoding) {
	    case ID3_PropertyMap::ENC_TEXT_PARTINSET:
	    {
		QString s = parseId3Frame2String(frame);
		int cd  = 0;
		int cds = 0;
		if (s.contains(QLatin1Char('/'))) {
		    int i = s.indexOf(QLatin1Char('/'));
		    cd = s.leftRef(i).toInt();
		    cds = s.midRef(i + 1).toInt();
		} else {
		    cd = s.toInt();
		}
		if (cd  > 0) info.set(Kwave::INF_CD , QVariant(cd));
		if (cds > 0) info.set(Kwave::INF_CDS, QVariant(cds));
		break;
	    }
	    case ID3_PropertyMap::ENC_TRACK_NUM:
	    {
		QString s = parseId3Frame2String(frame);
		int track  = 0;
		int tracks = 0;
		if (s.contains(QLatin1Char('/'))) {
		    int i = s.indexOf(QLatin1Char('/'));
		    track = s.leftRef(i).toInt();
		    tracks = s.midRef(i + 1).toInt();
		} else {
		    track = s.toInt();
		}
		if (track  > 0) info.set(Kwave::INF_TRACK , QVariant(track));
		if (tracks > 0) info.set(Kwave::INF_TRACKS, QVariant(tracks));
		break;
	    }
	    case ID3_PropertyMap::ENC_TERMS_OF_USE:
		// the same as ENC_COMMENT, but without "Description"
		/* FALLTHROUGH */
	    case ID3_PropertyMap::ENC_COMMENT:
	    {
		QString s = parseId3Frame2String(frame);

		// optionally prepend language
		char *lang = ID3_GetString(frame, ID3FN_LANGUAGE);
		if (lang) {
		    s = _("[") + _(lang) + _("] ") + s;
		    ID3_FreeString(lang);
		}

		// append to already existing tag, separated by a slash
		if (info.contains(property))
		    s = info.get(property).toString() + _(" / ") + s;
		info.set(property, QVariant(s));
		break;
	    }
	    case ID3_PropertyMap::ENC_GENRE_TYPE:
	    {
		QString s = parseId3Frame2String(frame);
		int genre = Kwave::GenreType::fromID3(s);
		if (genre >= 0)
		    s = Kwave::GenreType::name(genre, false);
		info.set(property, QVariant(s));
		break;
	    }
	    case ID3_PropertyMap::ENC_LENGTH:
	    {
		// length in ms -> convert this to samples
		QString       s    = parseId3Frame2String(frame);
		const double  rate = info.rate();
		bool          ok   = false;
		const double  ms   = s.toDouble(&ok) + 0.5;
		if (ok && (rate > 0)) {
		    // NOTE: this overwrites the length found in the header!
		    sample_index_t length = static_cast<sample_index_t>(
			(rate * ms) / 1000.0);
		    info.setLength(length);
		}
		break;
	    }
	    case ID3_PropertyMap::ENC_TEXT_TIMESTAMP:
	    {
		if (!creation_date.isValid()) {
		    QString s = parseId3Frame2String(frame);
		    switch (id)
		    {
			case ID3FID_RECORDINGDATES:
			    // should be a ISO 8601 timestamp or similar
			    s = Kwave::string2date(s);
			    if (s.length())
				creation_date =
				    QDate::fromString(s, Qt::ISODate);
			    break;
			case ID3FID_DATE: {
			    // DDMM
			    unsigned int ddmm = s.toUInt();
			    day   = ddmm / 100;
			    month = ddmm % 100;
			    break;
			}
			case ID3FID_YEAR: /* FALLTHROUGH */
			case ID3FID_ORIGYEAR:
			    // YYYY
			    year = s.toUInt();
			    break;
			default:
			    break;
		    }
		}

		if (creation_time.isValid()) {
		    switch (id)
		    {
			case ID3FID_TIME:
			    creation_time = QTime::fromString(_("hhmm"));
			    break;
			default:
			    break;
		    }
		}
		break;
	    }
	    case ID3_PropertyMap::ENC_TEXT_SLASH:
	    {
		// append to already existing tag, separated by a slash
		QString s = parseId3Frame2String(frame);
		if (info.contains(property))
		    s = info.get(property).toString() + _(" / ") + s;
		info.set(property, QVariant(s));
		break;
	    }
	    case ID3_PropertyMap::ENC_TEXT_URL:   /* FALLTHROUGH */
	    case ID3_PropertyMap::ENC_TEXT:
		info.set(property, QVariant(parseId3Frame2String(frame)));
		break;
	    case ID3_PropertyMap::ENC_NONE: /* FALLTHROUGH */
	    default:
	    {
		QString s = parseId3Frame2String(frame);
		qWarning("unsupported ID3 tag: %d, descr: '%s', text: '%s'",
			 id, frame->GetDescription(), DBG(s));
		break;
	    }
	}
    }

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
QString Kwave::MP3Decoder::parseId3Frame2String(const ID3_Frame *frame)
{
    QString s;
    char *text = ID3_GetString(frame, ID3FN_TEXT);
    if (text && strlen(text)) {
	s = _(text);
	ID3_FreeString(text);
    }
    return s;
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

    /* read all available ID3 tags */
    ID3_Tag tag;
    ID3_QIODeviceReader adapter(src);
    tag.Link(adapter, static_cast<flags_t>(ID3TT_ALL));

    qDebug("NumFrames = %u", Kwave::toUint(tag.NumFrames()));
    /** @bug: id3lib crashes in this line on some MP3 files */
    if (tag.GetSpec() != ID3V2_UNKNOWN) {
	qDebug("Size = %u",      Kwave::toUint(tag.Size()));
    }
    qDebug("HasLyrics = %d", tag.HasLyrics());
    qDebug("HasV1Tag = %d",  tag.HasV1Tag());
    qDebug("HasV2Tag = %d",  tag.HasV2Tag());

    m_prepended_bytes = tag.GetPrependedBytes();
    m_appended_bytes  = tag.GetAppendedBytes();
    qDebug("prepended=%lu, appended=%lu", m_prepended_bytes, m_appended_bytes);

    const Mp3_Headerinfo *mp3hdr = tag.GetMp3HeaderInfo();
    if (!mp3hdr) {
	Kwave::MessageBox::sorry(widget,
	    i18n("The opened file is no MPEG file or it is damaged.\n"
	    "No header information has been found."));
	return false;
    }

    /* parse the MP3 header */
    if (!parseMp3Header(*mp3hdr, widget)) return false;

    /* parse the ID3 tags */
    if (!parseID3Tags(tag)) return false;

    /* accept the source */
    m_source = &src;
    Kwave::FileInfo info(metaData());
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
enum mad_flow Kwave::MP3Decoder::handleError(void */*data*/,
    struct mad_stream *stream, struct mad_frame */*frame*/)
{
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
    }

    long unsigned int pos = stream->this_frame - m_buffer;
    int result = 0;
    error = i18n("An error occurred while decoding the file:\n'%1',\n"
	         "at position %2.", error, pos);
    if (!m_failures) {
	m_failures = 1;
	result = Kwave::MessageBox::warningContinueCancel(m_parent_widget,
	         error + _("\n") + i18n("Do you still want to continue?"));
	if (result != KMessageBox::Continue) return MAD_FLOW_BREAK;
    } else if (m_failures == 1) {
	result = Kwave::MessageBox::warningYesNo(m_parent_widget,
	    error + _("\n") +
	    i18n("Do you want to continue and ignore all following errors?"));
        m_failures++;
	if (result != KMessageBox::Yes) return MAD_FLOW_BREAK;
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
    if (bytes_to_read) size += m_source->read(
	reinterpret_cast<char *>(m_buffer) + rest, bytes_to_read);
    if (!size) return MAD_FLOW_STOP; // no more data

    // buffer is filled -> process it
    mad_stream_buffer(stream, m_buffer, size);

    return MAD_FLOW_CONTINUE;
}

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

/**
 * 32-bit pseudo-random number generator
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
static inline quint32 prng(quint32 state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

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
enum mad_flow Kwave::MP3Decoder::processOutput(void */*data*/,
    struct mad_header const */*header*/, struct mad_pcm *pcm)
{
    static Kwave::audio_dither dither;
    qint32 sample;
    Kwave::SampleArray buffer(pcm->length);

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
                     Q_NULLPTR /* header */,
                     Q_NULLPTR /* filter */,
                     _output_adapter,
                     _error_adapter,
                     Q_NULLPTR /* message */);

    // decode through libmad...
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    // release the decoder
    mad_decoder_finish(&decoder);

    return (result == 0);
}

//***************************************************************************
void Kwave::MP3Decoder::close()
{
    m_source = Q_NULLPTR;
}

//***************************************************************************
//***************************************************************************
