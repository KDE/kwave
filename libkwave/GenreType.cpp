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
QMap<int, KLazyLocalizedString> Kwave::GenreType::m_map;

//***************************************************************************
Kwave::GenreType::GenreType()
{
}

//***************************************************************************
QString Kwave::GenreType::name(int id, bool localized)
{
    fill();

    if (m_map.contains(id))
        return (localized) ? m_map[id].toString() :
                           _(m_map[id].untranslatedText());
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
    QMap<int, KLazyLocalizedString>::Iterator it;
    for (it = m_map.begin(); it != m_map.end(); ++it) {
        if (!_(it.value().untranslatedText()).compare(name,
            Qt::CaseInsensitive))
                return it.key();
        if (it.value().toString().compare(name, Qt::CaseInsensitive) == 0)
            return it.key();
    }

    return -1;
}

//***************************************************************************
QStringList Kwave::GenreType::allTypes()
{
    fill();

    QStringList list;
    for (QMap<int, KLazyLocalizedString>::const_iterator
         it = m_map.constBegin();
         it != m_map.constEnd(); ++it)
    {
        list.append(it.value().toString());
    }

    return list;
}

//***************************************************************************
void Kwave::GenreType::fill()
{
    if (m_map.count()) return;
    {
        static const KLazyLocalizedString map[] = {
            // The following genres are defined in ID3v1
            kli18n("Blues"),            //   0
            kli18n("Classic Rock"),
            kli18n("Country"),
            kli18n("Dance"),
            kli18n("Disco"),
            kli18n("Funk"),             //   5
            kli18n("Grunge"),
            kli18n("Hip-Hop"),
            kli18n("Jazz"),
            kli18n("Metal"),
            kli18n("New Age"),          //  10
            kli18n("Oldies"),
            kli18n("Other"),
            kli18n("Pop"),
            kli18n("Rhythm and Blues"),
            kli18n("Rap"),              //  15
            kli18n("Reggae"),
            kli18n("Rock"),
            kli18n("Techno"),
            kli18n("Industrial"),
            kli18n("Alternative"),      //  20
            kli18n("Ska"),
            kli18n("Death Metal"),
            kli18n("Pranks"),
            kli18n("Soundtrack"),
            kli18n("Euro-Techno"),      //  25
            kli18n("Ambient"),
            kli18n("Trip-Hop"),
            kli18n("Vocal"),
            kli18n("Jazz & Funk"),
            kli18n("Fusion"),           //  30
            kli18n("Trance"),
            kli18n("Classical"),
            kli18n("Instrumental"),
            kli18n("Acid"),
            kli18n("House"),            //  35
            kli18n("Game"),
            kli18n("Sound Clip"),
            kli18n("Gospel"),
            kli18n("Noise"),
            kli18n("Alternative Rock"), //  40
            kli18n("Bass"),
            kli18n("Soul"),
            kli18n("Punk"),
            kli18n("Space"),
            kli18n("Meditative"),       //  45
            kli18n("Instrumental Pop"),
            kli18n("Instrumental Rock"),
            kli18n("Ethnic"),
            kli18n("Gothic"),
            kli18n("Darkwave"),         //  50
            kli18n("Techno-Industrial"),
            kli18n("Electronic"),
            kli18n("Pop-Folk"),
            kli18n("Eurodance"),
            kli18n("Dream"),            //  55
            kli18n("Southern Rock"),
            kli18n("Comedy"),
            kli18n("Cult"),
            kli18n("Gangsta"),
            kli18n("Top 40"),           //  60
            kli18n("Christian Rap"),
            kli18n("Pop/Funk"),
            kli18n("Jungle"),
            kli18n("Native American"),
            kli18n("Cabaret"),          //  65
            kli18n("New Wave"),
            kli18n("Psychedelic"),
            kli18n("Rave"),
            kli18n("Showtunes"),
            kli18n("Trailer"),          //  70
            kli18n("Lo-Fi"),
            kli18n("Tribal"),
            kli18n("Acid Punk"),
            kli18n("Acid Jazz"),
            kli18n("Polka"),            //  75
            kli18n("Retro"),
            kli18n("Musical"),
            kli18n("Rock 'n' Roll"),
            kli18n("Hard Rock"),

            // The following genres are Winamp extensions
            kli18n("Folk"),             //  80
            kli18n("Folk-Rock"),
            kli18n("National Folk"),
            kli18n("Swing"),
            kli18n("Fast Fusion"),
            kli18n("Bebop"),            //  85
            kli18n("Latin"),
            kli18n("Revival"),
            kli18n("Celtic"),
            kli18n("Bluegrass"),
            kli18n("Avantgarde"),       //  90
            kli18n("Gothic Rock"),
            kli18n("Progressive Rock"),
            kli18n("Psychedelic Rock"),
            kli18n("Symphonic Rock"),
            kli18n("Slow Rock"),        //  95
            kli18n("Big Band"),
            kli18n("Chorus"),
            kli18n("Easy Listening"),
            kli18n("Acoustic"),
            kli18n("Humour"),           // 100
            kli18n("Speech"),
            kli18n("Chanson"),
            kli18n("Opera"),
            kli18n("Chamber Music"),
            kli18n("Sonata"),           // 105
            kli18n("Symphony"),
            kli18n("Booty Bass"),
            kli18n("Primus"),
            kli18n("Porn Groove"),
            kli18n("Satire"),           // 110
            kli18n("Slow Jam"),
            kli18n("Club"),
            kli18n("Tango"),
            kli18n("Samba"),
            kli18n("Folklore"),         // 115
            kli18n("Ballad"),
            kli18n("Power Ballad"),
            kli18n("Rhythmic Soul"),
            kli18n("Freestyle"),
            kli18n("Duet"),             // 120
            kli18n("Punk Rock"),
            kli18n("Drum Solo"),
            kli18n("A Cappella"),
            kli18n("Euro-House"),
            kli18n("Dance Hall"),       // 125
            kli18n("Goa"),
            kli18n("Drum & Bass"),
            kli18n("Club-House"),
            kli18n("Hardcore Techno"),
            kli18n("Terror"),           // 130
            kli18n("Indie"),
            kli18n("BritPop"),
            kli18n("Negerpunk"),
            kli18n("Polsk Punk"),
            kli18n("Beat"),             // 135
            kli18n("Christian Gangsta Rap"),
            kli18n("Heavy Metal"),
            kli18n("Black Metal"),
            kli18n("Crossover"),
            kli18n("Contemporary Christian"), // 140
            kli18n("Christian Rock"),

            // WinAmp 1.91
            kli18n("Merengue"),         // 142
            kli18n("Salsa"),
            kli18n("Thrash Metal"),
            kli18n("Anime"),            // 145
            kli18n("JPop"),
            kli18n("SynthPop"),

            // WinAmp 5.6
            kli18n("Abstract"),         // 148
            kli18n("Art Rock"),
            kli18n("Baroque"),          // 150
            kli18n("Bhangra"),
            kli18n("Big Beat"),
            kli18n("Breakbeat"),
            kli18n("Chillout"),
            kli18n("Downtempo"),        // 155
            kli18n("Dub"),
            kli18n("EBM"),
            kli18n("Eclectic"),
            kli18n("Electro"),
            kli18n("Electroclash"),     // 160
            kli18n("Emo"),
            kli18n("Experimental"),
            kli18n("Garage"),
            kli18n("Global"),
            kli18n("IDM"),              // 165
            kli18n("Illbient"),
            kli18n("Industro-Goth"),
            kli18n("Jam Band"),
            kli18n("Krautrock"),
            kli18n("Leftfield"),        // 170
            kli18n("Lounge"),
            kli18n("Math Rock"),
            kli18n("New Romantic"),
            kli18n("Nu-Breakz"),
            kli18n("Post-Punk"),        // 175
            kli18n("Post-Rock"),
            kli18n("Psytrance"),
            kli18n("Shoegaze"),
            kli18n("Space Rock"),
            kli18n("Trop Rock"),        // 180
            kli18n("World Music"),
            kli18n("Neoclassical"),
            kli18n("Audiobook"),
            kli18n("Audio Theatre"),
            kli18n("Neue Deutsche Welle"), // 185
            kli18n("Podcast"),
            kli18n("Indie Rock"),
            kli18n("G-Funk"),
            kli18n("Dubstep"),
            kli18n("Garage Rock"),      // 190
            kli18n("Psybient")
        };

        for (unsigned int i = 0; i < sizeof(map) / sizeof(map[0]); ++i)
            m_map.insert(i, map[i]);

        m_map.insert(     -1, kli18n("Unknown"));
    }

}

//***************************************************************************
//***************************************************************************
