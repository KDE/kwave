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

#include <qiodevice.h>
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
    return m_source.size() ? m_source.size() - 1 : 0;
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::getCur()
{
    return m_source.at();
}

//***************************************************************************
ID3_Reader::pos_type ID3_QIODeviceReader::setCur(ID3_Reader::pos_type pos)
{
    m_source.at(pos);
    return m_source.at();
}

//***************************************************************************
ID3_Reader::int_type ID3_QIODeviceReader::readChar()
{
    ID3_Reader::int_type ch = m_source.getch();
    return ch;
}

//***************************************************************************
ID3_Reader::int_type ID3_QIODeviceReader::peekChar()
{
    ID3_Reader::pos_type pos = m_source.at();
    ID3_Reader::int_type ch = readChar();
    m_source.at(pos);
    return ch;
}

//***************************************************************************
ID3_Reader::size_type ID3_QIODeviceReader::readChars(
    ID3_Reader::char_type buf[], ID3_Reader::size_type len)
{
    char_type *p = &(buf[0]);
    ID3_Reader::size_type size = m_source.readBlock((char*)p, len);
    return size;
}

//***************************************************************************
//***************************************************************************
