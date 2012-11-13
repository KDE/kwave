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

#include "VorbisCommentMap.h"

//***************************************************************************
VorbisCommentMap::VorbisCommentMap()
{
    insert("DATE",         Kwave::INF_CREATION_DATE ); // creation date (iso)
    insert("TITLE",        Kwave::INF_NAME          ); // name
    insert("VERSION",      Kwave::INF_VERSION       ); // version of the song
    insert("ALBUM",        Kwave::INF_ALBUM         ); // name of the album
    insert("TRACKNUMBER",  Kwave::INF_TRACK         ); // index of the track
    insert("ARTIST",       Kwave::INF_AUTHOR        ); // author's name
    insert("PERFORMER",    Kwave::INF_PERFORMER     ); // performer
    insert("COPYRIGHT",    Kwave::INF_COPYRIGHT     ); // copyright
    insert("LICENSE",      Kwave::INF_LICENSE       ); // name of the license
    insert("ORGANIZATION", Kwave::INF_ORGANIZATION  ); // producing organization
    insert("DESCRIPTION",  Kwave::INF_SUBJECT       ); // subject
    insert("GENRE",        Kwave::INF_GENRE         ); // genre
    insert("LOCATION",     Kwave::INF_SOURCE        ); // source
    insert("CONTACT",      Kwave::INF_CONTACT       ); // contact address(es)
    insert("ISRC",         Kwave::INF_ISRC          ); // ISRC code
    insert("ENCODER",      Kwave::INF_SOFTWARE      ); // software
    insert("VBR_QUALITY",  Kwave::INF_VBR_QUALITY   ); // VBR quality
}

//***************************************************************************
QString VorbisCommentMap::findProperty(const Kwave::FileProperty property)
{
    QMap<QString, Kwave::FileProperty>::Iterator it;
    for (it=begin(); it != end(); ++it) {
        if (it.value() == property) return it.key();
    }
    return 0;
}

//***************************************************************************
bool VorbisCommentMap::containsProperty(const Kwave::FileProperty property)
{
    return (findProperty(property).length() != 0);
}

//***************************************************************************
//***************************************************************************
