/***************************************************************************
         LineParser.cpp  -  parses a string buffer into lines
			     -------------------
    begin                : Jan 28 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcstring.h>

#include "LineParser.h"

//***************************************************************************
//***************************************************************************
LineParser::LineParser(const QByteArray &init)
    :m_buffer(init), m_pos(0)
{
}

//***************************************************************************
LineParser::~LineParser ()
{
}

//***************************************************************************
QString LineParser::nextLine()
{
    // return with zero-length string if end reached
    unsigned int size = m_buffer.size();
    if (!size) return QString(0);

    QString line = "";
    while ((m_pos < size) && !line.length()) {
	line = "";
	while ((m_pos < size) && (m_buffer[m_pos] != 0x0D) &&
	    (m_buffer[m_pos] != 0x0A) && (m_buffer[m_pos] != 0x00))
	{
	    line += m_buffer[m_pos++];
	}
	line = line.stripWhiteSpace();
	m_pos++;
    }
    return line;
}

//***************************************************************************
//***************************************************************************
