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

#include <kmessagebox.h>

#include <id3/globals.h>
#include <id3/tag.h>
#include <id3/misc_support.h>

#include "libkwave/CompressionType.h"
#include "libkwave/GenreType.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libgui/ConfirmCancelProxy.h"

#include "libmad/mad.h"

#include "MP3Decoder.h"
#include "ID3_QIODeviceReader.h"

//***************************************************************************
MP3Decoder::MP3Decoder()
    :Decoder(), m_source(0)
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
    debug("crc = 0x%08X", header.crc);
    if ((header.crc == MP3CRC_MISMATCH) || (header.crc == MP3CRC_ERROR_SIZE)){
	if (KMessageBox::warningContinueCancel(widget,
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
	    warning("unknown mpeg layer '%d'", header.layer);
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
	    warning("unknown mpeg version '%d'", header.version);
    }

    /* bit rate */
    if (header.bitrate > 0) m_info.set(INF_MPEG_BITRATE,
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
	    if (KMessageBox::warningContinueCancel(widget,
	        i18n("The file contains an invalid channel mode 0x"
	             "%1\nAssuming Mono...").arg(mode))
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

   
    debug("framesize=%d", header.framesize);
    
    debug("frames = %u", header.frames);
    
    m_info.set(INF_PRIVATE, header.privatebit);
    m_info.set(INF_COPYRIGHTED, header.copyrighted);
    m_info.set(INF_ORIGINAL, header.original);

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
	    //ID3FID_OWNERSHIP         // Ownership frame.
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
	    //ID3FID_ENCODEDBY         // Encoded by.
	    //ID3FID_LYRICIST          // Lyricist/Text writer.
	    //ID3FID_FILETYPE          // File type.
	    //ID3FID_TIME              // Time.
	    //ID3FID_CONTENTGROUP      // Content group description.

	    case ID3FID_TITLE:        // Title/songname/content description.
		parseId3Frame(frame, INF_NAME); break;

	    //ID3FID_SUBTITLE         // Subtitle/Description refinement.
	    //ID3FID_INITIALKEY       // Initial key.
	    //ID3FID_LANGUAGE         // Language(s).
	    //ID3FID_SONGLEN          // Length.
	    //ID3FID_MUSICIANCREDITLIST // Musician credits list.

	    case ID3FID_MEDIATYPE:    // Medium type.
		parseId3Frame(frame, INF_MEDIUM); break;

	    //ID3FID_MOOD             // Mood.
	    //ID3FID_ORIGALBUM        // Original album/movie/show title.
	    //ID3FID_ORIGFILENAME     // Original filename.
	    //ID3FID_ORIGLYRICIST     // Original lyricist(s)/text writer(s).

	    case ID3FID_ORIGARTIST:   // Original artist(s)/performer(s).
		parseId3Frame(frame, INF_AUTHOR); break;
	    
	    //ID3FID_ORIGYEAR         // Original release year.
	    //ID3FID_FILEOWNER        // File owner/licensee.
	    case ID3FID_LEADARTIST:   // Lead performer(s)/Soloist(s).
		parseId3Frame(frame, INF_ARTIST); break;
	    
	    //ID3FID_BAND             // Band/orchestra/accompaniment.
	    //ID3FID_CONDUCTOR        // Conductor/performer refinement.
	    //ID3FID_MIXARTIST        // Interpreted, remixed, or otherwise
	                              // modified by.
	    case ID3FID_PARTINSET:    // Part of a set.
		parseId3Frame(frame, INF_CD); break;
	    
	    //ID3FID_PRODUCEDNOTICE   // Produced notice.
	    //ID3FID_PUBLISHER        // Publisher.

	    case ID3FID_TRACKNUM:     // Track number/Position in set.
		parseId3Frame(frame, INF_TRACK); break;

	    //ID3FID_RECORDINGDATES   // Recording dates.
	    //ID3FID_NETRADIOSTATION  // Internet radio station name.
	    //ID3FID_NETRADIOOWNER    // Internet radio station owner.
	    //ID3FID_SIZE             // Size.
	    //ID3FID_ALBUMSORTORDER   // Album sort order.
	    //ID3FID_PERFORMERSORTORDER// Performer sort order.
	    //ID3FID_TITLESORTORDER   // Title sort order.
	    //ID3FID_ISRC             // ISRC (international standard
	                              // recording code).
	    //ID3FID_ENCODERSETTINGS  // Software/Hardware and settings
	                              // used for encoding.
	    //ID3FID_SETSUBTITLE      // Set subtitle.

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
		debug("frame with id=%d, descr=%s, text=%s",
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
    debug("MP3Decoder::open()"); // ###
    info().clear();
    ASSERT(!m_source);
    if (m_source) warning("MP3Decoder::open(), already open !");

    /* open the file in readonly mode with seek enabled */
    ASSERT(src.isDirectAccess());
    if (!src.isDirectAccess()) return false;
    if (!src.open(IO_ReadOnly)) {
	warning("unable to open source in read-only mode!");
	return false;
    }
    
    /* read all available ID3 tags */
    ID3_Tag tag;
    ID3_QIODeviceReader adapter(src);
    tag.Link(adapter, ID3TT_ALL);

    debug("NumFrames = %d", tag.NumFrames());
    debug("Size = %d",      tag.Size());
    debug("HasLyrics = %d", tag.HasLyrics());
    debug("HasV1Tag = %d",  tag.HasV1Tag());
    debug("HasV2Tag = %d",  tag.HasV2Tag());

    const Mp3_Headerinfo *mp3hdr = tag.GetMp3HeaderInfo();
    if (!mp3hdr) {
	KMessageBox::sorry(widget,
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

    return true;
}

//***************************************************************************
bool MP3Decoder::decode(QWidget */*widget*/, MultiTrackWriter &dst)
{
    ASSERT(m_source);
    if (!m_source) return false;

//    struct mad_stream stream;
//
//    mad_stream_init(&stream);
//
//    mad_stream_finish(&stream);
    
    return true; // ###
}

//***************************************************************************
void MP3Decoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
