/*************************************************************************
     WavPropertyMap.cpp  -  map for translating properties to chunk names
                             -------------------
    begin                : Sat Jul 06 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "WavPropertyMap.h"

//***************************************************************************
WavPropertyMap::WavPropertyMap()
{
    // NOTE #1: the left column is allowed to have multiple entries with the
    //          same property, when encoding the first one is used, when
    //          decoding, the other ones serve as alternatives
    // NOTE #2: the chunk names in the right column must be *unique* !
    // well-known tags
    insert(Kwave::INF_AUTHOR        ,"AUTH"); // author's name
    insert(Kwave::INF_ANNOTATION    ,"ANNO"); // annotations
    insert(Kwave::INF_ARCHIVAL      ,"IARL"); // archival location (EXIF 2.3)
    insert(Kwave::INF_PERFORMER     ,"IART"); // performer  (EXIF 2.3)
    insert(Kwave::INF_COMMISSIONED  ,"ICMS"); // commissioned  (EXIF 2.3)
    insert(Kwave::INF_COMMENTS      ,"ICMT"); // comments  (EXIF 2.3)
    insert(Kwave::INF_COPYRIGHT     ,"ICOP"); // copyright  (EXIF 2.3)
    insert(Kwave::INF_CREATION_DATE ,"ICRD"); // creation date (iso)  (EXIF 2.3)
    insert(Kwave::INF_ENGINEER      ,"IENG"); // engineer  (EXIF 2.3)
    insert(Kwave::INF_GENRE         ,"IGNR"); // genre  (EXIF 2.3)
    insert(Kwave::INF_KEYWORDS      ,"IKEY"); // keywords  (EXIF 2.3)
    insert(Kwave::INF_MEDIUM        ,"IMED"); // medium  (EXIF 2.3)
    insert(Kwave::INF_NAME          ,"INAM"); // name  (EXIF 2.3)
    insert(Kwave::INF_PRODUCT       ,"IPRD"); // product  (EXIF 2.3)
    insert(Kwave::INF_SOFTWARE      ,"ISFT"); // software  (EXIF 2.3)
    insert(Kwave::INF_SOURCE        ,"ISRC"); // source  (EXIF 2.3)
    insert(Kwave::INF_SOURCE_FORM   ,"ISRF"); // source form  (EXIF 2.3)
    insert(Kwave::INF_TECHNICAN     ,"ITCH"); // technican  (EXIF 2.3)
    insert(Kwave::INF_SUBJECT       ,"ISBJ"); // subject  (EXIF 2.3)

    // tags from SoundForge Pro
    insert(Kwave::INF_TRACK         ,"TRCK"); // track number
    insert(Kwave::INF_VERSION       ,"TVER"); // version/remix
    insert(Kwave::INF_ORGANIZATION  ,"TORG"); // organization/label

    // some others, alternatives
    insert(Kwave::INF_ALBUM         ,"IALB"); // name of the album
    insert(Kwave::INF_COPYRIGHT     ,"(c) "); // copyright
    insert(Kwave::INF_CREATION_DATE ,"DTIM"); // date/time original
    insert(Kwave::INF_CREATION_DATE ,"YEAR"); // year  (MovieID ref12)
    insert(Kwave::INF_GENRE         ,"GENR"); // genre (MovieID ref12)
    insert(Kwave::INF_GENRE         ,"ISGN"); // second genre (IMDB)
    insert(Kwave::INF_AUTHOR        ,"IWRI"); // written by (IMDB)
    insert(Kwave::INF_ENGINEER      ,"IEDT"); // edited by (IMDB)
    insert(Kwave::INF_CD            ,"IPTR"); // part (?)

    // non-standard, probably only known by Kwave
    insert(Kwave::INF_CONTACT       ,"cnt "); // contact information for creator
    insert(Kwave::INF_ISRC          ,"isrc"); // International Standard Recording Code
    insert(Kwave::INF_LICENSE       ,"lic "); // license information
}

//***************************************************************************
void WavPropertyMap::insert(const Kwave::FileProperty property,
                            const QByteArray &chunk)
{
    Pair p(property, chunk);
    append(p);
}

//***************************************************************************
QByteArray WavPropertyMap::findProperty(
	const Kwave::FileProperty property) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.first == property)
	    return p.second;
    }
    return "";
}

//***************************************************************************
bool WavPropertyMap::containsProperty(const Kwave::FileProperty property) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.first == property)
	    return true;
    }
    return false;
}

//***************************************************************************
bool WavPropertyMap::containsChunk(const QByteArray &chunk) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.second == chunk)
	    return true;
    }
    return false;
}

//***************************************************************************
QList<QByteArray> WavPropertyMap::chunks() const
{
    QList<QByteArray> list;
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (!list.contains(p.second))
	    list.append(p.second);
    }
    return list;
}

//***************************************************************************
Kwave::FileProperty WavPropertyMap::property(const QByteArray &chunk) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.second == chunk) return p.first;
    }
    return static_cast<Kwave::FileProperty>(-1);
}

//***************************************************************************
QList<Kwave::FileProperty> WavPropertyMap::properties() const
{
    QList<Kwave::FileProperty> list;
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (!list.contains(p.first))
	    list.append(p.first);
    }
    return list;
}

//***************************************************************************
//***************************************************************************
