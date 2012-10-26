/*************************************************************************
    ID3_PropertyMap.cpp  -  map for translating properties to ID3 frame tags
                             -------------------
    begin                : Sat Jul 30 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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

#include <id3/field.h>

#include "ID3_PropertyMap.h"

//***************************************************************************
Kwave::ID3_PropertyMap::ID3_PropertyMap()
    :m_list()
{
    // NOTE #1: the left column is allowed to have multiple entries with the
    //          same property, when encoding the first one is used, when
    //          decoding, the other ones serve as alternatives
    // NOTE #2: the ID3 tag names in the right column must be *unique* !

    // Album/Movie/Show title
    insert(INF_ALBUM,         ID3FID_ALBUM              , ENC_TEXT);
    // Original album/movie/show title
    insert(INF_ALBUM,         ID3FID_ORIGALBUM          , ENC_TEXT);
    // Subtitle/Description refinement
    insert(INF_ANNOTATION,    ID3FID_SUBTITLE           , ENC_TEXT);
    // User defined text information
    insert(INF_ANNOTATION,    ID3FID_USERTEXT           , ENC_TEXT);
    //     INF_ARCHIVAL
    // Original artist(s)/performer(s)
    insert(INF_AUTHOR,        ID3FID_ORIGARTIST         , ENC_TEXT_SLASH);
    // Involved people list
    insert(INF_AUTHOR,        ID3FID_INVOLVEDPEOPLE     , ENC_TEXT_LIST);
    // Involved people list
    insert(INF_AUTHOR,        ID3FID_INVOLVEDPEOPLE2    , ENC_TEXT_LIST);
    // Band/orchestra/accompaniment
    insert(INF_AUTHOR,        ID3FID_BAND               , ENC_TEXT);
    // Original lyricist(s)/text writer(s)
    insert(INF_AUTHOR,        ID3FID_ORIGLYRICIST       , ENC_TEXT_SLASH);
    // Official artist/performer webpage
    insert(INF_AUTHOR,        ID3FID_WWWARTIST          , ENC_TEXT_URL);
    // Official publisher webpage
    insert(INF_AUTHOR,        ID3FID_WWWPUBLISHER       , ENC_TEXT_URL);
    //     INF_BITRATE_LOWER
    //     INF_BITRATE_NOMINAL
    //     INF_BITRATE_UPPER
    //     INF_BITS_PER_SAMPLE
    // Part of a set
    insert(INF_CD,            ID3FID_PARTINSET          , ENC_TEXT_PARTINSET);
    insert(INF_CDS,           ID3FID_PARTINSET          , ENC_TEXT_PARTINSET);
    // Internet radio station name
    insert(INF_COMMISSIONED,  ID3FID_NETRADIOSTATION    , ENC_TEXT);
    // Internet radio station owner
    insert(INF_COMMISSIONED,  ID3FID_NETRADIOOWNER      , ENC_TEXT);
    // Comments
    insert(INF_COMMENTS,      ID3FID_COMMENT            , ENC_COMMENT);
    // Official audio source webpage
    insert(INF_CONTACT,       ID3FID_WWWAUDIOSOURCE     , ENC_TEXT_SLASH);
    // Official internet radio station homepage
    insert(INF_CONTACT,       ID3FID_WWWRADIOPAGE       , ENC_TEXT_SLASH);
    // Official audio file webpage
    insert(INF_CONTACT,       ID3FID_WWWAUDIOFILE       , ENC_TEXT_SLASH);
    // Ownership (not supported by id3lib)
    //    (INF_CONTACT,       ID3FID_OWNERSHIP          , ENC_OWNERSHIP?);
    // Copyright message.
    insert(INF_COPYRIGHT,     ID3FID_COPYRIGHT          , ENC_TEXT);
    // Copyright/Legal infromation
    insert(INF_COPYRIGHT,     ID3FID_WWWCOPYRIGHT       , ENC_TEXT_URL);
    // Terms of use
    insert(INF_COPYRIGHT,     ID3FID_TERMSOFUSE         , ENC_TERMS_OF_USE);
    //     INF_COPYRIGHTED

    // Recording dates
    insert(INF_CREATION_DATE, ID3FID_RECORDINGDATES     , ENC_TEXT_TIMESTAMP);
    // Recording time (not supported by id3lib)
    //    (INF_CREATION_DATE, ID3FID_RECORDINGTIME      , ENC_TEXT_TIMESTAMP);
    // Date
    insert(INF_CREATION_DATE, ID3FID_DATE               , ENC_TEXT_TIMESTAMP);
    // Year
    insert(INF_CREATION_DATE, ID3FID_YEAR               , ENC_TEXT_TIMESTAMP);
    // Time
    insert(INF_CREATION_DATE, ID3FID_TIME               , ENC_TEXT_TIMESTAMP);
    // Release time (not supported by id3lib)
    //    (INF_CREATION_DATE, ID3FID_RELEASETIME        , ENC_TEXT_TIMESTAMP);
    // Original release year
    insert(INF_CREATION_DATE, ID3FID_ORIGYEAR           , ENC_TEXT_TIMESTAMP);
    // Original release time (not supported by id3lib)
    //    (INF_CREATION_DATE, ID3FID_ORIGRELEASETIME    , ENC_TEXT_TIMESTAMP);
    // Tagging time (not supported by id3lib)
    //    (INF_CREATION_DATE, ID3FID_TAGGINGTIME        , ENC_TEXT_TIMESTAMP);
    // Encoding time (not supported by id3lib)
    //    (INF_CREATION_DATE, ID3FID_ENCODINGTIME       , ENC_TEXT_TIMESTAMP);

    //     INF_ENGINEER
    //     INF_FILENAME
    //     INF_FILESIZE
    // Content type (Genre)
    insert(INF_GENRE,         ID3FID_CONTENTTYPE        , ENC_GENRE_TYPE);
    // ISRC
    insert(INF_ISRC,          ID3FID_ISRC               , ENC_TEXT);
    //     INF_KEYWORDS
    //     INF_LABELS
    // Length
    insert(INF_LENGTH,	      ID3FID_SONGLEN            , ENC_LENGTH);
    // File owner/licensee
    insert(INF_LICENSE,       ID3FID_FILEOWNER          , ENC_TEXT);
    // Medium type
    insert(INF_MEDIUM,        ID3FID_MEDIATYPE          , ENC_TEXT); // ### TODO ### TMED
    //     INF_MIMETYPE
    //     INF_MPEG_EMPHASIS
    //     INF_MPEG_LAYER
    //     INF_MPEG_MODEEXT
    //     INF_MPEG_VERSION
    // Title/songname/content description
    insert(INF_NAME,          ID3FID_TITLE              , ENC_TEXT);
    // Composer
    insert(INF_ORGANIZATION,  ID3FID_COMPOSER           , ENC_TEXT_SLASH);
    // Publisher
    insert(INF_ORGANIZATION,  ID3FID_PUBLISHER          , ENC_TEXT_SLASH);
    // Produced notice
    insert(INF_ORGANIZATION,  ID3FID_PRODUCEDNOTICE     , ENC_TEXT_SLASH);
    //     INF_ORIGINAL
    // Lyricist/Text writer
    insert(INF_PERFORMER,     ID3FID_LYRICIST           , ENC_TEXT_SLASH);
    // Lead performer(s)/Soloist(s).
    insert(INF_PERFORMER,     ID3FID_LEADARTIST         , ENC_TEXT_SLASH);
    // Musician credits list
    insert(INF_PERFORMER,     ID3FID_MUSICIANCREDITLIST , ENC_TEXT_SLASH);
    // Conductor/performer refinement
    insert(INF_PERFORMER,     ID3FID_CONDUCTOR          , ENC_TEXT);
    // Private frame
    //     INF_PRIVATE
    //     INF_PRODUCT
    //     INF_SAMPLE_FORMAT
    //     INF_SAMPLE_RATE
    //     INF_SOFTWARE
    //     INF_SOURCE
    //     INF_SOURCE_FORM
    //     INF_SUBJECT
    // Encoded by.
    insert(INF_TECHNICAN,     ID3FID_ENCODEDBY          , ENC_TEXT);
    // Track number/Position in set
    insert(INF_TRACK,         ID3FID_TRACKNUM           , ENC_TRACK_NUM);
    // Number of Tracks
    insert(INF_TRACKS,        ID3FID_TRACKNUM           , ENC_TRACK_NUM);
    //     INF_VBR_QUALITY
    // Interpreted, remixed / modified by
    insert(INF_VERSION,       ID3FID_MIXARTIST          , ENC_TEXT);
    // Set subtitle
    insert(INF_VERSION,       ID3FID_SETSUBTITLE        , ENC_TEXT);
    //                        ID3FID_PRIVATE => user defined data

    insert(INF_UNKNOWN,       ID3FID_NOFRAME            , ENC_NONE);
}

//***************************************************************************
void Kwave::ID3_PropertyMap::insert(const FileProperty property,
    const ID3_FrameID id, const Kwave::ID3_PropertyMap::Encoding encoding)
{
    Kwave::ID3_PropertyMap::Mapping mapping;

    mapping.m_property = property;
    mapping.m_frame_id = id;
    mapping.m_encoding = encoding;
    m_list.append(mapping);
}

//***************************************************************************
ID3_FrameID Kwave::ID3_PropertyMap::findProperty(
    const FileProperty property) const
{
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if ((m.m_property == property) && supported(m.m_frame_id))
	    return m.m_frame_id;
    }
    return ID3FID_NOFRAME;
}

//***************************************************************************
bool Kwave::ID3_PropertyMap::containsProperty(const FileProperty property) const
{
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if ((m.m_property == property) && supported(m.m_frame_id))
	    return true;
    }
    return false;
}

//***************************************************************************
bool Kwave::ID3_PropertyMap::containsID(const ID3_FrameID id) const
{
    if (!supported(id))
	return false;

    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if (m.m_frame_id == id)
	    return true;
    }
    return false;
}

//***************************************************************************
Kwave::ID3_PropertyMap::Encoding Kwave::ID3_PropertyMap::encoding(
    const ID3_FrameID id) const
{
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if (m.m_frame_id == id)
	    return m.m_encoding;
    }
    return ID3_PropertyMap::ENC_NONE;
}

//***************************************************************************
QList<ID3_FrameID> Kwave::ID3_PropertyMap::knownIDs() const
{
    QList<ID3_FrameID> list;
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if (!list.contains(m.m_frame_id))
	    list.append(m.m_frame_id);
    }
    return list;
}

//***************************************************************************
FileProperty Kwave::ID3_PropertyMap::property(const ID3_FrameID id) const
{
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if (m.m_frame_id == id) return m.m_property;
    }
    return INF_UNKNOWN;
}

//***************************************************************************
QList<FileProperty> Kwave::ID3_PropertyMap::properties() const
{
    QList<FileProperty> list;
    foreach(const Kwave::ID3_PropertyMap::Mapping &m, m_list) {
	if (!list.contains(m.m_property))
	    list.append(m.m_property);
    }
    return list;
}

//***************************************************************************
bool Kwave::ID3_PropertyMap::supported(const ID3_FrameID id) const
{
    ID3_FrameInfo frameInfo;
    return (frameInfo.NumFields(id) != 0);
}

//***************************************************************************
//***************************************************************************
