/*************************************************************************
    ID3_QIODeviceStream.h  -  Adapter between QIODevice and Taglib::IOStream
                             -------------------
    begin                : Tue Aug 27 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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

#ifndef TAGLIB_QIODEVICE_STREAM_H
#define TAGLIB_QIODEVICE_STREAM_H

#include "config.h"

#include <QtCore>
#include <QIODevice>

#include <taglib/taglib.h>
#include <taglib/tiostream.h>

namespace Kwave
{
    /**
     * @class ID3_QIODeviceStream
     * adapter between QIODevice and TagLib::IOStream
     */
    class TagLib_QIODeviceStream: public TagLib::IOStream
    {
    public:

        /**
         * constructor
         * @param device reference to the target QIODevice
         */
        explicit TagLib_QIODeviceStream(QIODevice &device)
            :TagLib::IOStream(), m_device(device)
        {
        }

        /** destructor */
        ~TagLib_QIODeviceStream() override = default;

        /**
         * get the file name
         * @return empty file name
         */
        TagLib::FileName name() const override
        {
            return TagLib::FileName("");
        }

        /**
         * read a block of data from the stream
         * @param length maximum number of bytes to read
         * @return byte vector containing the read data
         */
        TagLib::ByteVector readBlock(size_t length) override
        {
            TagLib::ByteVector buffer(static_cast<unsigned int>(length), 0);
            qint64 bytes_read = m_device.read(buffer.data(), length);
            if (bytes_read < 0)
                bytes_read = 0;
            buffer.resize(static_cast<unsigned int>(bytes_read));
            return buffer;
        }

        /**
         * write a block of data to the stream
         * @param data byte vector containing the data to write
         */
        void writeBlock(const TagLib::ByteVector &data) override
        {
            m_device.write(data.data(), data.size());
        }

        /**
         * insert data at a specific position (not implemented)
         * @param data byte vector to insert
         * @param start offset where insertion starts
         * @param replace number of bytes to replace
         */
        void insert(const TagLib::ByteVector &data,
                    TagLib::offset_t start = 0,
                    size_t replace = 0) override
        {
            Q_UNUSED(data);
            Q_UNUSED(start);
            Q_UNUSED(replace);
        }

        /**
         * remove a block of data (not implemented)
         * @param start offset where removal starts
         * @param length number of bytes to remove
         */
        void removeBlock(TagLib::offset_t start = 0, size_t length = 0) override
        {
            Q_UNUSED(start);
            Q_UNUSED(length);
        }

        /**
         * check if the stream is read-only
         * @return true if stream is read-only
         */
        bool readOnly() const override
        {
            return !m_device.isWritable();
        }

        /**
         * check if the stream is open
         * @return true if stream is open
         */
        bool isOpen() const override
        {
            return m_device.isOpen();
        }

        /**
         * reposition the stream offset
         * @param offset target position offset
         * @param p seek origin (Beginning, Current, or End)
         */
        void seek(TagLib::offset_t offset, Position p = Beginning) override
        {
            qint64 pos = 0;
            if (p == Beginning)
                pos = offset;
            else if (p == Current)
                pos = m_device.pos() + offset;
            else if (p == End)
                pos = m_device.size() + offset;
            m_device.seek(pos);
        }

        /** clear error flags (not implemented) */
        void clear() override
        {
        }

        /**
         * get the current stream position
         * @return current byte position
         */
        TagLib::offset_t tell() const override
        {
            return m_device.pos();
        }

        /**
         * get the total stream length
         * @return size of stream in bytes
         */
        TagLib::offset_t length() override
        {
            return m_device.size();
        }

        /**
         * truncate stream length (not implemented)
         * @param length target length in bytes
         */
        void truncate(TagLib::offset_t length) override
        {
            Q_UNUSED(length);
        }

    private:

        /** reference to a QIODevice used as stream destination/source */
        QIODevice &m_device;
    };

}

#endif /* TAGLIB_QIODEVICE_STREAM_H */

//***************************************************************************
//***************************************************************************
