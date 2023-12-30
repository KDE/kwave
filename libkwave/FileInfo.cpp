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
#include <KLocalizedString>

#include "libkwave/FileInfo.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/String.h"

/** prefix of all property names */
#define FILE_INFO_PROPERTY_PREFIX _("FILE_INFO: ")

/***************************************************************************/
void Kwave::FileInfo::PropertyTypesMap::fill()
{
    append(Kwave::INF_UNKNOWN,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	QString(), KLazyLocalizedString());

    append(Kwave::INF_ALBUM,
	FP_NONE,
	_(kli18n("Album").untranslatedText()),
	kli18n("Name of the album if the source is an album\n"
	    "that consist of more medias."));
    append(Kwave::INF_ANNOTATION,
	FP_NONE,
	_(kli18n("Annotation").untranslatedText()),
	kli18n(
	    "Provides general comments about the file or the subject of\n"
	    "the file. If the comment is several sentences long, end\n"
	    "each sentence with a period. Do not include newline\n"
	    "characters!"));
    append(Kwave::INF_ARCHIVAL,
	FP_NONE,
	_(kli18n("Archival location").untranslatedText()),
	kli18n("Indicates where the subject of the file is archived."));
    append(Kwave::INF_AUTHOR,
	FP_NONE,
	_(kli18n("Author").untranslatedText()),
	kli18n("Identifies the name of the author of the original\n"
	    "subject of the file."
	    "\nExample: 'van Beethoven, Ludwig'"));
    append(Kwave::INF_BITRATE_LOWER,
	FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Lower Bitrate").untranslatedText()),
	kli18n("Specifies the lower limit in a VBR bitstream."));
    append(Kwave::INF_BITRATE_MODE,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Bitrate Mode").untranslatedText()),
	kli18n("Bitrate Mode (ABR, VBR, CBR, etc...)"));
    append(Kwave::INF_BITRATE_NOMINAL,
	FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Bitrate").untranslatedText()),
	kli18n(
	    "Nominal bitrate of the audio stream in bits per second"));
    append(Kwave::INF_BITRATE_UPPER,
	FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Upper Bitrate").untranslatedText()),
	kli18n("Specifies the upper limit in a VBR bitstream."));
    append(Kwave::INF_BITS_PER_SAMPLE,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Bits per Sample").untranslatedText()),
	kli18n("Specifies the number of bits per sample."));
    append(Kwave::INF_CD,
	FP_FORMAT_NUMERIC,
	_(kli18n("CD").untranslatedText()),
	kli18n(
	    "Number of the CD, if the source is an album of more CDROMs"));
    append(Kwave::INF_CDS,
	FP_FORMAT_NUMERIC,
	_(kli18n("CDS").untranslatedText()),
	kli18n(
	    "Number of CDs, if the source is an album of more CDROMs"));
    append(Kwave::INF_COMMISSIONED,
	FP_NONE,
	_(kli18n("Commissioned").untranslatedText()),
	kli18n("Lists the name of the person or organization\n"
	    "that commissioned the subject of the file."));
    append(Kwave::INF_COMMENTS,
	FP_NONE,
	_(kli18n("Comments").untranslatedText()),
	kli18n(
	    "Provides general comments about the file or the subject of\n"
	    "the file. If the comment is several sentences long, end\n"
	    "each sentence with a period. Do not include newline\n"
	    "characters!"));
    append(Kwave::INF_COMPRESSION,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Compression").untranslatedText()),
	kli18n("Sets a mode for compressing the audio\n"
	    "data to reduce disk space."));
    append(Kwave::INF_CONTACT,
	FP_NONE,
	_(kli18n("Contact").untranslatedText()),
	kli18n("Contact information for the creators or distributors of\n"
	    "the track. This could be a URL, an email address, the\n"
	    "physical address of the producing label."));
    append(Kwave::INF_COPYRIGHT,
	FP_NONE,
	_(kli18n("Copyright").untranslatedText()),
	kli18n("Records the copyright information for the file. "
	    "If there are\n"
	    "multiple copyrights, separate them by a semicolon followed\n"
	    "by a space.\n"
	    "Example: 'Copyright Linux community 2002'"));
    append(Kwave::INF_COPYRIGHTED,
	FP_NONE,
	_(kli18n("Copyrighted").untranslatedText()),
	kli18n("Indicates whether the file is protected by "
		"copyright or not."));
    append(Kwave::INF_CREATION_DATE,
	FP_NONE,
	_(kli18n("Date").untranslatedText()),
	kli18n("Specifies the date the subject of the file was created.\n"
	    "Example: '2001-12-24'"));
    append(Kwave::INF_ENGINEER,
	FP_NONE,
	_(kli18n("Engineer").untranslatedText()),
	kli18n("Shows the name of the engineer who worked on the file.\n"
	    "If there are multiple engineers, separate the names by\n"
	    "a semicolon and a blank."));
    append(Kwave::INF_ESTIMATED_LENGTH,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Estimated Length").untranslatedText()),
	kli18n("Estimated length of the file in samples"));
    append(Kwave::INF_FILENAME,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Filename").untranslatedText()),
	kli18n("Name of the opened file"));
    append(Kwave::INF_FILESIZE,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("File Size").untranslatedText()),
	kli18n("Size of the file in bytes"));
    append(Kwave::INF_GENRE,
	FP_NONE,
	_(kli18n("Genre").untranslatedText()),
	kli18n("Describes the genre or style of the original work.\n"
	    "Examples: 'classic', 'pop'"));
    append(Kwave::INF_ISRC,
	FP_READONLY,
	_(kli18n("ISRC").untranslatedText()),
	kli18n("ISRC number for the track; see the ISRC intro page\n"
	    "for more information on ISRC numbers.\n"
	    "http://www.ifpi.org/site-content/online/isrc_intro.html"));
    append(Kwave::INF_KEYWORDS,
	FP_NONE,
	_(kli18n("Keywords").untranslatedText()),
	kli18n("Provides a list of keywords that refer to the\n"
	    "file or subject of the file."));
    append(Kwave::INF_LABELS,
	FP_INTERNAL,
	_(kli18n("Labels").untranslatedText()),
	kli18n("The list of labels/markers."));
    append(Kwave::INF_LENGTH,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Length").untranslatedText()),
	kli18n("Length of the file in samples."));
    append(Kwave::INF_LICENSE,
	FP_NONE,
	_(kli18n("License").untranslatedText()),
	kli18n("License information, e.g., 'All Rights Reserved',\n"
	    "'Any Use Permitted', an URL to a license or the\n"
	    "EFF Open Audio License ('distributed under the\n"
	    "terms of the Open Audio License.\n"
	    "See http://www.eff.org/IP/Open_licenses/eff_oal.html\n"
	    "for details'), etc."));
    append(Kwave::INF_MEDIUM,
	FP_NONE,
	_(kli18n("Medium").untranslatedText()),
	kli18n("Describes the original subject of the file,\n"
	    "where it was first recorded.\n"
	    "Example: 'orchestra'"));
    append(Kwave::INF_MIMETYPE,
	FP_READONLY | FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Mime Type").untranslatedText()),
	kli18n("Mime type of the file format"));
    append(Kwave::INF_MPEG_EMPHASIS,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Emphasis").untranslatedText()),
	kli18n("Audio emphasis mode"));
    append(Kwave::INF_MPEG_LAYER,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Layer").untranslatedText()),
	kli18n("MPEG Layer, I, II or III"));
    append(Kwave::INF_MPEG_MODEEXT,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Mode Extension").untranslatedText()),
	kli18n("MPEG Mode Extension (only if Joint Stereo)"));
    append(Kwave::INF_MPEG_VERSION,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Version").untranslatedText()),
	kli18n("MPEG Version, 1, 2 or 2.5"));
    append(Kwave::INF_NAME,
	FP_NONE,
	_(kli18n("Name").untranslatedText()),
	kli18n("Stores the title of the subject of the file.\n"
	    "Example: \"Symphony No.6, Op.68 'Pastoral'\""));
    append(Kwave::INF_OPUS_FRAME_LEN,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Opus Frame Length").untranslatedText()),
	kli18n("Opus Frame Length in ms (supported values are "
	            "2.5, 5, 10, 20, 40, or 60 ms)"));
    append(Kwave::INF_ORGANIZATION,
	FP_NONE,
	_(kli18n("Organization").untranslatedText()),
	kli18n("Name of the organization producing the track\n"
	    "(i.e. the 'record label')"));
    append(Kwave::INF_ORIGINAL,
	FP_NO_LOAD_SAVE,
	_(kli18n("Original").untranslatedText()),
	kli18n("Indicates whether the file is an original or a copy"));
    append(Kwave::INF_PERFORMER,
	FP_NONE,
	_(kli18n("Performer").untranslatedText()),
	kli18n("The artist(s) who performed the work. In classical\n"
	    "music this would be the conductor, orchestra, soloists.\n"
	    "In an audio book it would be the actor who did the reading."));
    append(Kwave::INF_PRIVATE,
	FP_NONE,
	_(kli18n("Private").untranslatedText()),
	kli18n("Indicates whether the subject is private"));
    append(Kwave::INF_PRODUCT,
	FP_NONE,
	_(kli18n("Product").untranslatedText()),
	kli18n("Specifies the name or the title the\n"
	    "file was originally intended for.\n"
	    "Example: 'Linux audio collection'"));
    append(Kwave::INF_SAMPLE_FORMAT,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Sample Format").untranslatedText()),
	kli18n("Format used for storing the digitized audio samples.\n"
	    "Example: '32-bit IEEE floating-point'"));
    append(Kwave::INF_SAMPLE_RATE,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Sample Rate").untranslatedText()),
	kli18n("Number of samples per second"));
    append(Kwave::INF_SOFTWARE,
	FP_NONE,
	_(kli18n("Software").untranslatedText()),
	kli18n("Identifies the name of the software package\n"
	    "used to create the file.\n"
	    "Example: 'Kwave v0.6.4-1'"));
    append(Kwave::INF_SOURCE,
	FP_NONE,
	_(kli18n("Source").untranslatedText()),
	kli18n("Identifies the name of the person or organization\n"
	    "who supplied the original subject of the file.\n"
	    "Example: 'Chaotic Sound Research'"));
    append(Kwave::INF_SOURCE_FORM,
	FP_NONE,
	_(kli18n("Source form").untranslatedText()),
	kli18n("Identifies the original form of\n"
	    "the material that was digitized.\n"
	    "Examples: 'Record/Vinyl/90RPM', 'Audio DAT', "
	    "'tape/CrO2/60min'"));
    append(Kwave::INF_SUBJECT,
	FP_NONE,
	_(kli18n("Subject").untranslatedText()),
	kli18n("Describes the subject of the file.\n"
	    "Example: 'Bird voices at early morning'"));
    append(Kwave::INF_TECHNICAN,
	FP_NONE,
	_(kli18n("Technician").untranslatedText()),
	kli18n(
	    "Identifies the technician who digitized the subject file.\n"
	    "Example: 'Torvalds, Linus'"));
    append(Kwave::INF_TRACK,
	FP_FORMAT_NUMERIC,
	_(kli18n("Track").untranslatedText()),
	kli18n("Track of the CD if the source was a CDROM."));
    append(Kwave::INF_TRACKS,
	FP_FORMAT_NUMERIC,
	_(kli18n("Tracks").untranslatedText()),
	kli18n("Number of tracks of the CD if the source was a CDROM."));
    append(Kwave::INF_CHANNELS,
	FP_INTERNAL | FP_NO_LOAD_SAVE | FP_FORMAT_NUMERIC,
	_(kli18n("Channels").untranslatedText()),
	kli18n("Specifies the number of channels of the signal."));
    append(Kwave::INF_VBR_QUALITY,
	FP_INTERNAL | FP_NO_LOAD_SAVE,
	_(kli18n("Base Quality").untranslatedText()),
	kli18n("Base quality of the compression in VBR mode"));
    append(Kwave::INF_VERSION,
	FP_NONE,
	_(kli18n("Version").untranslatedText()),
	kli18n("May be used to differentiate multiple versions\n"
	    "of the same track title in a single collection.\n"
	    "(e.g. remix info)"));

    // please do not simply extend here, sort in alphabetically instead...

}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::FileInfo::PropertyTypesMap::all() const
{
    return keys();
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
	foreach (Kwave::FileProperty key, m_property_map.keys()) {
	    QString name = FILE_INFO_PROPERTY_PREFIX +
		m_property_map.name(key);
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
    if (!m_property_map.keys().contains(key))
	return false;
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);
    return hasProperty(name);
}

/***************************************************************************/
void Kwave::FileInfo::set(Kwave::FileProperty key, const QVariant &value)
{
    if (!m_property_map.keys().contains(key))
	return;
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);
    setProperty(name, value);
}

/***************************************************************************/
QVariant Kwave::FileInfo::get(Kwave::FileProperty key) const
{
    if (!m_property_map.keys().contains(key))
	return QVariant();
    QString name = FILE_INFO_PROPERTY_PREFIX + m_property_map.name(key);

    if (!hasProperty(name)) return QVariant();
    return property(name);
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
    foreach (Kwave::FileProperty key, m_property_map.keys()) {
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
    qDebug("    id = #%s", DBG(id()));
    qDebug("default properties:");
    qDebug("   length = %lu samples", static_cast<unsigned long int>(length()));
    qDebug("   rate   = %0.1f Hz", rate());
    qDebug("   bits   = %u", bits());
    qDebug("   tracks = %u", tracks());

//     qDebug("labels:");
//     foreach (Kwave::Label label, m_labels) {
// 	qDebug("   [%10lu] = '%s'", static_cast<unsigned long int>(label.pos()),
// 	                           DBG(label.name()));
//     }

    qDebug("other properties:");
    QMap<Kwave::FileProperty, QVariant>::Iterator it;
    foreach (Kwave::FileProperty key, m_property_map.keys()) {
	if (!contains(key)) continue;
	QVariant val = get(key);
	QString name = m_property_map.name(key);
	qDebug("   '%s' = '%s'", DBG(name), DBG(val.toString()));

// 	for updating the list of properties in the appendix of the handbook:
// 	QString name  = m_property_map.name(key);
// 	QString descr = m_property_map.description(key, false);
// 	qDebug("\t    <row>");
// 	qDebug("\t\t<entry colname='c1'>&no-i18n-tag;%s</entry>", DBG(name));
// 	qDebug("\t\t<entry colname='c2'>");
// 	qDebug("\t\t    %s", DBG(descr.replace(_("\n"), _("\n\t\t    "))));
// 	qDebug("\t\t</entry>");
// 	qDebug("\t    </row>");
    }
    qDebug("-------------------------");
}

/***************************************************************************/
/***************************************************************************/
