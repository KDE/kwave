/*************************************************************************
  ID3_QIODeviceWriter.h  -  Adapter between QIODevice and ID3_Writer
                             -------------------
    begin                : Mon May 28 2012
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

#ifndef ID3_QIODEVICE_WRITER_H
#define ID3_QIODEVICE_WRITER_H

#include "config.h"

#include <id3/globals.h>
#include <id3/writer.h>

class QIODevice;

namespace Kwave
{
    /**
     * @class ID3_QIODeviceWriter
     * Adapter between QIODevice and ID3_Writer
     */
    class ID3_QIODeviceWriter: public ID3_Writer
    {
    public:

        /** Constructor */
        explicit ID3_QIODeviceWriter(QIODevice &dest);

        /** Destructor */
        virtual ~ID3_QIODeviceWriter();

        /** Close the destination. Not implemented. */
        void close() override;

        /** Flush the destination, not implemented (not needed) */
        void flush() override;

        /** Get the start position, always zero */
        ID3_Writer::pos_type getBeg() override;

        /** Get the end position, identical to size()-1 */
        ID3_Writer::pos_type getEnd() override;

        /** Returns the current position */
        ID3_Writer::pos_type getCur() override;

        /** Returns the number of bytes written */
        ID3_Writer::size_type getSize() override;

        /** Returns the maximum number of bytes written */
        ID3_Writer::size_type getMaxSize() override;

        /**
         * Write up to len characters into buf and advance the internal position
         * accordingly.
         */
        virtual ID3_Writer::size_type writeChars(
            const ID3_Writer::char_type buf[], ID3_Writer::size_type len)
            override;

        virtual ID3_Writer::size_type writeChars(
            const char buf[], ID3_Writer::size_type len)
            override;

        /** returns true if the writer has readed eof (always false) */
        bool atEnd() override;

    private:

        /** reference to a QIODevice that is used as destination */
        QIODevice &m_dest;

        /** number of bytes written */
        ID3_Writer::size_type m_written;
    };
}

#endif /* ID3_QIODEVICE_WRITER_H */

//***************************************************************************
//***************************************************************************
