/***************************************************************************
           LineParser.h  -  parses a string buffer into lines
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

#ifndef _LINE_PARSER_H_
#define _LINE_PARSER_H_

#include <qcstring.h>
#include <qstring.h>

/**
 * @class LineParser
 * Parses a string that consists of multiple lines, separates
 * and returns single lines.
 */
class LineParser
{
public:
    /**
     * Constructor.
     * @param init string buffer
     */
    LineParser(const QByteArray &init);

    /** Destructor. */
    ~LineParser ();

    /**
     * Parses one line and returns the line without trailing
     * or leading newlines or whitespaces. If the end of the
     * string buffer is reached, the returned string will be
     * zero-length. Zero-length strings in the string buffer
     * will be skipped.
     */
    QString nextLine();

private:

    /** Internal string buffer */
    QByteArray m_buffer;

    /** current position */
    unsigned int m_pos;
};

#endif /* _LINE_PARSER_H_ */
