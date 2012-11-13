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

#include "libkwave/FileInfo.h"
#include "libkwave/MetaDataList.h"

/** FileInfo flag: for internal usage only, do not show to the user */
#define FP_INTERNAL     0x0001

/** FileInfo flag: readonly, cannot be modified by the user */
#define FP_READONLY     0x0002

/** FileInfo flag: available for the GUI, but not for loading/saving */
#define FP_NO_LOAD_SAVE 0x0004

/** prefix of all property names */
#define FILE_INFO_PROPERTY_PREFIX "FILE_INFO: "

/***************************************************************************/
void Kwave::FileInfo::PropertyTypesMap::fill()
{
    append(Kwave::INF_UNKNOWN, FP_INTERNAL | FP_NO_LOAD_SAVE,
           QString(), QString());

    append(Kwave::INF_ALBUM, 0,
        I18N_NOOP("Album"),
        i18n("Name of the album if the source is an album\n"
             "that consist of more medias."));
    append(Kwave::INF_ANNOTATION, 0,
        I18N_NOOP("Annotation"),
        i18n("Provides general comments about the file or the subject of\n"
             "the file. If the comment is several sentences long, end\n"
             "each sentence with a period. Do not include newline\n"
             "characters!"));
    append(Kwave::INF_ARCHIVAL, 0,
        I18N_NOOP("Archival location"),
        i18n("Indicates where the subject of the file is archived."));
    append(Kwave::INF_AUTHOR, 0,
        I18N_NOOP("Author"),
        i18n("Identifies the name of the author of the original\n"
             "subject of the file."
             "\nExample: 'van Beethoven, Ludwig'"));
    append(Kwave::INF_BITRATE_LOWER, FP_NO_LOAD_SAVE,
        I18N_NOOP("Lower Bitrate"),
        i18n("Specifies the lower limit in a VBR bitstream."));
    append(Kwave::INF_BITRATE_NOMINAL, FP_NO_LOAD_SAVE,
        I18N_NOOP("Bitrate"),
        i18n("Nominal bitrate of the audio stream in bits per second"));
    append(Kwave::INF_BITRATE_UPPER, FP_NO_LOAD_SAVE,
        I18N_NOOP("Upper Bitrate"),
        i18n("Specifies the upper limit in a VBR bitstream."));
    append(Kwave::INF_BITS_PER_SAMPLE, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Bits per Sample"),
        i18n("Specifies the number of bits per sample."));
    append(Kwave::INF_CD, 0,
        I18N_NOOP("CD"),
        i18n("Number of the CD, if the source is\nan album of more CDROMs"));
    append(Kwave::INF_CDS, 0,
        I18N_NOOP("CDS"),
        i18n("Number of CDs, if the source is\nan album of more CDROMs"));
    append(Kwave::INF_COMMISSIONED, 0,
        I18N_NOOP("Commisioned"),
        i18n("Lists the name of the person or organization\n"
             "that commissioned the subject of the file."));
    append(Kwave::INF_COMMENTS, 0,
        I18N_NOOP("Comments"),
        i18n("Provides general comments about the file or the subject of\n"
             "the file. If the comment is several sentences long, end\n"
             "each sentence with a period. Do not include newline\n"
             "characters!"));
    append(Kwave::INF_COMPRESSION, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Compression"),
        i18n("Sets a mode for compressing the audio\n"
             "data to reduce disk space."));
    append(Kwave::INF_CONTACT, 0,
        I18N_NOOP("Contact"),
        i18n("Contact information for the creators or distributors of\n"
             "the track. This could be a URL, an email address, the\n"
             "physical address of the producing label."));
    append(Kwave::INF_COPYRIGHT, 0,
        I18N_NOOP("Copyright"),
        i18n("Records the copyright information for the file. If there are\n"
             "multiple copyrights, separate them by a semicolon followed\n"
             "by a space.\n"
             "Example: 'Copyright Linux community 2002'"));
    append(Kwave::INF_COPYRIGHTED, 0,
        I18N_NOOP("Copyrighted"),
        i18n("Indicates whether the file is protected by copyright or not."));
    append(Kwave::INF_CREATION_DATE, 0,
        I18N_NOOP("Date"),
        i18n("Specifies the date the subject of the file was created.\n"
             "Example: '2001-12-24'"));
    append(Kwave::INF_ENGINEER, 0,
        I18N_NOOP("Engineer"),
        i18n("Shows the name of the engineer who worked on the file.\n"
             "If there are multiple engineers, separate the names by\n"
             "a semicolon and a blank."));
    append(Kwave::INF_FILENAME, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Filename"),
        i18n("Name of the opened file"));
    append(Kwave::INF_FILESIZE, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("File Size"),
        i18n("Size of the file in bytes"));
    append(Kwave::INF_GENRE, 0,
        I18N_NOOP("Genre"),
        i18n("Describes the genre or style of the original work.\n"
             "Examples: 'classic', 'pop'"));
    append(Kwave::INF_ISRC, FP_READONLY,
        I18N_NOOP("ISRC"),
        i18n("ISRC number for the track; see the ISRC intro page\n"
             "for more information on ISRC numbers.\n"
             "http://www.ifpi.org/site-content/online/isrc_intro.html"));
    append(Kwave::INF_KEYWORDS, 0,
        I18N_NOOP("Keywords"),
        i18n("Provides a list of keywords that refer to the\n"
             "file or subject of the file."));
    append(Kwave::INF_LABELS, FP_INTERNAL,
        I18N_NOOP("Labels"),
        i18n("The list of labels/markers."));
    append(Kwave::INF_LENGTH, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Length"),
        i18n("Length of the file in samples."));
    append(Kwave::INF_LICENSE, 0,
        I18N_NOOP("License"),
        i18n("License information, e.g., 'All Rights Reserved',\n"
             "'Any Use Permitted', an URL to a license or the\n"
             "EFF Open Audio License ('distributed under the\n"
             "terms of the Open Audio License.\n"
             "See http://www.eff.org/IP/Open_licenses/eff_oal.html\n"
             "for details'), etc."));
    append(Kwave::INF_MEDIUM, 0,
        I18N_NOOP("Medium"),
        i18n("Describes the original subject of the file,\n"
             "where it was first recorded.\n"
             "Example: 'orchester'"));
    append(Kwave::INF_MIMETYPE, FP_READONLY | FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Mime Type"),
        i18n("Mime type of the file format"));
    append(Kwave::INF_MPEG_EMPHASIS, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Emphasis"),
        i18n("Audio emphasis mode"));
    append(Kwave::INF_MPEG_LAYER, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Layer"),
        i18n("MPEG Layer, I, II or III"));
    append(Kwave::INF_MPEG_MODEEXT, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Mode Extension"),
        i18n("MPEG Mode Extension (only if Joint Stereo)"));
    append(Kwave::INF_MPEG_VERSION, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Version"),
        i18n("MPEG Version, 1, 2 or 2.5"));
    append(Kwave::INF_NAME, 0,
        I18N_NOOP("Name"),
        i18n("Stores the title of the subject of the file.\n"
             "Example: \"Symphony No.6, Op.68 'Pastoral'\""));
    append(Kwave::INF_ORGANIZATION, 0,
        I18N_NOOP("Organization"),
        i18n("Name of the organization producing the track\n"
             "(i.e. the 'record label')"));
    append(Kwave::INF_ORIGINAL, FP_NO_LOAD_SAVE,
        I18N_NOOP("Original"),
        i18n("Indicates whether the file is an original or a copy"));
    append(Kwave::INF_PERFORMER, 0,
        I18N_NOOP("Performer"),
        i18n("The artist(s) who performed the work. In classical\n"
             "music this would be the conductor, orchestra, soloists.\n"
             "In an audio book it would be the actor who did the reading."));
    append(Kwave::INF_PRIVATE, 0,
        I18N_NOOP("Private"),
        i18n("Indicates whether the subject is private"));
    append(Kwave::INF_PRODUCT, 0,
        I18N_NOOP("Product"),
        i18n("Specifies the name or the title the\n"
             "file was originally intended for.\n"
             "Example: 'Linux audio collection'"));
    append(Kwave::INF_SAMPLE_FORMAT, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Sample Format"),
        i18n("Format used for storing the digitized audio samples.\n"
             "Example: '32-bit IEEE floating-point'"));
    append(Kwave::INF_SAMPLE_RATE, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Sample Rate"),
        i18n("Number of samples per second\n"));
    append(Kwave::INF_SOFTWARE, 0,
        I18N_NOOP("Software"),
        i18n("Identifies the name of the software package\n"
             "used to create the file.\n"
             "Example: 'Kwave v0.6.4-1'"));
    append(Kwave::INF_SOURCE, 0,
        I18N_NOOP("Source"),
        i18n("Identifies the name of the person or organization\n"
             "who supplied the original subject of the file.\n"
             "Example: 'Chaotic Sound Research'"));
    append(Kwave::INF_SOURCE_FORM, 0,
        I18N_NOOP("Source form"),
        i18n("Identifies the original form of\n"
             "the material that was digitized.\n"
             "Examples: 'Record/Vinyl/90RPM', 'Audio DAT', "
             "'tape/CrO2/60min'"));
    append(Kwave::INF_SUBJECT, 0,
        I18N_NOOP("Subject"),
        i18n("Describes the subject of the file.\n"
             "Example: 'Bird voices at early morning'"));
    append(Kwave::INF_TECHNICAN, 0,
        I18N_NOOP("Technican"),
        i18n("Identifies the technican who digitized the subject file.\n"
             "Example: 'Torvalds, Linus'"));
    append(Kwave::INF_TRACK, 0,
        I18N_NOOP("Track"),
        i18n("Track of the CD if the source was a CDROM."));
    append(Kwave::INF_TRACKS, 0,
        I18N_NOOP("Tracks"),
        i18n("Number of tracks of the CD if the source was a CDROM."));
    append(Kwave::INF_CHANNELS, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Channels"),
        i18n("Specifies the number of channels of the signal."));
    append(Kwave::INF_VBR_QUALITY, FP_INTERNAL | FP_NO_LOAD_SAVE,
        I18N_NOOP("Base Quality"),
        i18n("Base quality of the compression in VBR mode"));
    append(Kwave::INF_VERSION, 0,
        I18N_NOOP("Version"),
        i18n("May be used to differentiate multiple versions\n"
             "of the same track title in a single collection.\n"
             "(e.g. remix info)"));

    // please do not simply extend here, sort in alphabetically instead...

}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::FileInfo::PropertyTypesMap::all() const
{
    return allKeys();
}

/***************************************************************************/
/***************************************************************************/
Kwave::FileInfo::FileInfo()
    :Kwave::MetaData(Kwave::MetaData::Signal),
     m_property_map()
{
    setProperty(Kwave::MetaData::STDPROP_TYPE, metaDataType());
}

/***************************************************************************/
Kwave::FileInfo::FileInfo(const Kwave::MetaDataList &meta_data_list)
    :Kwave::MetaData(
	(meta_data_list.selectByType(metaDataType()).isEmpty()) ?
	Kwave::MetaData(Kwave::MetaData::Signal) :
	meta_data_list.selectByType(metaDataType()).values().first()
    ),
     m_property_map()
{
    setProperty(Kwave::MetaData::STDPROP_TYPE, metaDataType());

    foreach (const Kwave::MetaData &meta_data, meta_data_list) {
	foreach (Kwave::FileProperty key, m_property_map.allKeys()) {
	    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);
	    if (meta_data.hasProperty(name)) {
		// take over each supported property
		QVariant value = meta_data[name];
		setProperty(name, value);
	    }
	}
    }
}

/***************************************************************************/
Kwave::FileInfo::~FileInfo()
{
}

/***************************************************************************/
bool Kwave::FileInfo::contains(Kwave::FileProperty key) const
{
    if (!m_property_map.allKeys().contains(key))
	return false;
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);
    return hasProperty(name);
}

/***************************************************************************/
void Kwave::FileInfo::set(Kwave::FileProperty key, const QVariant &value)
{
    if (!m_property_map.allKeys().contains(key))
	return;
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);
    setProperty(name, value);
}

/***************************************************************************/
QVariant Kwave::FileInfo::get(Kwave::FileProperty key) const
{
    if (!m_property_map.allKeys().contains(key))
	return QVariant();
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);

    if (!hasProperty(name)) return QVariant();
    return property(name);
}

/***************************************************************************/
bool Kwave::FileInfo::isInternal(Kwave::FileProperty key) const
{
    int flags = m_property_map.data(key);
    return (flags & FP_INTERNAL);
}

/***************************************************************************/
bool Kwave::FileInfo::canLoadSave(Kwave::FileProperty key) const
{
    int flags = m_property_map.data(key);
    return !(flags & FP_NO_LOAD_SAVE);
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::FileInfo::allKnownProperties() const
{
    return m_property_map.all();
}

/***************************************************************************/
const QMap<Kwave::FileProperty, QVariant> Kwave::FileInfo::properties() const
{
    QMap<Kwave::FileProperty, QVariant> map;
    foreach (Kwave::FileProperty key, m_property_map.allKeys()) {
	if (!contains(key)) continue;
	map[key] = get(key);
    }
    return map;
}

/***************************************************************************/
sample_index_t Kwave::FileInfo::length() const
{
    QVariant value = get(Kwave::INF_LENGTH);
    bool ok = false;
    sample_index_t len = static_cast<sample_index_t>(value.toULongLong(&ok));
    return (ok) ? len : 0;
}

/***************************************************************************/
void Kwave::FileInfo::setLength(sample_index_t length)
{
    set(Kwave::INF_LENGTH, QVariant(static_cast<qulonglong>(length)));
}

/***************************************************************************/
double Kwave::FileInfo::rate() const
{
    QVariant value = get(Kwave::INF_SAMPLE_RATE);
    bool ok = false;
    double r = value.toDouble(&ok);
    return (ok) ? r : 0;
}

/***************************************************************************/
void Kwave::FileInfo::setRate(double rate)
{
    set(Kwave::INF_SAMPLE_RATE, rate);
}

/***************************************************************************/
unsigned int Kwave::FileInfo::bits() const
{
    QVariant value = get(Kwave::INF_BITS_PER_SAMPLE);
    bool ok = false;
    unsigned int bits = value.toUInt(&ok);
    return (ok) ? bits : 0;
}

/***************************************************************************/
void Kwave::FileInfo::setBits(unsigned int bits)
{
    set(Kwave::INF_BITS_PER_SAMPLE, bits);
}

/***************************************************************************/
unsigned int Kwave::FileInfo::tracks() const
{
    QVariant value = get(Kwave::INF_CHANNELS);
    bool ok = false;
    unsigned int tracks = value.toUInt(&ok);
    return (ok) ? tracks : 0;
}

/***************************************************************************/
void Kwave::FileInfo::setTracks(unsigned int tracks)
{
    set(Kwave::INF_CHANNELS, tracks);
}

/***************************************************************************/
void Kwave::FileInfo::dump() const
{
    qDebug("--- dump of file info ---");
    qDebug("    id = #%s", id().toLocal8Bit().data());
    qDebug("default properties:");
    qDebug("   length = %lu samples", static_cast<unsigned long int>(length()));
    qDebug("   rate   = %0.1f Hz", rate());
    qDebug("   bits   = %u", bits());
    qDebug("   tracks = %u", tracks());

//     qDebug("labels:");
//     foreach (Kwave::Label label, m_labels) {
// 	qDebug("   [%10lu] = '%s'", static_cast<unsigned long int>(label.pos()),
// 	                           label.name().toLocal8Bit().data());
//     }

    qDebug("other properties:");
    QMap<Kwave::FileProperty, QVariant>::Iterator it;
    foreach (Kwave::FileProperty key, m_property_map.allKeys()) {
	if (!contains(key)) continue;
	QVariant val = get(key);
	QString name = m_property_map.name(key);
	qDebug("   '%s' = '%s'", name.toLocal8Bit().data(),
	                         val.toString().toLocal8Bit().data());
    }
    qDebug("-------------------------");
}

/***************************************************************************/
/***************************************************************************/
