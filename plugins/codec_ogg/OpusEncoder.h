/*************************************************************************
    OpusEncoder.h  -  sub encoder base class for Opus in an Ogg container
                             -------------------
    begin                : Thu Jan 03 2013
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

#ifndef _OPUS_ENCODER_H_
#define _OPUS_ENCODER_H_

#include "config.h"

#include <ogg/ogg.h>
#include <opus/opus.h>
#include <opus/opus_multistream.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackSink.h"
#include "libkwave/VorbisCommentMap.h"
#include "libkwave/modules/SampleBuffer.h"

#include "OggSubEncoder.h"
#include "OpusHeader.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class ChannelMixer;
    class FileInfo;
    class MultiTrackReader;
    class StreamObject;

    class OpusEncoder: public Kwave::OggSubEncoder
    {
    public:

	/**
	 * Constructor
	 * @param os reference to the ogg stream state
	 * @param og reference to the ogg page
	 * @param op reference to the ogg packet
	 */
	OpusEncoder(ogg_stream_state &os,
	            ogg_page         &og,
	            ogg_packet       &op);

	/** Destructor */
	virtual ~OpusEncoder();

	/**
	 * parse the header of the stream and initialize the decoder
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @param src MultiTrackReader used as source of the audio data
	 * @param src MultiTrackReader used as source of the audio data
	 * @return true if succeeded, false if failed
	 */
	virtual bool open(QWidget *widget,
	                  const Kwave::FileInfo &info,
	                  Kwave::MultiTrackReader &src);

	/**
	 * write the header information
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
	virtual bool writeHeader(QIODevice &dst);

	/**
	 * encode received ogg data
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
	virtual bool encode(Kwave::MultiTrackReader &src,
	                    QIODevice &dst);

	/**
	 * finished the encoding, clean up
	 */
	virtual void close();

    private:

	/**
	 * set up the downmixing mode
	 * @param widget a QWidget to be used as parent for error messages
	 * @param tracks number of tracks
	 * @param bitrats in bits/sec or -1 for "auto"
	 * @return true if succeeded or false if failed/canceled
	 */
	bool setupDownMix(QWidget *widget, unsigned int tracks, int bitrate);

	/**
	 * determine the bitrate to use for encoding
	 * @param widget a QWidget to be used as parent for error messages
	 * @param tracks number of tracks
	 * @return true if succeeded or false if failed/canceled
	 */
	bool setupBitrate(QWidget *widget, unsigned int tracks);

	/**
	 * determine the sample rate to use for encoding
	 * @param widget a QWidget to be used as parent for error messages
	 * @param tracks number of tracks
	 * @param rate sample rate of the original in samples/sec
	 * @return true if succeeded or false if failed/canceled
	 */
	bool setupCodingRate(QWidget *widget, unsigned int tracks, double rate);

	/**
	 * set up the encoder, including packet size and channel mapping
	 * @param widget a QWidget to be used as parent for error messages
	 * @param tracks number of tracks
	 * @param rate sample rate of the original in samples/sec
	 * @return true if succeeded or false if failed/canceled
	 */
	bool setupEncoder(QWidget *widget, unsigned int tracks, double rate);

	/**
	 * set up the bitrate mode, e.g. VBR, CBR etc
	 * @param widget a QWidget to be used as parent for error messages
	 * @return true if succeeded or false if failed/canceled
	 */
	bool setupBitrateMode(QWidget *widget);

	/**
	 * write the Opus header into the Ogg stream
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded or false if failed/canceled
	 */
	bool writeOpusHeader(QIODevice &dst);

	/**
	 * write the Opus tags into the Ogg stream
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded or false if failed/canceled
	 */
	bool writeOpusTags(QIODevice &dst);

	/**
	 * Write the current ogg page to the destination IO device
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded or false if failed/canceled
	 */
	bool writeOggPage(QIODevice &dst);

	/**
	 * Fill the input buffer of the encoder with samples.
	 * @param src MultiTrackReader used as source of the audio data
	 * @return number of samples read
	 */
	unsigned int fillInBuffer(Kwave::MultiTrackReader &src);

    private:

	/** map for translating Opus comments to Kwave FileInfo */
	Kwave::VorbisCommentMap m_comments_map;

	/** file info, set in open(...) */
	Kwave::FileInfo m_info;

	/** take physical pages, weld into a logical stream of packets */
	ogg_stream_state &m_os;

	/** one Ogg bitstream page.  Opus packets are inside */
	ogg_page         &m_og;

	/** one raw packet of data for decode */
	ogg_packet       &m_op;

	/**
	 * downmix mode: off, automatic, mono or stereo
	 */
	enum {
	    DOWNMIX_OFF    = -1, /**< no downmixing               */
	    DOWNMIX_AUTO   =  0, /**< automatic, based on bitrate */
	    DOWNMIX_MONO   =  1, /**< downmix to mono             */
	    DOWNMIX_STEREO =  2  /**< downmix to stereo           */
	} m_downmix;

	/** bitrate in bits/sec */
	int m_bitrate;

	/** encoding sample rate in bits/sec */
	int m_coding_rate;

	/** number of tracks used for encoding, after downmixing */
	unsigned int m_encoder_channels;

	/** channel mixer (if downmixing is required) */
	Kwave::ChannelMixer *m_channel_mixer;

	/** sample rate converter (if needed) */
	Kwave::StreamObject *m_rate_converter;

	/** frame size in samples */
	unsigned int m_frame_size;

	/** number of samples to pad at the end to compensate preskip */
	unsigned int m_extra_out;

	/** Opus header, including channel map */
	Kwave::opus_header_t m_opus_header;

	/** maximum number of bytes per frame */
	unsigned int m_max_frame_bytes;

	/** buffer for one packet */
	unsigned char *m_packet_buffer;

	/** the Opus multistream encoder */
	OpusMSEncoder *m_encoder;

	/** input buffer of the encoder */
	float *m_encoder_input;

	/**
	 * end of the queue that possibly consists of a channel mixer (in
	 * case of downmixing), a rate converter (if needed) and a buffer
	 * (if one of the two objects mentioned before is present).
	 */
	Kwave::StreamObject *m_last_queue_element;

	/** multi track buffer, for blockwise reading from the source device */
	Kwave::MultiTrackSink<Kwave::SampleBuffer, true> *m_buffer;
    };
}

#endif /* _OPUS_ENCODER_H_ */

//***************************************************************************
//***************************************************************************
