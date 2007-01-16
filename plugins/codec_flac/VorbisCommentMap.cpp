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
    insert("DATE",         INF_CREATION_DATE ); // creation date (iso)
    insert("TITLE",        INF_NAME          ); // name
    insert("VERSION",      INF_VERSION       ); // version of the song
    insert("ALBUM",        INF_ALBUM         ); // name of the album
    insert("TRACKNUMBER",  INF_TRACK         ); // index of the track
    insert("ARTIST",       INF_AUTHOR        ); // author's name
    insert("PERFORMER",    INF_PERFORMER     ); // performer
    insert("COPYRIGHT",    INF_COPYRIGHT     ); // copyright
    insert("LICENSE",      INF_LICENSE       ); // name of the license
    insert("ORGANIZATION", INF_ORGANIZATION  ); // producing organization
    insert("DESCRIPTION",  INF_SUBJECT       ); // subject
    insert("GENRE",        INF_GENRE         ); // genre
    insert("LOCATION",     INF_SOURCE        ); // source
    insert("CONTACT",      INF_CONTACT       ); // contact address(es)
    insert("ISRC",         INF_ISRC          ); // ISRC code
    insert("ENCODER",      INF_SOFTWARE      ); // software
    insert("VBR_QUALITY",  INF_VBR_QUALITY   ); // VBR quality
}

//***************************************************************************
QString VorbisCommentMap::findProperty(const FileProperty property)
{
    QMap<QString, FileProperty>::Iterator it;
    for (it=begin(); it != end(); ++it) {
        if (it.data() == property) return it.key();
    }
    return 0;
}

//***************************************************************************
bool VorbisCommentMap::containsProperty(const FileProperty property)
{
    return (findProperty(property).length() != 0);
}

//***************************************************************************
//***************************************************************************
