/*************************************************************************
    ID3_QIODeviceReader.cpp  -  Adapter between QIODevice and ID3_Reader
                             -------------------
    begin                : Wed Aug 14 2002
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

#include "config.h"

#include <QIODevice>

#include "ID3_QIODeviceReader.h"

//***************************************************************************
ID3_QIODeviceReader::ID3_QIODeviceReader(QIODevice &source)
        :ID3_Reader(), m_source(source)
{
}

//***************************************************************************
ID3_QIODeviceReader::~ID3_QIODeviceReader()
{
}

//***************************************************************************
void ID3_QIODeviceReader::close()
{
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::getBeg()
{
    return 0;
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::getEnd()
{
    return static_cast<ID3_Reader::pos_type>(m_source.size());
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::getCur()
{
    return static_cast<ID3_Reader::pos_type>(m_source.pos());
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::setCur(ID3_Reader::pos_type pos)
{
    m_source.seek(static_cast<qint64>(pos));
    return m_source.pos();
}

//***************************************************************************
ID3_Reader::int_type ID3_QIODeviceReader::readChar()
{
    unsigned char c = 0;
    m_source.getChar(reinterpret_cast<char *>(&c));
    return static_cast<ID3_Reader::int_type>(c);
}

//***************************************************************************
ID3_Reader::int_type ID3_QIODeviceReader::peekChar()
{
    qint64 pos = m_source.pos();
    ID3_Reader::int_type ch = readChar();
    m_source.seek(pos);
    return ch;
}

//***************************************************************************
ID3_Reader::size_type ID3_QIODeviceReader::readChars(
    char_type buf[], size_type len)
{
     qint64 size = m_source.read(
	reinterpret_cast<char *>(&(buf[0])),
	static_cast<qint64>(len)
    );
    return static_cast<ID3_Reader::size_type>(size);
}

//***************************************************************************
ID3_Reader::size_type ID3_QIODeviceReader::readChars(
    char buf[], size_type len)
{
    return this->readChars(reinterpret_cast<char_type *>(buf), len);
}

//***************************************************************************
//***************************************************************************
