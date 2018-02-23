/*************************************************************************
          WavEncoder.h  -  encoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef WAV_ENCODER_H
#define WAV_ENCODER_H

#include "config.h"

#include <QList>

#include "libkwave/Encoder.h"
#include "libkwave/MetaDataList.h"

#include "WavPropertyMap.h"

class QWidget;

namespace Kwave
{

    class LabelList;

    class WavEncoder: public Kwave::Encoder
    {
    public:
	/** Constructor */
	WavEncoder();

	/** Destructor */
        virtual ~WavEncoder() Q_DECL_OVERRIDE;

	/** Returns a new instance of the encoder */
        virtual Encoder *instance() Q_DECL_OVERRIDE;

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
                            const Kwave::MetaDataList &meta_data)
            Q_DECL_OVERRIDE;

	/** Returns a list of supported file properties */
        virtual QList<Kwave::FileProperty> supportedProperties()
            Q_DECL_OVERRIDE;

    private:

	/**
	 * write the INFO chunk with all known file properties
	 *
	 * @param dst file or other source to receive a stream of bytes
	 * @param info information about the file to be saved
	 */
	void writeInfoChunk(QIODevice &dst, Kwave::FileInfo &info);

	/**
	 * write the 'cue list' and the label names (if any)
	 *
	 * @param dst file or other source to receive a stream of bytes
	 * @param labels a list of labels
	 */
	void writeLabels(QIODevice &dst, const Kwave::LabelList &labels);

	/**
	 * Fix the size of the "data" and the "RIFF" chunk, as libaudiofile
	 * is sometimes really buggy due to internal calculations done
	 * with "float" as data type. This can lead to broken files as the
	 * data and also the RIFF chunk sizes are too small.
	 *
	 * @param dst file or other source to receive a stream of bytes
	 * @param info information about the file to be saved
	 * @param frame_size number of bytes per sample
	 */
	void fixAudiofileBrokenHeaderBug(QIODevice &dst, Kwave::FileInfo &info,
	                                 unsigned int frame_size);

    private:

	/** map for translating chunk names to FileInfo properties */
	Kwave::WavPropertyMap m_property_map;

    };
}

#endif /* WAV_ENCODER_H */

//***************************************************************************
//***************************************************************************
