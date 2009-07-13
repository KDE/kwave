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

#include <id3/globals.h>
#include <id3/tag.h>
#include <id3/misc_support.h>

#include "libkwave/CompressionType.h"
#include "libkwave/ConfirmCancelProxy.h"
#include "libkwave/GenreType.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "MP3Decoder.h"
#include "ID3_QIODeviceReader.h"

//***************************************************************************
MP3Decoder::MP3Decoder()
    :Decoder(), m_source(0), m_dest(0), m_buffer(0), m_buffer_size(0),
     m_prepended_bytes(0), m_appended_bytes(0), m_failures(0),
     m_parent_widget(0)
{
    /* included in KDE: */
    addMimeType("audio/x-mpga",   i18n("MPEG layer1 audio"),
                "*.mpga *.mpg *.mp1");
    addMimeType("audio/x-mp2",    i18n("MPEG layer2 audio"), "*.mp2");
    addMimeType("audio/x-mp3",    i18n("MPEG layer3 audio"), "*.mp3");

    /* like defined in RFC3003 */
    addMimeType("audio/mpeg",     i18n("MPEG audio"), "*.mpga *.mpg *.mp1");
    addMimeType("audio/mpeg",     i18n("MPEG layer2 audio"), "*.mp2");
    addMimeType("audio/mpeg",     i18n("MPEG layer3 audio"), "*.mp3");

    // NOTE: all mime types above should be recognized in the
    //       fileinfo plugin!

}

//***************************************************************************
MP3Decoder::~MP3Decoder()
{
    if (m_source) close();
}

//***************************************************************************
Decoder *MP3Decoder::instance()
{
    return new MP3Decoder();
}

//***************************************************************************
bool MP3Decoder::parseMp3Header(const Mp3_Headerinfo &header, QWidget *widget)
{
    /* first of all check CRC, it might be senseless if the file is broken */
    qDebug("crc = 0x%08X", header.crc);
    if ((header.crc == MP3CRC_MISMATCH) || (header.crc == MP3CRC_ERROR_SIZE)){
	if (Kwave::MessageBox::warningContinueCancel(widget,
	    i18n("The file has an invalid checksum.\n"
	         "Do you still want to continue?"))
	             != KMessageBox::Continue) return false;
    }

    /* MPEG layer */
    switch (header.layer) {
	case MPEGLAYER_I:
	    m_info.set(INF_COMPRESSION,
	               QVariant(CompressionType::MPEG_LAYER_I));
	    m_info.set(INF_MPEG_LAYER, QVariant(1));
	    break;
	case MPEGLAYER_II:
	    m_info.set(INF_COMPRESSION,
	               QVariant(CompressionType::MPEG_LAYER_II));
	    m_info.set(INF_MPEG_LAYER, QVariant(2));
	    break;
	case MPEGLAYER_III:
	    m_info.set(INF_COMPRESSION,
	               QVariant(CompressionType::MPEG_LAYER_III));
	    m_info.set(INF_MPEG_LAYER, QVariant(3));
	    break;
	default:
	    qWarning("unknown mpeg layer '%d'", header.layer);
    }

    /* MPEG version */
    switch (header.version) {
	case MPEGVERSION_1:
	    m_info.set(INF_MPEG_VERSION, QVariant(1));
	    break;
	case MPEGVERSION_2:
	    m_info.set(INF_MPEG_VERSION, QVariant(2));
	    break;
	case MPEGVERSION_2_5:
	    m_info.set(INF_MPEG_VERSION, QVariant(2.5));
	    break;
	default:
	    qWarning("unknown mpeg version '%d'", header.version);
    }

    /* bit rate */
    if (header.bitrate > 0) m_info.set(INF_BITRATE_NOMINAL,
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
    m_info.setTracks(tracks);

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
	m_info.set(INF_MPEG_MODEEXT, modeext);
    }

    /* Emphasis mode */
    // 0 = none
    // 1 = 50/15ms
    // 2 = reserved
    // 3 = CCIT J.17
    if (header.emphasis > 0)
        m_info.set(INF_MPEG_EMPHASIS, header.emphasis);

//  qDebug("framesize=%d", header.framesize);
//  qDebug("frames = %u", header.frames);

    if (header.privatebit)  m_info.set(INF_PRIVATE, header.privatebit);
    if (header.copyrighted) m_info.set(INF_COPYRIGHTED, header.copyrighted);
    if (header.original)    m_info.set(INF_ORIGINAL, header.original);

    m_info.setRate(header.frequency); // sample rate
    m_info.setBits(SAMPLE_BITS);      // fake Kwave's default resolution
    m_info.setLength(header.time * header.frequency);

    return true;
}

//***************************************************************************
bool MP3Decoder::parseID3Tags(ID3_Tag &tag)
{
    if (tag.NumFrames() < 1) return true; // no tags, nothing to do

    ID3_Tag::Iterator *it = tag.CreateIterator();
    ID3_Frame *frame;
    while (it && (frame = it->GetNext())) {
	ID3_FrameID id = frame->GetID();
        switch (id) {
	    //ID3FID_AUDIOCRYPTO       // Audio encryption.
	    //ID3FID_PICTURE           // Attached picture.
	    //ID3FID_AUDIOSEEKPOINT    // Audio seek point index.

	    case ID3FID_COMMENT: // Comments.
		parseId3Frame(frame, INF_COMMENTS); break;

	    //ID3FID_COMMERCIAL        // Commercial frame.
	    //ID3FID_CRYPTOREG         // Encryption method registration.
	    //ID3FID_EQUALIZATION2     // Equalisation (2).
	    //ID3FID_EQUALIZATION      // Equalization.
	    //ID3FID_EVENTTIMING       // Event timing codes.
	    //ID3FID_GENERALOBJECT     // General encapsulated object.
	    //ID3FID_GROUPINGREG       // Group identification registration.
	    //ID3FID_INVOLVEDPEOPLE    // Involved people list.
	    //ID3FID_LINKEDINFO        // Linked information.
	    //ID3FID_CDID              // Music CD identifier.
	    //ID3FID_MPEGLOOKUP        // MPEG location lookup table.

	    case ID3FID_OWNERSHIP:     // Ownership frame.
		parseId3Frame(frame, INF_CONTACT); break;

	    //ID3FID_PRIVATE           // Private frame.
	    //ID3FID_PLAYCOUNTER       // Play counter.
	    //ID3FID_POPULARIMETER     // Popularimeter.
	    //ID3FID_POSITIONSYNC      // Position synchronisation frame.
	    //ID3FID_BUFFERSIZE        // Recommended buffer size.
	    //ID3FID_VOLUMEADJ2        // Relative volume adjustment (2).
	    //ID3FID_VOLUMEADJ         // Relative volume adjustment.
	    //ID3FID_REVERB            // Reverb.
	    //ID3FID_SEEKFRAME         // Seek frame.
	    //ID3FID_SIGNATURE         // Signature frame.
	    //ID3FID_SYNCEDLYRICS      // Synchronized lyric/text.
	    //ID3FID_SYNCEDTEMPO       // Synchronized tempo codes.

	    case ID3FID_ALBUM:         // Album/Movie/Show title.
		parseId3Frame(frame, INF_ALBUM); break;

	    //ID3FID_BPM               // BPM (beats per minute).
	    //ID3FID_COMPOSER          // Composer.

	    case ID3FID_CONTENTTYPE: { // Content type (Genre)
		parseId3Frame(frame, INF_GENRE);
		QString genre = QVariant(m_info.get(INF_GENRE)).toString();
		int id = GenreType::fromID3(genre);
		m_info.set(INF_GENRE, GenreType::name(id));
		break;
	    }
	    case ID3FID_COPYRIGHT:     // Copyright message.
		parseId3Frame(frame, INF_COPYRIGHT); break;

	    //ID3FID_DATE              // Date.
	    //ID3FID_ENCODINGTIME      // Encoding time.
	    //ID3FID_PLAYLISTDELAY     // Playlist delay.
	    //ID3FID_ORIGRELEASETIME   // Original release time.
	    //ID3FID_RECORDINGTIME     // Recording time.
	    //ID3FID_RELEASETIME       // Release time.
	    //ID3FID_TAGGINGTIME       // Tagging time.
	    //ID3FID_INVOLVEDPEOPLE2   // Involved people list.
	    //ID3FID_ENCODEDBY         //

	    case ID3FID_ENCODEDBY:     // Encoded by.
		parseId3Frame(frame, INF_TECHNICAN); break;

	    //ID3FID_FILETYPE          // File type.
	    //ID3FID_TIME              // Time.
	    //ID3FID_CONTENTGROUP      // Content group description.

	    case ID3FID_TITLE:        // Title/songname/content description.
		parseId3Frame(frame, INF_NAME); break;

	    case ID3FID_SUBTITLE:     // Subtitle/Description refinement.
		parseId3Frame(frame, INF_ANNOTATION); break;

	    //ID3FID_INITIALKEY       // Initial key.
	    //ID3FID_LANGUAGE         // Language(s).
	    //ID3FID_SONGLEN          // Length.
	    //ID3FID_MUSICIANCREDITLIST // Musician credits list.

	    case ID3FID_MEDIATYPE:    // Medium type.
		parseId3Frame(frame, INF_MEDIUM); break;

	    //ID3FID_MOOD             // Mood.
	    //ID3FID_ORIGALBUM        // Original album/movie/show title.
	    //ID3FID_ORIGFILENAME     // Original filename.

	    case ID3FID_LYRICIST:     // Lyricist/Text writer.
	    case ID3FID_ORIGLYRICIST: // Original lyricist(s)/text writer(s).
		parseId3Frame(frame, INF_PERFORMER); break;

	    case ID3FID_ORIGARTIST:   // Original artist(s)/performer(s).
		parseId3Frame(frame, INF_AUTHOR); break;

	    //ID3FID_ORIGYEAR         // Original release year.
	    case ID3FID_FILEOWNER:    // File owner/licensee.
		parseId3Frame(frame, INF_LICENSE); break;
	    case ID3FID_LEADARTIST:   // Lead performer(s)/Soloist(s).
		parseId3Frame(frame, INF_PERFORMER); break;

	    //ID3FID_BAND             // Band/orchestra/accompaniment.
	    //ID3FID_CONDUCTOR        // Conductor/performer refinement.
	    case ID3FID_MIXARTIST:    // Interpreted, remixed, or otherwise
	                              // modified by
		parseId3Frame(frame, INF_VERSION); break;
	    case ID3FID_PARTINSET:    // Part of a set.
		parseId3Frame(frame, INF_CD); break;

	    case ID3FID_PRODUCEDNOTICE:// Produced notice.
		parseId3Frame(frame, INF_ORGANIZATION); break;
	    case ID3FID_PUBLISHER:    // Publisher.
		parseId3Frame(frame, INF_ORGANIZATION); break;
	    case ID3FID_TRACKNUM:     // Track number/Position in set.
		parseId3Frame(frame, INF_TRACK); break;

	    //ID3FID_RECORDINGDATES   // Recording dates.
	    //ID3FID_NETRADIOSTATION  // Internet radio station name.
	    //ID3FID_NETRADIOOWNER    // Internet radio station owner.
	    //ID3FID_SIZE             // Size.
	    //ID3FID_ALBUMSORTORDER   // Album sort order.
	    //ID3FID_PERFORMERSORTORDER// Performer sort order.
	    //ID3FID_TITLESORTORDER   // Title sort order.

	    case ID3FID_ISRC:         // ISRC (international standard
	                              // recording code).
		parseId3Frame(frame, INF_ISRC); break;

	    //ID3FID_ENCODERSETTINGS  // Software/Hardware and settings
	                              // used for encoding.

	    case ID3FID_SETSUBTITLE:  // Set subtitle.
		parseId3Frame(frame, INF_VERSION); break;
	    case ID3FID_USERTEXT:     // User defined text information.
		parseId3Frame(frame, INF_ANNOTATION); break;

	    //ID3FID_YEAR             // Year.
	    //ID3FID_UNIQUEFILEID     // Unique file identifier.
	    //ID3FID_TERMSOFUSE       // Terms of use.
	    //ID3FID_UNSYNCEDLYRICS   // Unsynchronized lyric/text
	                              // transcription
	    //ID3FID_WWWCOMMERCIALINFO// Commercial information.
	    //ID3FID_WWWCOPYRIGHT     // Copyright/Legal information.
	    //ID3FID_WWWAUDIOFILE     // Official audio file webpage.
	    //ID3FID_WWWARTIST        // Official artist/performer webpage.
	    //ID3FID_WWWAUDIOSOURCE   // Official audio source webpage.
	    //ID3FID_WWWRADIOPAGE     // Official internet radio station
	                              // homepage.
	    //ID3FID_WWWPAYMENT       // Payment.
	    //ID3FID_WWWPUBLISHER     // Official publisher webpage.
	    //ID3FID_WWWUSER          // User defined URL link.
	    //ID3FID_METACRYPTO       // Encrypted meta frame (id3v2.2.x).
	    //ID3FID_METACOMPRESSION  // Compressed meta frame (id3v2.2.1).

	    default:
		char *text = ID3_GetString(frame, ID3FN_TEXT);
		qDebug("frame with id=%d, descr=%s, text=%s",
		      id, frame->GetDescription(), text);
		if (text) delete [] text;
	}

    }

    return true;
}

//***************************************************************************
void MP3Decoder::parseId3Frame(ID3_Frame *frame, FileProperty property)
{
    if (!frame) return;
    char *text = ID3_GetString(frame, ID3FN_TEXT);
    if (text) {
	m_info.set(property, QVariant(text));
	delete [] text;
    }
}

//***************************************************************************
bool MP3Decoder::open(QWidget *widget, QIODevice &src)
{
    qDebug("MP3Decoder::open()");
    info().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("MP3Decoder::open(), already open !");

    /* open the file in readonly mode with seek enabled */
    Q_ASSERT(!src.isSequential());
    if (src.isSequential()) return false;
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("unable to open source in read-only mode!");
	return false;
    }

    /* read all available ID3 tags */
    ID3_Tag tag;
    ID3_QIODeviceReader adapter(src);
    tag.Link(adapter, static_cast<flags_t>(ID3TT_ALL));

    qDebug("NumFrames = %d", static_cast<int>(tag.NumFrames()));
    /** @bug: id3lib crashes in this line on some MP3 files */
    if (tag.GetSpec() != ID3V2_UNKNOWN) {
	qDebug("Size = %d",      static_cast<int>(tag.Size()));
    }
    qDebug("HasLyrics = %d", tag.HasLyrics());
    qDebug("HasV1Tag = %d",  tag.HasV1Tag());
    qDebug("HasV2Tag = %d",  tag.HasV2Tag());

    m_prepended_bytes = tag.GetPrependedBytes();
    m_appended_bytes  = tag.GetAppendedBytes();
    qDebug("prepended=%u, appended=%u",m_prepended_bytes, m_appended_bytes);

    const Mp3_Headerinfo *mp3hdr = tag.GetMp3HeaderInfo();
    if (!mp3hdr) {
	Kwave::MessageBox::sorry(widget,
	    i18n("The opened file is no MPEG file or is damaged.\n"
	    "No header information has been found..."));
	return false;
    }

    /* parse the MP3 header */
    if (!parseMp3Header(*mp3hdr, widget)) return false;

    /* parse the ID3 tags */
    if (!parseID3Tags(tag)) return false;

    /* accept the source */
    m_source = &src;
    m_info.set(INF_MIMETYPE, "audio/mpeg");


    // allocate a transfer buffer with 128 kB
    if (m_buffer) delete m_buffer;
    m_buffer_size = (128 << 10);

    m_buffer = static_cast<unsigned char *>(malloc(m_buffer_size));
    if (!m_buffer) return false; // out of memory :-(

    return true;
}

//***************************************************************************
static enum mad_flow _input_adapter(void *data, struct mad_stream *stream)
{
    MP3Decoder *decoder = reinterpret_cast<MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ? decoder->fillInput(stream) : MAD_FLOW_STOP;
}

//***************************************************************************
static enum mad_flow _output_adapter(void *data,
                                     struct mad_header const *header,
                                     struct mad_pcm *pcm)
{
    MP3Decoder *decoder = reinterpret_cast<MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ?
        decoder->processOutput(data, header, pcm) : MAD_FLOW_STOP;
}

//***************************************************************************
static enum mad_flow _error_adapter(void *data, struct mad_stream *stream,
                                    struct mad_frame *frame)
{
    MP3Decoder *decoder = reinterpret_cast<MP3Decoder *>(data);
    Q_ASSERT(decoder);
    return (decoder) ?
        decoder->handleError(data, stream, frame) : MAD_FLOW_BREAK;
}

//***************************************************************************
enum mad_flow MP3Decoder::handleError(void */*data*/,
    struct mad_stream *stream, struct mad_frame */*frame*/)
{
    if (m_failures >= 2) return MAD_FLOW_CONTINUE; // ignore errors
    if (stream->error == MAD_ERROR_NONE) return MAD_FLOW_CONTINUE; // ???

    QString error;
    switch (stream->error) {
	case MAD_ERROR_BUFLEN:
	case MAD_ERROR_BUFPTR:
	case MAD_ERROR_NOMEM:
		error = i18n("out of memory");
		break;
	case MAD_ERROR_BADCRC:
		error = i18n("checksum error");
		break;
	case MAD_ERROR_LOSTSYNC:
		error = i18n("synchronization lost");
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
		error = i18n("file contains invalid data");
		break;
	default:
		error = i18n("unknown error 0x%X. damaged file?",
		static_cast<int>(stream->error));
    }

    unsigned int pos = stream->this_frame - m_buffer;
    int result = 0;
    error = i18n("An error occurred while decoding the file:\n'%1',\n"
	         "at position %2.", error, pos);
    if (!m_failures) {
	m_failures = 1;
	result = Kwave::MessageBox::warningContinueCancel(m_parent_widget,
	         error + "\n" + i18n("Do you still want to continue?"));
	if (result != KMessageBox::Continue) return MAD_FLOW_BREAK;
    } else if (m_failures == 1) {
	result = Kwave::MessageBox::warningYesNo(m_parent_widget,
	    error + "\n" +
	    i18n("Do you want to continue and ignore all following errors?"));
        m_failures++;
	if (result != KMessageBox::Yes) return MAD_FLOW_BREAK;
    }

    return MAD_FLOW_CONTINUE;
}

//***************************************************************************
enum mad_flow MP3Decoder::fillInput(struct mad_stream *stream)
{
    Q_ASSERT(m_source);
    if (!m_source) return MAD_FLOW_STOP;

    // check if the user pressed cancel
    if (m_dest->isCanceled()) return MAD_FLOW_STOP;

    // preserve the remaining bytes from the last pass
    int rest = stream->bufend - stream->next_frame;
    if (rest) memmove(m_buffer, stream->next_frame, rest);

    // clip source at "eof-appended_bytes"
    unsigned int bytes_to_read = m_buffer_size - rest;
    if (m_source->pos() + bytes_to_read > m_source->size()-m_appended_bytes)
        bytes_to_read = m_source->size() - m_appended_bytes-m_source->pos();

    // abort if nothing more to read, even if there are
    // some "left-overs" from the previous pass
    if (!bytes_to_read) return MAD_FLOW_STOP;

    // read from source to fill up the buffer
    unsigned int size = rest;
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
struct audio_dither {
  mad_fixed_t error[3];
  mad_fixed_t random;
};

/**
 * 32-bit pseudo-random number generator
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
static inline unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/**
 * generic linear sample quantize and dither routine
 * (copied from mpg231, mad.c)
 * @author Rob Leslie
 */
static inline int32_t audio_linear_dither(unsigned int bits,
    mad_fixed_t sample, struct audio_dither *dither)
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
    output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

    scalebits = MAD_F_FRACBITS + 1 - bits;
    mask = (1L << scalebits) - 1;

    /* dither */
    random  = prng(dither->random);
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
enum mad_flow MP3Decoder::processOutput(void */*data*/,
    struct mad_header const */*header*/, struct mad_pcm *pcm)
{
    static struct audio_dither dither;
    int32_t sample;
    Kwave::SampleArray buffer(pcm->length);

    // loop over all tracks
    const unsigned int tracks = m_dest->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
	unsigned int nsamples = pcm->length;
	mad_fixed_t const *p = pcm->samples[track];
	unsigned int ofs = 0;

	// and render samples into Kwave's internal format
	while (nsamples--) {
	    sample = static_cast<int32_t>(audio_linear_dither(SAMPLE_BITS,
	             static_cast<mad_fixed_t>(*p++), &dither));
	    buffer[ofs++] = static_cast<sample_t>(sample);
	}
	*(*m_dest)[track] << buffer;
    }

    return MAD_FLOW_CONTINUE;
}

//***************************************************************************
bool MP3Decoder::decode(QWidget *widget, MultiTrackWriter &dst)
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
		     0 /* header */,
		     0 /* filter */,
                     _output_adapter,
                     _error_adapter, 0 /* message */);

    // decode through libmad...
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    // release the decoder
    mad_decoder_finish(&decoder);

    return (result == 0);
}

//***************************************************************************
void MP3Decoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
