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
    // well-known tags:
    insert(INF_AUTHOR        ,"AUTH"); // author's name
    insert(INF_ANNOTATION    ,"ANNO"); // annotations
    insert(INF_ARCHIVAL      ,"IARL"); // archival location
    insert(INF_PERFORMER     ,"IART"); // performer
    insert(INF_COMMISSIONED  ,"ICMS"); // commissioned
    insert(INF_COMMENTS      ,"ICMT"); // comments
    insert(INF_COPYRIGHT     ,"ICOP"); // copyright
    insert(INF_COPYRIGHT     ,"(c) "); // copyright
    insert(INF_CREATION_DATE ,"ICRD"); // creation date (iso)
    insert(INF_ENGINEER      ,"IENG"); // engineer
    insert(INF_GENRE         ,"IGNR"); // genre
    insert(INF_KEYWORDS      ,"IKEY"); // keywords
    insert(INF_MEDIUM        ,"IMED"); // medium
    insert(INF_NAME          ,"INAM"); // name
    insert(INF_ALBUM         ,"IPRD"); // album
    insert(INF_PRODUCT       ,"IPRD"); // product (alternative)
    insert(INF_SOFTWARE      ,"ISFT"); // software
    insert(INF_SOURCE        ,"ISRC"); // source
    insert(INF_SOURCE_FORM   ,"ISRF"); // source form
    insert(INF_TECHNICAN     ,"ITCH"); // technican
    insert(INF_SUBJECT       ,"ISBJ"); // subject
    insert(INF_TRACK         ,"ISBJ"); // track number (alternative)
}

//***************************************************************************
void WavPropertyMap::insert(const FileProperty property,
                            const QByteArray &chunk)
{
    Pair p(property, chunk);
    append(p);
}

//***************************************************************************
QByteArray WavPropertyMap::findProperty(const FileProperty property) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.first == property)
	    return p.second;
    }
    return "";
}

//***************************************************************************
bool WavPropertyMap::containsProperty(const FileProperty property) const
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
FileProperty WavPropertyMap::property(const QByteArray &chunk) const
{
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (p.second == chunk) return p.first;
    }
    return static_cast<FileProperty>(-1);
}

//***************************************************************************
QList<FileProperty> WavPropertyMap::properties() const
{
    QList<FileProperty> list;
    foreach(const Pair &p, QList<Pair>(*this)) {
	if (!list.contains(p.first))
	    list.append(p.first);
    }
    return list;
}

//***************************************************************************
//***************************************************************************
