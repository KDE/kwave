/*************************************************************************
           MP3Encoder.h  -  export of MP3 data via "lame"
                             -------------------
    begin                : Sat May 19 2012
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

#ifndef MP3_ENCODER_H
#define MP3_ENCODER_H

#include "config.h"

#include <climits>

#include <QList>
#include <QMutex>
#include <QProcess>

#include "libkwave/Encoder.h"

#include "ID3_PropertyMap.h"

class ID3_Tag;
class QIODevice;
class QWidget;

namespace Kwave
{

    class MultiTrackReader;

    class MP3Encoder: public Kwave::Encoder
    {
        Q_OBJECT
    public:
        /** Constructor */
        MP3Encoder();

        /** Destructor */
        ~MP3Encoder() override;

        /** Returns a new instance of the encoder */
        Kwave::Encoder *instance() override;

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
                            override;

        /** Returns a list of supported file properties */
        virtual QList<Kwave::FileProperty> supportedProperties()
                            override;

    private slots:

        /** called when data from the external process is available */
        void dataAvailable();

    private:

        /**
         * encode all meta data into ID3 tags
         * @param meta_data reference to the meta data to encode
         * @param tag the ID3 tag to receive the ID3 frames
         */
        void encodeID3Tags(const Kwave::MetaDataList &meta_data,
                           ID3_Tag &tag);

    private:

        /** property - to - ID3 mapping */
        ID3_PropertyMap m_property_map;

        /** lock for protecting m_dst and m_process */
        QMutex m_lock;

        /** pointer to the QIODevice for storing, used while encoding */
        QIODevice *m_dst;

        /** the external process with the encoder */
        QProcess m_process;

        /** path to the external program */
        QString m_program;

        /** list with commandline parameters */
        QStringList m_params;

        /**
         * buffer for writing to the encoder
         * @note The size of this buffer should never be bigger than
         * PIPE_BUF (see POSIX.1-2001), otherwise there could be some
         * leftover when writing to the stdin queue of the process
         * which would be caught and queued up within Qt.
         * After some time that leads to a critically increasing memory
         * consumption and a large delay when the progress bar has
         * reached 99%.
         */
        quint8 m_write_buffer[PIPE_BUF];

        /** buffer for reading from the encoder (size is not critical) */
        char m_read_buffer[PIPE_BUF];

    };
}

#endif /* MP3_ENCODER_H */

//***************************************************************************
//***************************************************************************
