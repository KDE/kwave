/***************************************************************************
           FileInfo.cpp  -  information about an audio file
			     -------------------
    begin                : Mar 13 2002
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
#include "FileInfo.h"

/***************************************************************************/
FileInfo::FileInfo()
    :m_length(), m_rate(0.0), m_bits(0), m_tracks(0), m_properties()
{
}

/***************************************************************************/
FileInfo::FileInfo(const FileInfo &inf)
    :m_length(inf.length()), m_rate(inf.rate()), m_bits(inf.bits()),
     m_tracks(inf.tracks()), m_properties(inf.m_properties)
{
}

/***************************************************************************/
FileInfo::~FileInfo()
{
}

/***************************************************************************/
void FileInfo::set(const QString &name, const QVariant &value)
{
    if (!value.isValid()) {
	// delete
	m_properties.remove(name);
    } else {
	// insert or add
	m_properties.replace(name, value);
    }
}

/***************************************************************************/
const QVariant &FileInfo::get(const QString &name)
{
    return m_properties[name];
}

/***************************************************************************/
void FileInfo::clear()
{
    m_length = 0;
    m_rate   = 0;
    m_bits   = 0;
    m_tracks = 0;
    m_properties.clear();
}

/***************************************************************************/
void FileInfo::dump()
{
    debug("--- dump of file info ---");
    debug("default properties:");
    debug("   length = %u samples", m_length);
    debug("   rate   = %0.1gHz", m_rate);
    debug("   bits   = %u", m_bits);
    debug("   tracks = %u", m_tracks);
    debug("other properties:");
    QMap<QString, QVariant>::Iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); ++it) {
        QString key = it.key();
        QVariant val = it.data();
        debug("   '%s' = '%s'", key.data(), val.toString().data());
    }
    debug("-------------------------");
}

/***************************************************************************/
/***************************************************************************/
