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

#define FP_INTERNAL     0x0001
#define FP_READONLY     0x0002
#define FP_NO_LOAD_SAVE 0x0004

/***************************************************************************/
void FileInfo::PropertyTypesMap::fill()
{
    append(INF_ANNOTATION, 0,
        i18n("Annotation"),
        i18n("Provides general comments about the file or the subject of "
             "the file. If the comment is several sentences long, end "
             "each sentence with a period. Do not include newline "
             "characters!"));
    append(INF_ARCHIVAL, 0,
        i18n("Archival location"),
        i18n("Indicates where the subject of the file is archived."));
    append(INF_ARTIST, 0,
        i18n("Artist"),
        i18n("Lists the artist of the original subject of the file."
             "\nExample: 'van Beethoven, Ludwig'"));
    append(INF_AUTHOR, 0,
        i18n("Author"),
        i18n("Identifies the name of the author of the original "
             "subject of the file."
             "\nExample: 'van Beethoven, Ludwig'"));
    append(INF_COMMISSIONED, 0,
        i18n("Commisioned"),
        i18n("Lists the name of the person or organization that commissioned "
             "the subject of the file."));
    append(INF_COMMENTS, 0,
        i18n("Comments"),
        i18n("Provides general comments about the file or the subject of "
             "the file. If the comment is several sentences long, end "
             "each sentence with a period. Do not include newline "
             "characters!"));
    append(INF_COMPRESSION, FP_INTERNAL | FP_NO_LOAD_SAVE,
        i18n("Compression"),
        i18n("Sets a mode for compressing the audio data to reduce "
             "disk space."));
    append(INF_COPYRIGHT, 0,
        i18n("Copyright"),
        i18n("Records the copyright information for the file. If there are "
             "multiple copyrights, separate them by a semicolon followed "
             "by a space."
             "\nExample: 'Copyright Linux community 2002'"));
    append(INF_CREATION_DATE, 0,
        i18n("Date"),
        i18n("Specifies the date the subject of the file was created. "
             "List dates in year-month-day format, padding one-digit "
             "months and days with a zero on the left (ISO format)."
             "\nExample: '2001-12-24'"));
    append(INF_ENGINEER, 0,
        i18n("Engineer"),
        i18n("Shows the name of the engineer who worked on the file. "
             "If there are multiple engineers, separate the names by "
             "a semicolon and a blank."));
    append(INF_FILENAME, FP_INTERNAL | FP_NO_LOAD_SAVE,
        i18n("Filename"),
        i18n("Name of the opened file"));
    append(INF_FILESIZE, FP_INTERNAL | FP_NO_LOAD_SAVE,
	i18n("File Size"),
	i18n("Size of the file in bytes"));
    append(INF_GENRE, 0,
        i18n("Genre"),
        i18n("Describes the genre or style of the original work."
             "\nExamples: 'classic', 'pop'"));
    append(INF_KEYWORDS, 0,
        i18n("Keywords"),
        i18n("Provides a list of keywords that refer to the file or "
             "subject of the file. Separate multiple keywords with "
             "a semicolon and a blank."));
    append(INF_MEDIUM, 0,
        i18n("Medium"),
        i18n("Describes the original subject of the file, where it "
             "was first recorded."
             "\nExample: 'orchester'"));
    append(INF_MIMETYPE, FP_READONLY,
        i18n("Mime Type"),
        i18n("Mime type of the file format"));
    append(INF_NAME, 0,
        i18n("Name"),
        i18n("Stores the title of the subject of the file."
             "\nExample: 'Symphony No.6, Op.68 \"Pastoral\"'"));
    append(INF_PRODUCT, 0,
        i18n("Product"),
        i18n("Specifies the name or the title the file was originally "
             "intended for."
             "\nExample: 'Linux audio collection'"));
    append(INF_SAMPLE_FORMAT, FP_INTERNAL | FP_NO_LOAD_SAVE,
             "sample_fmt", 0);
    append(INF_SAMPLE_FORMAT_NAME, FP_INTERNAL | FP_NO_LOAD_SAVE,
        i18n("Sample Format"),
        i18n("Format used for storing the digitized audio samples."
             "\nExample: '32-bit IEEE floating-point'"));
    append(INF_SOFTWARE, 0,
        i18n("Software"),
        i18n("Identifies the name of the software package used to "
             "create the file. "
             "\nExample: 'Kwave v0.6.4-1'"));
    append(INF_SOURCE, 0,
        i18n("Source"),
        i18n("Identifies the name of the person or organization "
             "who supplied the original subject of the file. "
             "\nExample: 'Chaotic Sound Research'"));
    append(INF_SOURCE_FORM, 0,
        i18n("Source form"),
        i18n("Identifies the original form of the material that was "
             "digitized."
             "\nExamples: 'Record/Vinyl/90RPM', 'Audio DAT', "
             "'tape/CrO2/60min'"));
    append(INF_SUBJECT, 0,
        i18n("Subject"),
        i18n("Describes the subject of the file."
             "\nExample: 'Bird voices at early morning'"));
    append(INF_TECHNICAN, 0,
        i18n("Technican"),
        i18n("Identifies the technican who digitized the subject file. "
             "\nExample: 'Torvalds, Linus'"));

    // please do not extend here, sort in alphabetically instead...

}

/***************************************************************************/
/***************************************************************************/
FileInfo::FileInfo()
    :m_length(), m_rate(0.0), m_bits(0), m_tracks(0), m_properties()
{
}

/***************************************************************************/
FileInfo::FileInfo(const FileInfo &inf)
{
    copy(inf);
}

/***************************************************************************/
FileInfo::~FileInfo()
{
}

/***************************************************************************/
void FileInfo::copy(const FileInfo &source)
{
    m_length     = source.length();
    m_rate       = source.rate();
    m_bits       = source.bits();
    m_tracks     = source.tracks();
    m_properties = source.properties();
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
const QVariant &FileInfo::get(FileProperty key) const
{
    return m_properties[key];
}

/***************************************************************************/
bool FileInfo::isInternal(FileProperty key)
{
    int flags = m_property_map.data(key);
    return (flags & FP_INTERNAL);
}

/***************************************************************************/
bool FileInfo::canLoadSave(FileProperty key)
{
    int flags = m_property_map.data(key);
    return !(flags & FP_NO_LOAD_SAVE);
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
