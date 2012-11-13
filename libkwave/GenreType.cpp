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
QMap<int, const char *> Kwave::GenreType::m_map;

//***************************************************************************
Kwave::GenreType::GenreType()
{
}

//***************************************************************************
QString Kwave::GenreType::name(int id, bool localized)
{
    fill();

    if (m_map.contains(id))
	return QString((localized) ? i18n(m_map[id]) : m_map[id]);
    else
	return QString::number(id);
}

//***************************************************************************
int Kwave::GenreType::fromID3(const QString &tag)
{
    fill();

    QString s = tag;

    // remove brackets (optional)
    if (s.startsWith("(") && s.endsWith(")"))
	s = s.mid(1, tag.length()-2);

    bool ok = false;
    int i = s.toInt(&ok);
    if ((i < 0) || (i >= m_map.count())) ok = false;

    return (ok) ? i : id(tag);
}

//***************************************************************************
int Kwave::GenreType::id(const QString &name)
{
    fill();
    QMap<int, const char *>::Iterator it;
    for (it = m_map.begin(); it != m_map.end(); ++it) {
	if (QString(it.value()).compare(name, Qt::CaseInsensitive) == 0)
	    return it.key();
	if (i18n(it.value()).compare(name, Qt::CaseInsensitive) == 0)
	    return it.key();
    }

    return -1;
}

//***************************************************************************
QStringList Kwave::GenreType::allTypes()
{
    fill();

    QStringList list;
    foreach (const char *item, m_map.values())
	list.append(i18n(item));

    return list;
}

//***************************************************************************
void Kwave::GenreType::fill()
{
    if (m_map.count()) return;
    {
	static const char *map[] = {
	    // The following genres is defined in ID3v1
	    I18N_NOOP("Blues"),
	    I18N_NOOP("Classic Rock"),
	    I18N_NOOP("Country"),
	    I18N_NOOP("Dance"),
	    I18N_NOOP("Disco"),
	    I18N_NOOP("Funk"),
	    I18N_NOOP("Grunge"),
	    I18N_NOOP("Hip-Hop"),
	    I18N_NOOP("Jazz"),
	    I18N_NOOP("Metal"),
	    I18N_NOOP("New Age"),
	    I18N_NOOP("Oldies"),
	    I18N_NOOP("Other"),
	    I18N_NOOP("Pop"),
	    I18N_NOOP("R&B"),
	    I18N_NOOP("Rap"),
	    I18N_NOOP("Reggae"),
	    I18N_NOOP("Rock"),
	    I18N_NOOP("Techno"),
	    I18N_NOOP("Industrial"),
	    I18N_NOOP("Alternative"),
	    I18N_NOOP("Ska"),
	    I18N_NOOP("Death Metal"),
	    I18N_NOOP("Pranks"),
	    I18N_NOOP("Soundtrack"),
	    I18N_NOOP("Euro-Techno"),
	    I18N_NOOP("Ambient"),
	    I18N_NOOP("Trip-Hop"),
	    I18N_NOOP("Vocal"),
	    I18N_NOOP("Jazz+Funk"),
	    I18N_NOOP("Fusion"),
	    I18N_NOOP("Trance"),
	    I18N_NOOP("Classical"),
	    I18N_NOOP("Instrumental"),
	    I18N_NOOP("Acid"),
	    I18N_NOOP("House"),
	    I18N_NOOP("Game"),
	    I18N_NOOP("Sound Clip"),
	    I18N_NOOP("Gospel"),
	    I18N_NOOP("Noise"),
	    I18N_NOOP("AlternRock"),
	    I18N_NOOP("Bass"),
	    I18N_NOOP("Soul"),
	    I18N_NOOP("Punk"),
	    I18N_NOOP("Space"),
	    I18N_NOOP("Meditative"),
	    I18N_NOOP("Instrumental Pop"),
	    I18N_NOOP("Instrumental Rock"),
	    I18N_NOOP("Ethnic"),
	    I18N_NOOP("Gothic"),
	    I18N_NOOP("Darkwave"),
	    I18N_NOOP("Techno-Industrial"),
	    I18N_NOOP("Electronic"),
	    I18N_NOOP("Pop-Folk"),
	    I18N_NOOP("Eurodance"),
	    I18N_NOOP("Dream"),
	    I18N_NOOP("Southern Rock"),
	    I18N_NOOP("Comedy"),
	    I18N_NOOP("Cult"),
	    I18N_NOOP("Gangsta"),
	    I18N_NOOP("Top 40"),
	    I18N_NOOP("Christian Rap"),
	    I18N_NOOP("Pop/Funk"),
	    I18N_NOOP("Jungle"),
	    I18N_NOOP("Native American"),
	    I18N_NOOP("Cabaret"),
	    I18N_NOOP("New Wave"),
	    I18N_NOOP("Psychedelic"),
	    I18N_NOOP("Rave"),
	    I18N_NOOP("Showtunes"),
	    I18N_NOOP("Trailer"),
	    I18N_NOOP("Lo-Fi"),
	    I18N_NOOP("Tribal"),
	    I18N_NOOP("Acid Punk"),
	    I18N_NOOP("Acid Jazz"),
	    I18N_NOOP("Polka"),
	    I18N_NOOP("Retro"),
	    I18N_NOOP("Musical"),
	    I18N_NOOP("Rock & Roll"),
	    I18N_NOOP("Hard Rock"),

	    // The following genres are Winamp extensions
	    I18N_NOOP("Folk"),
	    I18N_NOOP("Folk-Rock"),
	    I18N_NOOP("National Folk"),
	    I18N_NOOP("Swing"),
	    I18N_NOOP("Fast Fusion"),
	    I18N_NOOP("Bebob"),
	    I18N_NOOP("Latin"),
	    I18N_NOOP("Revival"),
	    I18N_NOOP("Celtic"),
	    I18N_NOOP("Bluegrass"),
	    I18N_NOOP("Avantgarde"),
	    I18N_NOOP("Gothic Rock"),
	    I18N_NOOP("Progressive Rock"),
	    I18N_NOOP("Psychedelic Rock"),
	    I18N_NOOP("Symphonic Rock"),
	    I18N_NOOP("Slow Rock"),
	    I18N_NOOP("Big Band"),
	    I18N_NOOP("Chorus"),
	    I18N_NOOP("Easy Listening"),
	    I18N_NOOP("Acoustic"),
	    I18N_NOOP("Humour"),
	    I18N_NOOP("Speech"),
	    I18N_NOOP("Chanson"),
	    I18N_NOOP("Opera"),
	    I18N_NOOP("Chamber Music"),
	    I18N_NOOP("Sonata"),
	    I18N_NOOP("Symphony"),
	    I18N_NOOP("Booty Bass"),
	    I18N_NOOP("Primus"),
	    I18N_NOOP("Porn Groove"),
	    I18N_NOOP("Satire"),
	    I18N_NOOP("Slow Jam"),
	    I18N_NOOP("Club"),
	    I18N_NOOP("Tango"),
	    I18N_NOOP("Samba"),
	    I18N_NOOP("Folklore"),
	    I18N_NOOP("Ballad"),
	    I18N_NOOP("Power Ballad"),
	    I18N_NOOP("Rhythmic Soul"),
	    I18N_NOOP("Freestyle"),
	    I18N_NOOP("Duet"),
	    I18N_NOOP("Punk Rock"),
	    I18N_NOOP("Drum Solo"),
	    I18N_NOOP("Acapella"),
	    I18N_NOOP("Euro-House"),
	    I18N_NOOP("Dance Hall")
	};

	for (unsigned int i = 0; i < sizeof(map) / sizeof(map[0]); ++i)
	    m_map.insert(i, map[i]);

	m_map.insert(     -1, I18N_NOOP("Unknown"));
    }

}

//***************************************************************************
//***************************************************************************
