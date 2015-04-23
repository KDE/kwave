/*************************************************************************
VorbisCommentMap.cpp  -  map for translating properties to vorbis comments
                             -------------------
    begin                : Sun May 23 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#include "libkwave/String.h"
#include "libkwave/VorbisCommentMap.h"

/** wrapper for 'insert', to handle QLatin1String conversion */
#define INS(p,d) insert(_(p), d)

//***************************************************************************
Kwave::VorbisCommentMap::VorbisCommentMap()
{
    INS("DATE",         Kwave::INF_CREATION_DATE ); // creation date (iso)
    INS("TITLE",        Kwave::INF_NAME          ); // name
    INS("VERSION",      Kwave::INF_VERSION       ); // version of the song
    INS("ALBUM",        Kwave::INF_ALBUM         ); // name of the album
    INS("TRACKNUMBER",  Kwave::INF_TRACK         ); // index of the track
    INS("ARTIST",       Kwave::INF_AUTHOR        ); // author's name
    INS("PERFORMER",    Kwave::INF_PERFORMER     ); // performer
    INS("COPYRIGHT",    Kwave::INF_COPYRIGHT     ); // copyright
    INS("LICENSE",      Kwave::INF_LICENSE       ); // name of the license
    INS("ORGANIZATION", Kwave::INF_ORGANIZATION  ); // producing organization
    INS("DESCRIPTION",  Kwave::INF_SUBJECT       ); // subject
    INS("GENRE",        Kwave::INF_GENRE         ); // genre
    INS("LOCATION",     Kwave::INF_SOURCE        ); // source
    INS("CONTACT",      Kwave::INF_CONTACT       ); // contact address(es)
    INS("ISRC",         Kwave::INF_ISRC          ); // ISRC code
    INS("ENCODER",      Kwave::INF_SOFTWARE      ); // software
    INS("ENCODED_BY",   Kwave::INF_ENGINEER      ); // name of the encoder
    INS("VBR_QUALITY",  Kwave::INF_VBR_QUALITY   ); // VBR quality
}

//***************************************************************************
QString Kwave::VorbisCommentMap::findProperty(
    const Kwave::FileProperty property)
{
    QMap<QString, Kwave::FileProperty>::Iterator it;
    for (it = begin(); it != end(); ++it) {
        if (it.value() == property) return it.key();
    }
    return QString();
}

//***************************************************************************
bool Kwave::VorbisCommentMap::containsProperty(
    const Kwave::FileProperty property)
{
    return (findProperty(property).length() != 0);
}

//***************************************************************************
//***************************************************************************
