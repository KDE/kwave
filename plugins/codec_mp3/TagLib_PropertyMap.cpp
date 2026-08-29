/*************************************************************************
    TagLib_PropertyMap.cpp  -  map for translating properties to ID3 frame tags
                               -------------------
    begin                  : Fri Aug 28 2026
    copyright              : (C) 2026 by Thomas Eschenbacher
    email                  : Thomas.Eschenbacher@gmx.de
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

#include "TagLib_PropertyMap.h"

//***************************************************************************
Kwave::TagLib_PropertyMap::TagLib_PropertyMap()
    :m_list()
{
    // note #1: the first column is allowed to have multiple entries with the
    //          same property, when encoding the first one is used, when
    //          decoding, the other ones serve as alternatives
    // note #2: the ID3 tag names in the second column must be *unique* !

    // album/movie/show title
    insert(Kwave::INF_ALBUM,         "TALB", ENC_TEXT);
    // original album/movie/show title
    insert(Kwave::INF_ALBUM,         "TOAL", ENC_TEXT);
    // subtitle/description refinement
    insert(Kwave::INF_ANNOTATION,    "TIT3", ENC_TEXT);
    // user defined text information
    insert(Kwave::INF_ANNOTATION,    "TXXX", ENC_TEXT);
    // original artist(s)/performer(s)
    insert(Kwave::INF_AUTHOR,        "TOPE", ENC_TEXT_SLASH);
    // involved people list
    insert(Kwave::INF_AUTHOR,        "TIPL", ENC_TEXT_LIST);
    insert(Kwave::INF_AUTHOR,        "IPLS", ENC_TEXT_LIST);
    // band/orchestra/accompaniment
    insert(Kwave::INF_AUTHOR,        "TPE2", ENC_TEXT);
    // original lyricist(s)/text writer(s)
    insert(Kwave::INF_AUTHOR,        "TOLY", ENC_TEXT_SLASH);
    // official artist/performer webpage
    insert(Kwave::INF_AUTHOR,        "WOAR", ENC_TEXT_URL);
    // official publisher webpage
    insert(Kwave::INF_AUTHOR,        "WPUB", ENC_TEXT_URL);
    // part of a set
    insert(Kwave::INF_CD,            "TPOS", ENC_TEXT_PARTINSET);
    insert(Kwave::INF_CDS,           "TPOS", ENC_TEXT_PARTINSET);
    // internet radio station name
    insert(Kwave::INF_COMMISSIONED,  "TRSN", ENC_TEXT);
    // internet radio station owner
    insert(Kwave::INF_COMMISSIONED,  "TRSO", ENC_TEXT);
    // comments
    insert(Kwave::INF_COMMENTS,      "COMM", ENC_COMMENT);
    // official audio source webpage
    insert(Kwave::INF_CONTACT,       "WOAS", ENC_TEXT_SLASH);
    // official internet radio station homepage
    insert(Kwave::INF_CONTACT,       "WORS", ENC_TEXT_SLASH);
    // official audio file webpage
    insert(Kwave::INF_CONTACT,       "WOAF", ENC_TEXT_SLASH);
    // copyright message
    insert(Kwave::INF_COPYRIGHT,     "TCOP", ENC_TEXT);
    // copyright/legal information
    insert(Kwave::INF_COPYRIGHT,     "WCOP", ENC_TEXT_URL);
    // terms of use
    insert(Kwave::INF_COPYRIGHT,     "USER", ENC_TERMS_OF_USE);

    // recording dates
    insert(Kwave::INF_CREATION_DATE, "TDRC", ENC_TEXT_TIMESTAMP);
    insert(Kwave::INF_CREATION_DATE, "TRDA", ENC_TEXT_TIMESTAMP);
    // date
    insert(Kwave::INF_CREATION_DATE, "TDAT", ENC_TEXT_TIMESTAMP);
    // year
    insert(Kwave::INF_CREATION_DATE, "TYER", ENC_TEXT_TIMESTAMP);
    // time
    insert(Kwave::INF_CREATION_DATE, "TIME", ENC_TEXT_TIMESTAMP);
    // original release year
    insert(Kwave::INF_CREATION_DATE, "TORY", ENC_TEXT_TIMESTAMP);

    // content type (genre)
    insert(Kwave::INF_GENRE,         "TCON", ENC_GENRE_TYPE);
    // id3 tags (custom/binary)
    insert(Kwave::INF_ID3,           "",     ENC_BINARY);
    insert(Kwave::INF_ISRC,          "TSRC", ENC_TEXT);
    // length
    insert(Kwave::INF_LENGTH,        "TLEN", ENC_LENGTH);
    // file owner/licensee
    insert(Kwave::INF_LICENSE,       "TOWN", ENC_TEXT);
    // medium type
    insert(Kwave::INF_MEDIUM,        "TMED", ENC_TEXT);
    // title/songname/content description
    insert(Kwave::INF_NAME,          "TIT2", ENC_TEXT);
    // composer
    insert(Kwave::INF_ORGANIZATION,  "TCOM", ENC_TEXT_SLASH);
    // publisher
    insert(Kwave::INF_ORGANIZATION,  "TPUB", ENC_TEXT_SLASH);
    // produced notice
    insert(Kwave::INF_ORGANIZATION,  "TPRO", ENC_TEXT_SLASH);
    // lyricist/text writer
    insert(Kwave::INF_PERFORMER,     "TEXT", ENC_TEXT_SLASH);
    // lead performer(s)/soloist(s)
    insert(Kwave::INF_PERFORMER,     "TPE1", ENC_TEXT_SLASH);
    // musician credits list
    insert(Kwave::INF_PERFORMER,     "TMCL", ENC_TEXT_SLASH);
    // conductor/performer refinement
    insert(Kwave::INF_PERFORMER,     "TPE3", ENC_TEXT);
    // software
    insert(Kwave::INF_SOFTWARE,      "TSSE", ENC_TEXT);
    // encoded by
    insert(Kwave::INF_TECHNICAN,     "TENC", ENC_TEXT);
    // track number/position in set
    insert(Kwave::INF_TRACK,         "TRCK", ENC_TRACK_NUM);
    // number of tracks
    insert(Kwave::INF_TRACKS,        "TRCK", ENC_TRACK_NUM);
    // interpreted, remixed / modified by
    insert(Kwave::INF_VERSION,       "TPE4", ENC_TEXT);
    // set subtitle
    insert(Kwave::INF_VERSION,       "TSST", ENC_TEXT);

    insert(Kwave::INF_UNKNOWN,       "",     ENC_NONE);
}

//***************************************************************************
void Kwave::TagLib_PropertyMap::insert(const Kwave::FileProperty property,
    const TagLib::ByteVector &id,
    const Kwave::TagLib_PropertyMap::Encoding encoding)
{
    Kwave::TagLib_PropertyMap::Mapping mapping;

    mapping.m_property = property;
    mapping.m_frame_id = id;
    mapping.m_encoding = encoding;
    m_list.append(mapping);
}

//***************************************************************************
TagLib::ByteVector Kwave::TagLib_PropertyMap::findProperty(
    const Kwave::FileProperty property) const
{
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if ((m.m_property == property) && supported(m.m_frame_id))
            return m.m_frame_id;

    return TagLib::ByteVector();
}

//***************************************************************************
bool Kwave::TagLib_PropertyMap::containsProperty(
        const Kwave::FileProperty property) const
{
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if ((m.m_property == property) && supported(m.m_frame_id))
            return true;

    return false;
}

//***************************************************************************
bool Kwave::TagLib_PropertyMap::containsID(const TagLib::ByteVector &id) const
{
    if (!supported(id))
        return false;

    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if (m.m_frame_id == id)
            return true;

    return false;
}

//***************************************************************************
Kwave::TagLib_PropertyMap::Encoding Kwave::TagLib_PropertyMap::encoding(
    const TagLib::ByteVector &id) const
{
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if (m.m_frame_id == id)
            return m.m_encoding;

    return TagLib_PropertyMap::ENC_NONE;
}

//***************************************************************************
QList<TagLib::ByteVector> Kwave::TagLib_PropertyMap::knownIDs() const
{
    QList<TagLib::ByteVector> list;
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if (!list.contains(m.m_frame_id))
            list.append(m.m_frame_id);

    return list;
}

//***************************************************************************
Kwave::FileProperty Kwave::TagLib_PropertyMap::property(
    const TagLib::ByteVector &id) const
{
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if (m.m_frame_id == id)
            return m.m_property;

    return Kwave::INF_UNKNOWN;
}

//***************************************************************************
QList<Kwave::FileProperty> Kwave::TagLib_PropertyMap::properties() const
{
    QList<Kwave::FileProperty> list;
    for (const Kwave::TagLib_PropertyMap::Mapping &m : m_list)
        if (!list.contains(m.m_property))
            list.append(m.m_property);

    return list;
}

//***************************************************************************
bool Kwave::TagLib_PropertyMap::supported(const TagLib::ByteVector &id) const
{
    return !id.isEmpty();
}

//***************************************************************************
//***************************************************************************
