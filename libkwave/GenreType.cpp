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
	if (it.data() == name) return it.key();
    }
    return -1;
}

//***************************************************************************
void GenreType::fill()
{
    if (m_map.count()) return;

    m_map.insert(-1, i18n("Unknown"));

    // The following genres is defined in ID3v1
    m_map.insert(      0, i18n("Blues"));
    m_map.insert(      1, i18n("Classic Rock"));
    m_map.insert(      2, i18n("Country"));
    m_map.insert(      3, i18n("Dance"));
    m_map.insert(      4, i18n("Disco"));
    m_map.insert(      5, i18n("Funk"));
    m_map.insert(      6, i18n("Grunge"));
    m_map.insert(      7, i18n("Hip-Hop"));
    m_map.insert(      8, i18n("Jazz"));
    m_map.insert(      9, i18n("Metal"));
    m_map.insert(     10, i18n("New Age"));
    m_map.insert(     11, i18n("Oldies"));
    m_map.insert(     12, i18n("Other"));
    m_map.insert(     13, i18n("Pop"));
    m_map.insert(     14, i18n("R&B"));
    m_map.insert(     15, i18n("Rap"));
    m_map.insert(     16, i18n("Reggae"));
    m_map.insert(     17, i18n("Rock"));
    m_map.insert(     18, i18n("Techno"));
    m_map.insert(     19, i18n("Industrial"));
    m_map.insert(     20, i18n("Alternative"));
    m_map.insert(     21, i18n("Ska"));
    m_map.insert(     22, i18n("Death Metal"));
    m_map.insert(     23, i18n("Pranks"));
    m_map.insert(     24, i18n("Soundtrack"));
    m_map.insert(     25, i18n("Euro-Techno"));
    m_map.insert(     26, i18n("Ambient"));
    m_map.insert(     27, i18n("Trip-Hop"));
    m_map.insert(     28, i18n("Vocal"));
    m_map.insert(     29, i18n("Jazz+Funk"));
    m_map.insert(     30, i18n("Fusion"));
    m_map.insert(     31, i18n("Trance"));
    m_map.insert(     32, i18n("Classical"));
    m_map.insert(     33, i18n("Instrumental"));
    m_map.insert(     34, i18n("Acid"));
    m_map.insert(     35, i18n("House"));
    m_map.insert(     36, i18n("Game"));
    m_map.insert(     37, i18n("Sound Clip"));
    m_map.insert(     38, i18n("Gospel"));
    m_map.insert(     39, i18n("Noise"));
    m_map.insert(     40, i18n("AlternRock"));
    m_map.insert(     41, i18n("Bass"));
    m_map.insert(     42, i18n("Soul"));
    m_map.insert(     43, i18n("Punk"));
    m_map.insert(     44, i18n("Space"));
    m_map.insert(     45, i18n("Meditative"));
    m_map.insert(     46, i18n("Instrumental Pop"));
    m_map.insert(     47, i18n("Instrumental Rock"));
    m_map.insert(     48, i18n("Ethnic"));
    m_map.insert(     49, i18n("Gothic"));
    m_map.insert(     50, i18n("Darkwave"));
    m_map.insert(     51, i18n("Techno-Industrial"));
    m_map.insert(     52, i18n("Electronic"));
    m_map.insert(     53, i18n("Pop-Folk"));
    m_map.insert(     54, i18n("Eurodance"));
    m_map.insert(     55, i18n("Dream"));
    m_map.insert(     56, i18n("Southern Rock"));
    m_map.insert(     57, i18n("Comedy"));
    m_map.insert(     58, i18n("Cult"));
    m_map.insert(     59, i18n("Gangsta"));
    m_map.insert(     60, i18n("Top 40"));
    m_map.insert(     61, i18n("Christian Rap"));
    m_map.insert(     62, i18n("Pop/Funk"));
    m_map.insert(     63, i18n("Jungle"));
    m_map.insert(     64, i18n("Native American"));
    m_map.insert(     65, i18n("Cabaret"));
    m_map.insert(     66, i18n("New Wave"));
    m_map.insert(     67, i18n("Psychadelic"));
    m_map.insert(     68, i18n("Rave"));
    m_map.insert(     69, i18n("Showtunes"));
    m_map.insert(     70, i18n("Trailer"));
    m_map.insert(     71, i18n("Lo-Fi"));
    m_map.insert(     72, i18n("Tribal"));
    m_map.insert(     73, i18n("Acid Punk"));
    m_map.insert(     74, i18n("Acid Jazz"));
    m_map.insert(     75, i18n("Polka"));
    m_map.insert(     76, i18n("Retro"));
    m_map.insert(     77, i18n("Musical"));
    m_map.insert(     78, i18n("Rock & Roll"));
    m_map.insert(     79, i18n("Hard Rock"));

    // The following genres are Winamp extensions
    m_map.insert(     80, i18n("Folk"));
    m_map.insert(     81, i18n("Folk-Rock"));
    m_map.insert(     82, i18n("National Folk"));
    m_map.insert(     83, i18n("Swing"));
    m_map.insert(     84, i18n("Fast Fusion"));
    m_map.insert(     85, i18n("Bebob"));
    m_map.insert(     86, i18n("Latin"));
    m_map.insert(     87, i18n("Revival"));
    m_map.insert(     88, i18n("Celtic"));
    m_map.insert(     89, i18n("Bluegrass"));
    m_map.insert(     90, i18n("Avantgarde"));
    m_map.insert(     91, i18n("Gothic Rock"));
    m_map.insert(     92, i18n("Progressive Rock"));
    m_map.insert(     93, i18n("Psychedelic Rock"));
    m_map.insert(     94, i18n("Symphonic Rock"));
    m_map.insert(     95, i18n("Slow Rock"));
    m_map.insert(     96, i18n("Big Band"));
    m_map.insert(     97, i18n("Chorus"));
    m_map.insert(     98, i18n("Easy Listening"));
    m_map.insert(     99, i18n("Acoustic"));
    m_map.insert(    100, i18n("Humour"));
    m_map.insert(    101, i18n("Speech"));
    m_map.insert(    102, i18n("Chanson"));
    m_map.insert(    103, i18n("Opera"));
    m_map.insert(    104, i18n("Chamber Music"));
    m_map.insert(    105, i18n("Sonata"));
    m_map.insert(    106, i18n("Symphony"));
    m_map.insert(    107, i18n("Booty Bass"));
    m_map.insert(    108, i18n("Primus"));
    m_map.insert(    109, i18n("Porn Groove"));
    m_map.insert(    110, i18n("Satire"));
    m_map.insert(    111, i18n("Slow Jam"));
    m_map.insert(    112, i18n("Club"));
    m_map.insert(    113, i18n("Tango"));
    m_map.insert(    114, i18n("Samba"));
    m_map.insert(    115, i18n("Folklore"));
    m_map.insert(    116, i18n("Ballad"));
    m_map.insert(    117, i18n("Power Ballad"));
    m_map.insert(    118, i18n("Rhythmic Soul"));
    m_map.insert(    119, i18n("Freestyle"));
    m_map.insert(    120, i18n("Duet"));
    m_map.insert(    121, i18n("Punk Rock"));
    m_map.insert(    122, i18n("Drum Solo"));
    m_map.insert(    123, i18n("Acapella"));
    m_map.insert(    124, i18n("Euro-House"));
    m_map.insert(    125, i18n("Dance Hall"));
}

//***************************************************************************
//***************************************************************************
