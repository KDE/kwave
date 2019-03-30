/*************************************************************************
           MP3Decoder.h  -  decoder for MP3 data
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

#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include "config.h"

#include <mad.h>		// needs libmad-devel package

#include <id3/globals.h>

#include <QString>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

#include "ID3_PropertyMap.h"

class ID3_Frame;
class ID3_Tag;
class QWidget;

namespace Kwave
{
    class MP3Decoder: public Kwave::Decoder
    {
    public:

	/** Constructor */
	MP3Decoder();

	/** Destructor */
	virtual ~MP3Decoder();

	/** Returns a new instance of the decoder */
	virtual Kwave::Decoder *instance() Q_DECL_OVERRIDE;

	/**
	 * Opens the source and decodes the header information.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param source file or other source with a stream of bytes
	 * @return true if succeeded, false on errors
	 */
	virtual bool open(QWidget *widget, QIODevice &source) Q_DECL_OVERRIDE;

	/**
	 * Decodes a stream of bytes into a MultiWriter
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param dst MultiWriter that receives the audio data
	 * @return true if succeeded, false on errors
	 */
	virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst)
	    Q_DECL_OVERRIDE;

	/**
	* Closes the source.
	*/
	virtual void close() Q_DECL_OVERRIDE;

	/** Callback for filling libmad's input buffer */
	enum mad_flow fillInput(struct mad_stream *stream);

	/** Calback for processing libmad's output */
	enum mad_flow processOutput(void *data,
				    struct mad_header const *header,
				    struct mad_pcm *pcm);

	/** Callback for handling stream errors */
	enum mad_flow handleError(void *data, struct mad_stream *stream,
				struct mad_frame *frame);

    private:

	/** parse MP3 headers */
	bool parseMp3Header(const Mp3_Headerinfo &header, QWidget *widget);

	/** parse all known ID3 tags */
	bool parseID3Tags(ID3_Tag &tag);

	/**
	 * parse a ID3 frame into a string
	 * @param frame a ID3 frame
	 * @return QString with the content
	 */
	QString parseId3Frame2String(const ID3_Frame *frame);

    private:

	/** property - to - ID3 mapping */
	ID3_PropertyMap m_property_map;

	/** source of the raw mp3 data */
	QIODevice *m_source;

	/** destination of the audio data */
	Kwave::MultiWriter *m_dest;

	/** buffer for libmad */
	unsigned char *m_buffer;

	/** size of m_buffer in bytes */
	int m_buffer_size;

	/** number of prepended bytes / id3v2 tag */
	 size_t m_prepended_bytes;

	/** number of appended bytes / id3v1 tag */
	size_t m_appended_bytes;

	/** number of failures */
	unsigned int m_failures;

	/** widget used for displaying error messages */
	QWidget *m_parent_widget;

    };
}

#endif /* MP3_DECODER_H */

//***************************************************************************
//***************************************************************************
