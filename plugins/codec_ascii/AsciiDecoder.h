/*************************************************************************
         AsciiDecoder.h  -  decoder for ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef ASCII_DECODER_H
#define ASCII_DECODER_H

#include "config.h"

#include <QQueue>
#include <QString>
#include <QTextStream>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class AsciiDecoder: public Kwave::Decoder
    {
    public:
        /** Constructor */
        AsciiDecoder();

        /** Destructor */
        virtual ~AsciiDecoder() override;

        /** Returns a new instance of the decoder */
        virtual Kwave::Decoder *instance() override;

        /**
         * Opens the source and decodes the header information.
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param source file or other source with a stream of bytes
         * @return true if succeeded, false on errors
         */
        virtual bool open(QWidget *widget, QIODevice &source) override;

        /**
         * Decodes a stream of bytes into a MultiWriter
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param dst MultiWriter that receives the audio data
         * @return true if succeeded, false on errors
         */
        virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst)
            override;

        /**
         * Closes the source.
         */
        virtual void close() override;

    private:

        /**
         * try to read a complete line from the source, skip empty lines
         * and comments
         * @return true if one line was read, false if EOF was reached
         *         before one line was complete
         */
        bool readNextLine();

    private:

        /** source of the audio data */
        QTextStream m_source;

        /** destination of the audio data */
        Kwave::MultiWriter *m_dest;

        /** queue for buffering strings read from the file */
        QQueue<QString> m_queue_input;

        /** last read line number, starting with 1 */
        quint64 m_line_nr;

    };
}

#endif /* ASCII_DECODER_H */

//***************************************************************************
//***************************************************************************
