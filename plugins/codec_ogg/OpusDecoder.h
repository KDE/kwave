/*************************************************************************
          OpusDecoder.h  -  sub decoder for Opus in an Ogg container
                             -------------------
    begin                : Wed Dec 26 2012
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

#ifndef OPUS_DECODER_H
#define OPUS_DECODER_H

#ifdef HAVE_OGG_OPUS
#include "config.h"

#include <ogg/ogg.h>
#include <opus/opus.h>
#include <opus/opus_multistream.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackSink.h"
#include "libkwave/Sample.h"
#include "libkwave/VorbisCommentMap.h"
#include "libkwave/modules/SampleBuffer.h"

#include "OggSubDecoder.h"
#include "OpusHeader.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class MultiWriter;
    class StreamObject;

    class OpusDecoder: public Kwave::OggSubDecoder
    {
    public:
        /**
         * Constructor
         *
         * @param source pointer to a QIODevice to read from, must not be null
         * @param oy sync and verify incoming physical bitstream
         * @param os take physical pages, weld into a logical stream of packets
         * @param og one Ogg bitstream page, Opus packets are inside
         * @param op one raw packet of data for decode
         */
        OpusDecoder(QIODevice *source,
                    ogg_sync_state &oy,
                    ogg_stream_state &os,
                    ogg_page &og,
                    ogg_packet &op);

        /** destructor */
        ~OpusDecoder() override {}

        /**
         * parse the header of the stream and initialize the decoder
         * @param widget a QWidget to be used as parent for error messages
         * @param info reference to a FileInfo to fill
         * @return -1 if failed or +1 if succeeded
         */
        virtual int open(QWidget *widget, Kwave::FileInfo &info)
            override;

        /**
         * decode received ogg data
         * @param dst a MultiWriter to be used as sink
         * @return -1 if failed or >= 0 if succeeded
         */
        int decode(Kwave::MultiWriter &dst) override;

        /** reset the stream info */
        void reset() override;

        /**
         * finish the decoding, last chance to fix up some file info
         * @param info reference to a FileInfo to fill
         */
        void close(Kwave::FileInfo &info) override;

    protected:

        /**
         * Parses an Ogg comment into a into Kwave FileInfo.
         * If more than one occurrence is found, they are concatenated as a
         * semicolon separated list.
         * @param info the file info object to add the value
         * @param comment string with the full comment, assumed "tag=value"
         */
        void parseComment(Kwave::FileInfo &info, const QString &comment);

        /**
         * parse the "OpusHeader" header of the stream
         * @param widget a QWidget to be used as parent for error messages
         * @param info reference to a FileInfo to fill
         * @return -1 if failed or +1 if succeeded
         */
        virtual int parseOpusHead(QWidget *widget, Kwave::FileInfo &info);

        /**
         * parse the "OpusTags" header of the stream
         * @param widget a QWidget to be used as parent for error messages
         * @param info reference to a FileInfo to fill
         * @return -1 if failed or +1 if succeeded
         */
        virtual int parseOpusTags(QWidget *widget, Kwave::FileInfo &info);

    private:
        /** IO device to read from */
        QIODevice *m_source;

        /** first stream with audio data */
        qint64 m_stream_start_pos;

        /** last known position in the output stream [samples] */
        sample_index_t m_samples_written;

        /** sync and verify incoming physical bitstream */
        ogg_sync_state &m_oy;

        /** take physical pages, weld into a logical stream of packets */
        ogg_stream_state &m_os;

        /** one Ogg bitstream page.  Opus packets are inside */
        ogg_page &m_og;

        /** one raw packet of data for decode */
        ogg_packet &m_op;

        /** the Opus stream header */
        Kwave::opus_header_t m_opus_header;

        /** Opus multistream decoder object */
        OpusMSDecoder *m_opus_decoder;

        /** map for translating Opus comments to Kwave FileInfo */
        Kwave::VorbisCommentMap m_comments_map;

        /** buffer for decoded raw audio data */
        float *m_raw_buffer;

        /** multi track buffer, for blockwise writing to the destination */
        Kwave::MultiTrackSink<Kwave::SampleBuffer, true> *m_buffer;

        /** sample rate converter (when needed) */
        Kwave::StreamObject *m_rate_converter;

        /**
         * if true, the output of the rate converter or sample buffer has
         * been connected to the decoder's sink
         */
        bool m_output_is_connected;

        /** total number of packets */
        unsigned int m_packet_count;

        /**
         * total number of raw samples, at the input of the decoder,
         * before rate conversion
         */
        quint64 m_samples_raw;

        /** total number of bytes, without overhead */
        quint64 m_bytes_count;

        /** minimum detected length of a packet [samples] */
        int m_packet_len_min;

        /** maximum detected length of a packet [samples] */
        int m_packet_len_max;

        /** minimum detected size of a packet [bytes] */
        int m_packet_size_min;

        /** maximum detected size of a packet [bytes] */
        int m_packet_size_max;

        /** first detected granule pos (minimum) */
        qint64 m_granule_first;

        /** last detected granule pos (maximum) */
        qint64 m_granule_last;

        /** number of samples missing at the start (in first granule) */
        qint64 m_granule_offset;

        /** number of samples to skip at start */
        int m_preskip;
    };
}

#endif /* HAVE_OGG_OPUS */

#endif /* OPUS_DECODER_H */

//***************************************************************************
//***************************************************************************
