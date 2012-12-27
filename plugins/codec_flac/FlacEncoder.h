/*************************************************************************
          FlacEncoder.h  -  encoder for FLAC data
                             -------------------
    begin                : Tue Feb 28 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#ifndef _FLAC_ENCODER_H_
#define _FLAC_ENCODER_H_

#include "config.h"

#include <QtCore/QList>

#include <FLAC/format.h>
#include <FLAC++/encoder.h>
#include <FLAC++/metadata.h>

#include <vorbis/vorbisenc.h>

#include "libkwave/Encoder.h"
#include "libkwave/VorbisCommentMap.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class MultiTrackReader;

    class FlacEncoder: public Kwave::Encoder,
                       protected FLAC::Encoder::Stream
    {
    public:
	/** Constructor */
	FlacEncoder();

	/** Destructor */
	virtual ~FlacEncoder();

	/** Returns a new instance of the encoder */
	virtual Kwave::Encoder *instance();

	/**
	 * Encodes a signal into a stream of bytes.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst file or other source to receive a stream of bytes
	 * @param meta_data meta data of the file to save
	 * @return true if succeeded, false on errors
	 */
	virtual bool encode(QWidget *widget, Kwave::MultiTrackReader &src,
	                    QIODevice &dst,
	                    const Kwave::MetaDataList &meta_data);

	/** Returns a list of supported file properties */
	virtual QList<Kwave::FileProperty> supportedProperties();

    protected:

	/**
	 * Callback for writing data to the FLAC layer
	 *
	 * @param buffer array with samples
	 * @param bytes length of the buffer in bytes
	 * @param samples the number of samples
	 * @param current_frame index of the current frame
	 * @return FLAC stream encoder write status
	 */
	virtual ::FLAC__StreamEncoderWriteStatus write_callback(
	    const FLAC__byte buffer[], size_t bytes,
	    unsigned samples, unsigned current_frame);

	/**
	 * Callback for encoding meta data
	 *
	 * @param metadata pointer to a FLAC metadata structure that will
	 *        be filled
	 */
	virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);

	/**
	 * Encode all Kwave file info into FLAC meta data
	 *
	 * @param info information about the file to be saved
	 * @param flac_metadata QList with collects the FLAC metadata
	 */
	virtual void encodeMetaData(const Kwave::FileInfo &info,
	    QVector<FLAC__StreamMetadata *> &flac_metadata);

    protected:

	class VorbisCommentContainer
	{
	public:
	    /** Constructor */
	    VorbisCommentContainer();

	    /** Destructor */
	    virtual ~VorbisCommentContainer();

	    /**
	     * add a new comment
	     *
	     * @param tag name of the vorbis tag, as string
	     * @param value the value of the tag, as string
	     */
	    void add(const QString &tag, const QString &value);

	    /** Returns a pointer to the FLAC metadata */
	    FLAC__StreamMetadata *data();

	private:
	    /** list of metadata objects */
	    FLAC__StreamMetadata *m_vc;
	};

    private:

	/** map for translating vorbis comments to FileInfo properties */
	Kwave::VorbisCommentMap m_vorbis_comment_map;

	/** pointer to the QIODevice for storing, used while encoding */
	QIODevice *m_dst;

    };
}

#endif /* _FLAC_ENCODER_H_ */

//***************************************************************************
//***************************************************************************
