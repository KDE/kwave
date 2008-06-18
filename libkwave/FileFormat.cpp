/***************************************************************************
         FileFormat.cpp  -  information about a file format
			     -------------------
    begin                : Mar 05 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "config.h"
#include "FileFormat.h"

//***************************************************************************
FileFormat::FileFormat(const QString &mime_type, const QStringList &sub_types,
                       const QString &description,
                       const QStringList &extensions)
    :m_mime_type(mime_type), m_sub_types(sub_types),
     m_description(description), m_extensions(extensions)
{
}

//***************************************************************************
FileFormat::~FileFormat()
{
}

//***************************************************************************
QString FileFormat::mimeType()
{
    return m_mime_type;
}

//***************************************************************************
QStringList FileFormat::subTypes()
{
    return m_sub_types;
}

//***************************************************************************
QString FileFormat::description()
{
    return m_description;
}

//***************************************************************************
QStringList FileFormat::extensions()
{
    return m_extensions;
}

//***************************************************************************
//***************************************************************************
