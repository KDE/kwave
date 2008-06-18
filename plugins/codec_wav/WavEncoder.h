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

#ifndef _WAV_ENCODER_H_
#define _WAV_ENCODER_H_

#include "config.h"

#include <QList>

#include "libkwave/Encoder.h"

#include "WavPropertyMap.h"

class QWidget;

class WavEncoder: public Encoder
{
public:
    /** Constructor */
    WavEncoder();

    /** Destructor */
    virtual ~WavEncoder();

    /** Returns a new instance of the encoder */
    virtual Encoder *instance();

    /**
     * Encodes a signal into a stream of bytes.
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param src MultiTrackReader used as source of the audio data
     * @param dst file or other source to receive a stream of bytes
     * @param info information about the file to be saved
     * @return true if succeeded, false on errors
     */
    virtual bool encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, FileInfo &info);

    /** Returns a list of supported file properties */
    virtual QList<FileProperty> supportedProperties();

private:

    /**
     * write the INFO chunk with all known file properties
     *
     * @param dst file or other source to receive a stream of bytes
     * @param info information about the file to be saved
     */
    void writeInfoChunk(QIODevice &dst, FileInfo &info);

    /**
     * write the 'cue list' and the label names (if any)
     *
     * @param dst file or other source to receive a stream of bytes
     * @param info information about the file to be saved, including labels
     */
    void writeLabels(QIODevice &dst, FileInfo &info);

private:

    /** map for translating chunk names to FileInfo properties */
    WavPropertyMap m_property_map;

};

#endif /* _WAV_ENCODER_H_ */
