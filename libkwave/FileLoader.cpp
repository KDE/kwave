/***************************************************************************
         FileLoader.cpp  -  loader for loading and buffering small files
			     -------------------
    begin                : Jan 20 2001
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

#include "FileLoader.h"

#include <stdio.h>

#include <qcstring.h>
#include <qstring.h>

//***************************************************************************
//***************************************************************************
FileLoader::FileLoader(const QString &name)
{
    m_buf = 0;
    FILE *in = fopen(name.latin1(), "r");
    if (in) {
	fseek (in, 0, SEEK_END);
	unsigned int size = ftell (in);
	fseek (in, 0, SEEK_SET);
	
	m_buf.resize(size + 1);
	m_buf.fill(0x00);

	if (m_buf.size() == size+1) {
	    fread(m_buf.data(), size, 1, in);
	} else qDebug("FileLoader:not enough memory for reading file !");
    } else qDebug("FileLoader:could not open file !");

    fclose(in);
}

//***************************************************************************
FileLoader::~FileLoader ()
{
}

//***************************************************************************
const QByteArray &FileLoader::buffer()
{
    return m_buf;
}

//***************************************************************************
//***************************************************************************
