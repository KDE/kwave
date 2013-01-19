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
#define _INS(p,d) insert(_(p), d)

//***************************************************************************
Kwave::VorbisCommentMap::VorbisCommentMap()
{
    _INS("DATE",         Kwave::INF_CREATION_DATE ); // creation date (iso)
    _INS("TITLE",        Kwave::INF_NAME          ); // name
    _INS("VERSION",      Kwave::INF_VERSION       ); // version of the song
    _INS("ALBUM",        Kwave::INF_ALBUM         ); // name of the album
    _INS("TRACKNUMBER",  Kwave::INF_TRACK         ); // index of the track
    _INS("ARTIST",       Kwave::INF_AUTHOR        ); // author's name
    _INS("PERFORMER",    Kwave::INF_PERFORMER     ); // performer
    _INS("COPYRIGHT",    Kwave::INF_COPYRIGHT     ); // copyright
    _INS("LICENSE",      Kwave::INF_LICENSE       ); // name of the license
    _INS("ORGANIZATION", Kwave::INF_ORGANIZATION  ); // producing organization
    _INS("DESCRIPTION",  Kwave::INF_SUBJECT       ); // subject
    _INS("GENRE",        Kwave::INF_GENRE         ); // genre
    _INS("LOCATION",     Kwave::INF_SOURCE        ); // source
    _INS("CONTACT",      Kwave::INF_CONTACT       ); // contact address(es)
    _INS("ISRC",         Kwave::INF_ISRC          ); // ISRC code
    _INS("ENCODER",      Kwave::INF_SOFTWARE      ); // software
    _INS("ENCODED_BY",   Kwave::INF_ENGINEER      ); // name of the encoder
    _INS("VBR_QUALITY",  Kwave::INF_VBR_QUALITY   ); // VBR quality
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
