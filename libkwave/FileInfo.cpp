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
#include <klocale.h>
#include "FileInfo.h"

/** FileInfo flag: for internal usage only, do not show to the user */
#define FP_INTERNAL     0x0001

/** FileInfo flag: readonly, cannot be modified by the user */
#define FP_READONLY     0x0002

/** FileInfo flag: available for the GUI, but not for loading/saving */
#define FP_NO_LOAD_SAVE 0x0004

/***************************************************************************/
void FileInfo::PropertyTypesMap::fill()
{
    append(INF_ALBUM, 0,
        I18N_NOOP("Album"),
        i18n("Name of the album if the source is an album\n"
             "that consist of more medias."));
    append(INF_ANNOTATION, 0,
        I18N_NOOP("Annotation"),
        i18n("Provides general comments about the file or the subject of\n"
             "the file. If the comment is several sentences long, end\n"
             "each sentence with a period. Do not include newline\n"
             "characters!"));
    append(INF_ARCHIVAL, 0,
        I18N_NOOP("Archival location"),
        i18n("Indicates where the subject of the file is archived."));
    append(INF_AUTHOR, 0,
        I18N_NOOP("Author"),
        i18n("Identifies the name of the author of the original\n"
             "subject of the file."
             "\nExample: 'van Beethoven, Ludwig'"));
    append(INF_BITRATE_LOWER, FP_NO_LOAD_SAVE,
        I18N_NOOP("Lower Bitrate"),
        i18n("Specifies the lower limit in a VBR bitstream."));
    append(INF_BITRATE_NOMINAL, FP_NO_LOAD_SAVE,
        I18N_NOOP("Bitrate"),
        i18n("Nominal bitrate of the audio stream in bits per second"));
    append(INF_BITRATE_UPPER, FP_NO_LOAD_SAVE,
        I18N_NOOP("Upper Bitrate"),
        i18n("Specifies the upper limit in a VBR bitstream."));
    append(INF_CD, 0,
        I18N_NOOP("CD"),
        i18n("Number of the CD, if the source is\nan album of more CDROMs"));
    append(INF_COMMISSIONED, 0,
        I18N_NOOP("Commisioned"),
        i18n("Lists the name of the person or organization\n"
             "that commissioned the subject of the file."));
    append(INF_COMMENTS, 0,
        I18N_NOOP("Comments"),
        i18n("Provides general comments about the file or the subject of\n"
             "the file. If the comment is several sentences long, end\n"
             "each sentence with a period. Do not include newline\n"
             "characters!"));
    append(INF_COMPRESSION, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Compression"),
        i18n("Sets a mode for compressing the audio\n"
             "data to reduce disk space."));
    append(INF_CONTACT, 0,
        I18N_NOOP("Contact"),
        i18n("Contact information for the creators or distributors of\n"
             "the track. This could be a URL, an email address, the\n"
             "physical address of the producing label."));
    append(INF_COPYRIGHT, 0,
        I18N_NOOP("Copyright"),
        i18n("Records the copyright information for the file. If there are\n"
             "multiple copyrights, separate them by a semicolon followed\n"
             "by a space.\n"
             "Example: 'Copyright Linux community 2002'"));
    append(INF_COPYRIGHTED, 0,
        I18N_NOOP("Copyrighted"),
        i18n("Indicates whether the file is protected by copyright or not."));
    append(INF_CREATION_DATE, 0,
        I18N_NOOP("Date"),
        i18n("Specifies the date the subject of the file was created.\n"
             "Example: '2001-12-24'"));
    append(INF_ENGINEER, 0,
        I18N_NOOP("Engineer"),
        i18n("Shows the name of the engineer who worked on the file.\n"
             "If there are multiple engineers, separate the names by\n"
             "a semicolon and a blank."));
    append(INF_FILENAME, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Filename"),
        i18n("Name of the opened file"));
    append(INF_FILESIZE, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("File Size"),
        i18n("Size of the file in bytes"));
    append(INF_GENRE, 0,
        I18N_NOOP("Genre"),
        i18n("Describes the genre or style of the original work.\n"
             "Examples: 'classic', 'pop'"));
    append(INF_ISRC, FP_READONLY,
        I18N_NOOP("ISRC"),
        i18n("ISRC number for the track; see the ISRC intro page\n"
             "for more information on ISRC numbers.\n"
             "http://www.ifpi.org/site-content/online/isrc_intro.html"));
    append(INF_KEYWORDS, 0,
        I18N_NOOP("Keywords"),
        i18n("Provides a list of keywords that refer to the\n"
             "file or subject of the file."));
    append(INF_LABELS, FP_INTERNAL,
        I18N_NOOP("Labels"),
        i18n("The list of labels/markers."));
    append(INF_LICENSE, 0,
        I18N_NOOP("License"),
        i18n("License information, e.g., 'All Rights Reserved',\n"
             "'Any Use Permitted', an URL to a license or the\n"
             "EFF Open Audio License ('distributed under the\n"
             "terms of the Open Audio License.\n"
             "See http://www.eff.org/IP/Open_licenses/eff_oal.html\n"
             "for details'), etc."));
    append(INF_MEDIUM, 0,
        I18N_NOOP("Medium"),
        i18n("Describes the original subject of the file,\n"
             "where it was first recorded.\n"
             "Example: 'orchester'"));
    append(INF_MIMETYPE, FP_READONLY | FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Mime Type"),
        i18n("Mime type of the file format"));
    append(INF_MPEG_EMPHASIS, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Emphasis"),
        i18n("Audio emphasis mode"));
    append(INF_MPEG_LAYER, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Layer"),
        i18n("MPEG Layer, I, II or III"));
    append(INF_MPEG_MODEEXT, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Mode Extension"),
        i18n("MPEG Mode Extension (only if Joint Stereo)"));
    append(INF_MPEG_VERSION, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Version"),
        i18n("MPEG Version, 1, 2 or 2.5"));
    append(INF_NAME, 0,
        I18N_NOOP("Name"),
        i18n("Stores the title of the subject of the file.\n"
             "Example: \"Symphony No.6, Op.68 'Pastoral'\""));
    append(INF_ORGANIZATION, 0,
        I18N_NOOP("Organization"),
        i18n("Name of the organization producing the track\n"
             "(i.e. the 'record label')"));
    append(INF_ORIGINAL, FP_NO_LOAD_SAVE,
        I18N_NOOP("Original"),
        i18n("Indicates whether the file is an original or a copy"));
    append(INF_PERFORMER, 0,
        I18N_NOOP("Performer"),
        i18n("The artist(s) who performed the work. In classical\n"
             "music this would be the conductor, orchestra, soloists.\n"
             "In an audio book it would be the actor who did the reading."));
    append(INF_PRIVATE, 0,
        I18N_NOOP("Private"),
        i18n("Indicates whether the subject is private"));
    append(INF_PRODUCT, 0,
        I18N_NOOP("Product"),
        i18n("Specifies the name or the title the\n"
             "file was originally intended for.\n"
             "Example: 'Linux audio collection'"));
    append(INF_SAMPLE_FORMAT, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Sample Format"),
        i18n("Format used for storing the digitized audio samples.\n"
             "Example: '32-bit IEEE floating-point'"));
    append(INF_SOFTWARE, 0,
        I18N_NOOP("Software"),
        i18n("Identifies the name of the software package\n"
             "used to create the file.\n"
             "Example: 'Kwave v0.6.4-1'"));
    append(INF_SOURCE, 0,
        I18N_NOOP("Source"),
        i18n("Identifies the name of the person or organization\n"
             "who supplied the original subject of the file.\n"
             "Example: 'Chaotic Sound Research'"));
    append(INF_SOURCE_FORM, 0,
        I18N_NOOP("Source form"),
        i18n("Identifies the original form of\n"
             "the material that was digitized.\n"
             "Examples: 'Record/Vinyl/90RPM', 'Audio DAT', "
             "'tape/CrO2/60min'"));
    append(INF_SUBJECT, 0,
        I18N_NOOP("Subject"),
        i18n("Describes the subject of the file.\n"
             "Example: 'Bird voices at early morning'"));
    append(INF_TECHNICAN, 0,
        I18N_NOOP("Technican"),
        i18n("Identifies the technican who digitized the subject file.\n"
             "Example: 'Torvalds, Linus'"));
    append(INF_TRACK, 0,
        I18N_NOOP("Track"),
        i18n("Track of the CD if the source was a CDROM."));
    append(INF_VBR_QUALITY, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Base Quality"),
        i18n("Base quality of the compression in VBR mode"));
    append(INF_VERSION, 0,
        I18N_NOOP("Version"),
        i18n("May be used to differentiate multiple versions\n"
             "of the same track title in a single collection.\n"
             "(e.g. remix info)"));

    // please do not simply extend here, sort in alphabetically instead...

}

/***************************************************************************/
QList<FileProperty> FileInfo::PropertyTypesMap::all()
{
    return allKeys();
}

/***************************************************************************/
/***************************************************************************/
FileInfo::FileInfo()
    :m_length(), m_rate(0.0), m_bits(0), m_tracks(0), m_labels(),
     m_properties(), m_property_map()
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
    clear();
}

/***************************************************************************/
bool FileInfo::equals(const FileInfo &other)
{
    if (m_bits   != other.bits())   return false;
    if (m_length != other.length()) return false;
    if (m_rate   != other.rate())   return false;
    if (m_tracks != other.tracks()) return false;
    if (m_labels == other.labels()) return false;

    if (m_properties.count() != other.properties().count()) return false;

    QMap<FileProperty, QVariant>::Iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); ++it) {
	if (it.value() != other.get(it.key())) return false;
    }

    return true;
}

/***************************************************************************/
void FileInfo::copy(const FileInfo &source)
{
    m_length     = source.length();
    m_rate       = source.rate();
    m_bits       = source.bits();
    m_tracks     = source.tracks();
    m_labels     = source.labels();
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
	m_properties[key] = value;
    }
}

/***************************************************************************/
QVariant FileInfo::get(FileProperty key) const
{
    if (!m_properties.contains(key)) return QVariant();
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
QList<FileProperty> FileInfo::allKnownProperties()
{
    return m_property_map.all();
}

/***************************************************************************/
void FileInfo::clear()
{
    m_length = 0;
    m_rate   = 0;
    m_bits   = 0;
    m_tracks = 0;
    m_labels.clear();
    m_properties.clear();
}

/***************************************************************************/
void FileInfo::dump()
{
    qDebug("--- dump of file info ---");

    qDebug("default properties:");
    qDebug("   length = %lu samples", static_cast<unsigned long int>(m_length));
    qDebug("   rate   = %0.1f Hz", m_rate);
    qDebug("   bits   = %u", m_bits);
    qDebug("   tracks = %u", m_tracks);

    qDebug("labels:");
    foreach (Label label, m_labels) {
	qDebug("   [%10lu] = '%s'", static_cast<unsigned long int>(label.pos()),
	                           label.name().toLocal8Bit().data());
    }

    qDebug("other properties:");
    QMap<FileProperty, QVariant>::Iterator it;
    for (it = m_properties.begin(); it != m_properties.end(); ++it) {
	FileProperty key = it.key();
	QVariant val = it.value();
	QString name = m_property_map.name(key);
	qDebug("   '%s' = '%s'", name.toLocal8Bit().data(),
	                         val.toString().toLocal8Bit().data());
    }
    qDebug("-------------------------");
}

/***************************************************************************/
/***************************************************************************/
