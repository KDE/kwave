/*************************************************************************
    ID3_QIODeviceWriter.cpp  -  Adapter between QIODevice and ID3_Writer
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

#include <config.h>

#include <QtCore/QIODevice>

#include "ID3_QIODeviceWriter.h"

//***************************************************************************
Kwave::ID3_QIODeviceWriter::ID3_QIODeviceWriter(QIODevice &dest)
    :ID3_Writer(), m_dest(dest), m_written(0)
{
}

//***************************************************************************
Kwave::ID3_QIODeviceWriter::~ID3_QIODeviceWriter()
{
}

//***************************************************************************
void Kwave::ID3_QIODeviceWriter::close()
{
}

//***************************************************************************
void Kwave::ID3_QIODeviceWriter::flush()
{
}

//***************************************************************************
ID3_Writer::pos_type Kwave::ID3_QIODeviceWriter::getBeg()
{
    return 0;
}

//***************************************************************************
ID3_Writer::pos_type Kwave::ID3_QIODeviceWriter::getEnd()
{
    return getMaxSize();
}

//***************************************************************************
ID3_Writer::pos_type Kwave::ID3_QIODeviceWriter::getCur()
{
    return m_written;
}

//***************************************************************************
ID3_Writer::size_type Kwave::ID3_QIODeviceWriter::getSize()
{
    return m_written;
}

//***************************************************************************
ID3_Writer::size_type Kwave::ID3_QIODeviceWriter::getMaxSize()
{
    return (1U << ((sizeof(ID3_Writer::size_type) * 8) - 1));
}

//***************************************************************************
ID3_Writer::size_type Kwave::ID3_QIODeviceWriter::writeChars(
	    const ID3_Writer::char_type buf[], ID3_Writer::size_type len)
{
    return this->writeChars(reinterpret_cast<const char *>(buf), len);
}

//***************************************************************************
ID3_Writer::size_type Kwave::ID3_QIODeviceWriter::writeChars(
	    const char buf[], ID3_Writer::size_type len)
{
    ID3_Writer::size_type bytes =  static_cast<ID3_Writer::size_type>(
	m_dest.write(&(buf[0]), static_cast<qint64>(len)));
    if (bytes > 0)
	m_written += bytes;
    return bytes;
}

//***************************************************************************
bool Kwave::ID3_QIODeviceWriter::atEnd()
{
    return false;
}

//***************************************************************************
//***************************************************************************
