/*************************************************************************
            GenreType.h  -  Map for genre types
                             -------------------
    begin                : Wed Sep 01 2002
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

#include "config.h"
#include <klocale.h>
#include "GenreType.h"

//***************************************************************************
QMap<int, QString> GenreType::m_map;

//***************************************************************************
GenreType::GenreType()
{
}

//***************************************************************************
QString GenreType::name(int id)
{
    fill();
    if (!m_map.contains(id)) id = -1;
    return m_map[id];
}

//***************************************************************************
int GenreType::fromID3(const QString &tag)
{
    fill();
    if (tag.startsWith("(") && tag.endsWith(")")) {
	QString s = tag.mid(1, tag.length()-2);
	return s.toInt();
    } else {
	return id(tag);
    }
}

//***************************************************************************
int GenreType::id(const QString &name)
{
    fill();
    QMap<int, QString>::Iterator it;
    for (it=m_map.begin(); it != m_map.end(); ++it) {
	if (it.value() == name) return it.key();
    }
    return -1;
}


//***************************************************************************
void GenreType::fill()
{
    if (m_map.count()) return;
    {

// replace i18n with a macro, i18n sometimes makes trouble when
// called in static initializers
#ifdef i18n
#undef i18n
#endif
#define i18n(x) x

	static const char *map[] = {
	    // The following genres is defined in ID3v1
	    i18n("Blues"),
	    i18n("Classic Rock"),
	    i18n("Country"),
	    i18n("Dance"),
	    i18n("Disco"),
	    i18n("Funk"),
	    i18n("Grunge"),
	    i18n("Hip-Hop"),
	    i18n("Jazz"),
	    i18n("Metal"),
	    i18n("New Age"),
	    i18n("Oldies"),
	    i18n("Other"),
	    i18n("Pop"),
	    i18n("R&B"),
	    i18n("Rap"),
	    i18n("Reggae"),
	    i18n("Rock"),
	    i18n("Techno"),
	    i18n("Industrial"),
	    i18n("Alternative"),
	    i18n("Ska"),
	    i18n("Death Metal"),
	    i18n("Pranks"),
	    i18n("Soundtrack"),
	    i18n("Euro-Techno"),
	    i18n("Ambient"),
	    i18n("Trip-Hop"),
	    i18n("Vocal"),
	    i18n("Jazz+Funk"),
	    i18n("Fusion"),
	    i18n("Trance"),
	    i18n("Classical"),
	    i18n("Instrumental"),
	    i18n("Acid"),
	    i18n("House"),
	    i18n("Game"),
	    i18n("Sound Clip"),
	    i18n("Gospel"),
	    i18n("Noise"),
	    i18n("AlternRock"),
	    i18n("Bass"),
	    i18n("Soul"),
	    i18n("Punk"),
	    i18n("Space"),
	    i18n("Meditative"),
	    i18n("Instrumental Pop"),
	    i18n("Instrumental Rock"),
	    i18n("Ethnic"),
	    i18n("Gothic"),
	    i18n("Darkwave"),
	    i18n("Techno-Industrial"),
	    i18n("Electronic"),
	    i18n("Pop-Folk"),
	    i18n("Eurodance"),
	    i18n("Dream"),
	    i18n("Southern Rock"),
	    i18n("Comedy"),
	    i18n("Cult"),
	    i18n("Gangsta"),
	    i18n("Top 40"),
	    i18n("Christian Rap"),
	    i18n("Pop/Funk"),
	    i18n("Jungle"),
	    i18n("Native American"),
	    i18n("Cabaret"),
	    i18n("New Wave"),
	    i18n("Psychadelic"),
	    i18n("Rave"),
	    i18n("Showtunes"),
	    i18n("Trailer"),
	    i18n("Lo-Fi"),
	    i18n("Tribal"),
	    i18n("Acid Punk"),
	    i18n("Acid Jazz"),
	    i18n("Polka"),
	    i18n("Retro"),
	    i18n("Musical"),
	    i18n("Rock & Roll"),
	    i18n("Hard Rock"),

	    // The following genres are Winamp extensions
	    i18n("Folk"),
	    i18n("Folk-Rock"),
	    i18n("National Folk"),
	    i18n("Swing"),
	    i18n("Fast Fusion"),
	    i18n("Bebob"),
	    i18n("Latin"),
	    i18n("Revival"),
	    i18n("Celtic"),
	    i18n("Bluegrass"),
	    i18n("Avantgarde"),
	    i18n("Gothic Rock"),
	    i18n("Progressive Rock"),
	    i18n("Psychedelic Rock"),
	    i18n("Symphonic Rock"),
	    i18n("Slow Rock"),
	    i18n("Big Band"),
	    i18n("Chorus"),
	    i18n("Easy Listening"),
	    i18n("Acoustic"),
	    i18n("Humour"),
	    i18n("Speech"),
	    i18n("Chanson"),
	    i18n("Opera"),
	    i18n("Chamber Music"),
	    i18n("Sonata"),
	    i18n("Symphony"),
	    i18n("Booty Bass"),
	    i18n("Primus"),
	    i18n("Porn Groove"),
	    i18n("Satire"),
	    i18n("Slow Jam"),
	    i18n("Club"),
	    i18n("Tango"),
	    i18n("Samba"),
	    i18n("Folklore"),
	    i18n("Ballad"),
	    i18n("Power Ballad"),
	    i18n("Rhythmic Soul"),
	    i18n("Freestyle"),
	    i18n("Duet"),
	    i18n("Punk Rock"),
	    i18n("Drum Solo"),
	    i18n("Acapella"),
	    i18n("Euro-House"),
	    i18n("Dance Hall")
	};

// now use normal i18n
#undef i18n

	for (unsigned int i=0; i < sizeof(map)/sizeof(map[0]); ++i)
	    m_map.insert(i, i18n(map[i]));

	m_map.insert(     -1, i18n("Unknown"));
    }

}

//***************************************************************************
//***************************************************************************
