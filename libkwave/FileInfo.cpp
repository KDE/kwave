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

#define FP_INTERNAL 0x0001

/***************************************************************************/
void FileInfo::PropertyTypesMap::fill()
{
    append(INF_FILENAME, 0,
        i18n("Filename"),    i18n("Name of the opened file"));
    append(INF_MIMETYPE, 0,
        i18n("Mime Type"),   i18n("Mime type of the file format"));
    append(INF_NAME, 0,
        i18n("Name"),        i18n("Name of the song or whatever"));
    append(INF_AUTHOR, 0,
        i18n("Author"),      i18n("Name of the author/artist"));
    append(INF_ANNOTATION, 0,
        i18n("Annotation"),  i18n("Annotation/comment"));
    append(INF_ARCHIVAL, 0,
        i18n("Archival"),    i18n("Archival location"));
    append(INF_ARTIST, 0,
        i18n("Artist"),      i18n("Name ot the artist"));
    append(INF_COMMISSIONED, 0,
        i18n("Commisioned"), i18n("Name of the commissioner"));
    append(INF_COMMENTS, 0,
        i18n("Comments"),    i18n("Comments"));
    append(INF_COPYRIGHT, 0,
        i18n("Copyright"),   i18n("Copyright notice"));
    append(INF_CREATION_DATE, 0,
        i18n("Date"),        i18n("Date of the creation"));
    append(INF_ENGINEER, 0,
        i18n("Engineer"),    i18n("Name of the engineer"));
    append(INF_GENRE, 0,
        i18n("Genre"),       i18n("Genre of the song"));
    append(INF_KEYWORDS, 0,
        i18n("Keywords"),    i18n("Keywords for indexing"));
    append(INF_MEDIUM, 0,
        i18n("Medium"),      i18n("Medium where it was first recorded"));
    append(INF_PRODUCT, 0,
        i18n("Product"),     i18n("Name of the product to which it belongs"));
    append(INF_SOFTWARE, 0,
        i18n("Software"),    i18n("Name of the software"));
    append(INF_SOURCE_FORM, 0,
        i18n("Source form"), i18n("Source form of the medium"));
    append(INF_TECHNICAN, 0,
        i18n("Technican"),   i18n("Name of the technican"));
    append(INF_SUBJECT, 0,
        i18n("Subject"),     i18n("Subject of the song"));
    append(INF_SAMPLE_FORMAT, FP_INTERNAL,
        "sample_fmt", 0);
    append(INF_SAMPLE_FORMAT_NAME, 0,
        "Sample Format",     i18n("Format of the samples"));
    // please extend here...
}

/***************************************************************************/
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
void FileInfo::set(FileProperty key, const QVariant &value)
{
    if (!value.isValid()) {
	// delete
	m_properties.remove(key);
    } else {
	// insert or add
	m_properties.replace(key, value);
    }
}

/***************************************************************************/
const QVariant &FileInfo::get(FileProperty key)
{
    return m_properties[key];
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
    debug("   rate   = %0.1f Hz", m_rate);
    debug("   bits   = %u", m_bits);
    debug("   tracks = %u", m_tracks);
    debug("other properties:");
    QMap<FileProperty, QVariant>::Iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); ++it) {
        FileProperty key = it.key();
        QVariant val = it.data();
        QString name = m_property_map.name(key);
        debug("   '%s' = '%s'", name.data(), val.toString().data());
    }
    debug("-------------------------");
}

/***************************************************************************/
/***************************************************************************/
