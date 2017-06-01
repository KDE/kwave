/*************************************************************************
     AudiofileDecoder.h  -  import through libaudiofile
                             -------------------
    begin                : Tue May 28 2002
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

#ifndef AUDIOFILE_DECODER_H
#define AUDIOFILE_DECODER_H

#include "config.h"

#include "libkwave/Decoder.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class VirtualAudioFile;

    class AudiofileDecoder: public Kwave::Decoder
    {
    public:
	/** Constructor */
	AudiofileDecoder();

	/** Destructor */
	virtual ~AudiofileDecoder();

	/** Returns a new instance of the decoder */
	Kwave::Decoder *instance() Q_DECL_OVERRIDE;

	/**
	 * Opens the source and decodes the header information.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param source file or other source with a stream of bytes
	 * @return true if succeeded, false on errors
	 */
	bool open(QWidget *widget, QIODevice &source) Q_DECL_OVERRIDE;

	/**
	 * Decodes a stream of bytes into a MultiWriter
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param dst MultiWriter that receives the audio data
	 * @return true if succeeded, false on errors
	 */
	bool decode(QWidget *widget, Kwave::MultiWriter &dst) Q_DECL_OVERRIDE;

	/**
	 * Closes the source.
	 */
	void close() Q_DECL_OVERRIDE;

    private:

	/** source of the audio data */
	QIODevice *m_source;

	/** adapter for libaudiofile */
	Kwave::VirtualAudioFile *m_src_adapter;
    };
}

#endif /* AUDIOFILE_DECODER_H */

//***************************************************************************
//***************************************************************************
