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

#include "libkwave/CompressionType.h"
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
    debug("header info = %p", mp3hdr);

    /* parse the MP3 header */
    parseMp3Header(*mp3hdr, widget);
    
    
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
