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

#include <KLocalizedString>

#include "libkwave/GenreType.h"
#include "libkwave/String.h"

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
	return (localized) ? i18n(m_map[id]) : _(m_map[id]);
    else
	return QString::number(id);
}

//***************************************************************************
int Kwave::GenreType::fromID3(const QString &tag)
{
    fill();

    QString s = tag;

    // remove brackets (optional)
    if (s.startsWith(_("(")) && s.endsWith(_(")")))
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
	if (!_(it.value()).compare(name, Qt::CaseInsensitive))
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
	    // The following genres are defined in ID3v1
	    I18N_NOOP("Blues"),            //   0
	    I18N_NOOP("Classic Rock"),
	    I18N_NOOP("Country"),
	    I18N_NOOP("Dance"),
	    I18N_NOOP("Disco"),
	    I18N_NOOP("Funk"),             //   5
	    I18N_NOOP("Grunge"),
	    I18N_NOOP("Hip-Hop"),
	    I18N_NOOP("Jazz"),
	    I18N_NOOP("Metal"),
	    I18N_NOOP("New Age"),          //  10
	    I18N_NOOP("Oldies"),
	    I18N_NOOP("Other"),
	    I18N_NOOP("Pop"),
	    I18N_NOOP("Rhythm and Blues"),
	    I18N_NOOP("Rap"),              //  15
	    I18N_NOOP("Reggae"),
	    I18N_NOOP("Rock"),
	    I18N_NOOP("Techno"),
	    I18N_NOOP("Industrial"),
	    I18N_NOOP("Alternative"),      //  20
	    I18N_NOOP("Ska"),
	    I18N_NOOP("Death Metal"),
	    I18N_NOOP("Pranks"),
	    I18N_NOOP("Soundtrack"),
	    I18N_NOOP("Euro-Techno"),      //  25
	    I18N_NOOP("Ambient"),
	    I18N_NOOP("Trip-Hop"),
	    I18N_NOOP("Vocal"),
	    I18N_NOOP("Jazz & Funk"),
	    I18N_NOOP("Fusion"),           //  30
	    I18N_NOOP("Trance"),
	    I18N_NOOP("Classical"),
	    I18N_NOOP("Instrumental"),
	    I18N_NOOP("Acid"),
	    I18N_NOOP("House"),            //  35
	    I18N_NOOP("Game"),
	    I18N_NOOP("Sound Clip"),
	    I18N_NOOP("Gospel"),
	    I18N_NOOP("Noise"),
	    I18N_NOOP("Alternative Rock"), //  40
	    I18N_NOOP("Bass"),
	    I18N_NOOP("Soul"),
	    I18N_NOOP("Punk"),
	    I18N_NOOP("Space"),
	    I18N_NOOP("Meditative"),       //  45
	    I18N_NOOP("Instrumental Pop"),
	    I18N_NOOP("Instrumental Rock"),
	    I18N_NOOP("Ethnic"),
	    I18N_NOOP("Gothic"),
	    I18N_NOOP("Darkwave"),         //  50
	    I18N_NOOP("Techno-Industrial"),
	    I18N_NOOP("Electronic"),
	    I18N_NOOP("Pop-Folk"),
	    I18N_NOOP("Eurodance"),
	    I18N_NOOP("Dream"),            //  55
	    I18N_NOOP("Southern Rock"),
	    I18N_NOOP("Comedy"),
	    I18N_NOOP("Cult"),
	    I18N_NOOP("Gangsta"),
	    I18N_NOOP("Top 40"),           //  60
	    I18N_NOOP("Christian Rap"),
	    I18N_NOOP("Pop/Funk"),
	    I18N_NOOP("Jungle"),
	    I18N_NOOP("Native American"),
	    I18N_NOOP("Cabaret"),          //  65
	    I18N_NOOP("New Wave"),
	    I18N_NOOP("Psychedelic"),
	    I18N_NOOP("Rave"),
	    I18N_NOOP("Showtunes"),
	    I18N_NOOP("Trailer"),          //  70
	    I18N_NOOP("Lo-Fi"),
	    I18N_NOOP("Tribal"),
	    I18N_NOOP("Acid Punk"),
	    I18N_NOOP("Acid Jazz"),
	    I18N_NOOP("Polka"),            //  75
	    I18N_NOOP("Retro"),
	    I18N_NOOP("Musical"),
	    I18N_NOOP("Rock 'n' Roll"),
	    I18N_NOOP("Hard Rock"),

	    // The following genres are Winamp extensions
	    I18N_NOOP("Folk"),             //  80
	    I18N_NOOP("Folk-Rock"),
	    I18N_NOOP("National Folk"),
	    I18N_NOOP("Swing"),
	    I18N_NOOP("Fast Fusion"),
	    I18N_NOOP("Bebop"),            //  85
	    I18N_NOOP("Latin"),
	    I18N_NOOP("Revival"),
	    I18N_NOOP("Celtic"),
	    I18N_NOOP("Bluegrass"),
	    I18N_NOOP("Avantgarde"),       //  90
	    I18N_NOOP("Gothic Rock"),
	    I18N_NOOP("Progressive Rock"),
	    I18N_NOOP("Psychedelic Rock"),
	    I18N_NOOP("Symphonic Rock"),
	    I18N_NOOP("Slow Rock"),        //  95
	    I18N_NOOP("Big Band"),
	    I18N_NOOP("Chorus"),
	    I18N_NOOP("Easy Listening"),
	    I18N_NOOP("Acoustic"),
	    I18N_NOOP("Humour"),           // 100
	    I18N_NOOP("Speech"),
	    I18N_NOOP("Chanson"),
	    I18N_NOOP("Opera"),
	    I18N_NOOP("Chamber Music"),
	    I18N_NOOP("Sonata"),           // 105
	    I18N_NOOP("Symphony"),
	    I18N_NOOP("Booty Bass"),
	    I18N_NOOP("Primus"),
	    I18N_NOOP("Porn Groove"),
	    I18N_NOOP("Satire"),           // 110
	    I18N_NOOP("Slow Jam"),
	    I18N_NOOP("Club"),
	    I18N_NOOP("Tango"),
	    I18N_NOOP("Samba"),
	    I18N_NOOP("Folklore"),         // 115
	    I18N_NOOP("Ballad"),
	    I18N_NOOP("Power Ballad"),
	    I18N_NOOP("Rhythmic Soul"),
	    I18N_NOOP("Freestyle"),
	    I18N_NOOP("Duet"),             // 120
	    I18N_NOOP("Punk Rock"),
	    I18N_NOOP("Drum Solo"),
	    I18N_NOOP("A Cappella"),
	    I18N_NOOP("Euro-House"),
	    I18N_NOOP("Dance Hall"),       // 125
	    I18N_NOOP("Goa"),
	    I18N_NOOP("Drum & Bass"),
	    I18N_NOOP("Club-House"),
	    I18N_NOOP("Hardcore Techno"),
	    I18N_NOOP("Terror"),           // 130
	    I18N_NOOP("Indie"),
	    I18N_NOOP("BritPop"),
	    I18N_NOOP("Negerpunk"),
	    I18N_NOOP("Polsk Punk"),
	    I18N_NOOP("Beat"),             // 135
	    I18N_NOOP("Christian Gangsta Rap"),
	    I18N_NOOP("Heavy Metal"),
	    I18N_NOOP("Black Metal"),
	    I18N_NOOP("Crossover"),
	    I18N_NOOP("Contemporary Christian"), // 140
	    I18N_NOOP("Christian Rock"),

	    // WinAmp 1.91
	    I18N_NOOP("Merengue"),         // 142
	    I18N_NOOP("Salsa"),
	    I18N_NOOP("Thrash Metal"),
	    I18N_NOOP("Anime"),            // 145
	    I18N_NOOP("JPop"),
	    I18N_NOOP("SynthPop"),

	    // WinAmp 5.6
	    I18N_NOOP("Abstract"),         // 148
	    I18N_NOOP("Art Rock"),
	    I18N_NOOP("Baroque"),          // 150
	    I18N_NOOP("Bhangra"),
	    I18N_NOOP("Big Beat"),
	    I18N_NOOP("Breakbeat"),
	    I18N_NOOP("Chillout"),
	    I18N_NOOP("Downtempo"),        // 155
	    I18N_NOOP("Dub"),
	    I18N_NOOP("EBM"),
	    I18N_NOOP("Eclectic"),
	    I18N_NOOP("Electro"),
	    I18N_NOOP("Electroclash"),     // 160
	    I18N_NOOP("Emo"),
	    I18N_NOOP("Experimental"),
	    I18N_NOOP("Garage"),
	    I18N_NOOP("Global"),
	    I18N_NOOP("IDM"),              // 165
	    I18N_NOOP("Illbient"),
	    I18N_NOOP("Industro-Goth"),
	    I18N_NOOP("Jam Band"),
	    I18N_NOOP("Krautrock"),
	    I18N_NOOP("Leftfield"),        // 170
	    I18N_NOOP("Lounge"),
	    I18N_NOOP("Math Rock"),
	    I18N_NOOP("New Romantic"),
	    I18N_NOOP("Nu-Breakz"),
	    I18N_NOOP("Post-Punk"),        // 175
	    I18N_NOOP("Post-Rock"),
	    I18N_NOOP("Psytrance"),
	    I18N_NOOP("Shoegaze"),
	    I18N_NOOP("Space Rock"),
	    I18N_NOOP("Trop Rock"),        // 180
	    I18N_NOOP("World Music"),
	    I18N_NOOP("Neoclassical"),
	    I18N_NOOP("Audiobook"),
	    I18N_NOOP("Audit Theatre"),
	    I18N_NOOP("Neue Deutsche Welle"), // 185
	    I18N_NOOP("Podcast"),
	    I18N_NOOP("Indie Rock"),
	    I18N_NOOP("G-Funk"),
	    I18N_NOOP("Dubstep"),
	    I18N_NOOP("Garage Rock"),      // 190
	    I18N_NOOP("Psybient")
	};

	for (unsigned int i = 0; i < sizeof(map) / sizeof(map[0]); ++i)
	    m_map.insert(i, map[i]);

	m_map.insert(     -1, I18N_NOOP("Unknown"));
    }

}

//***************************************************************************
//***************************************************************************
