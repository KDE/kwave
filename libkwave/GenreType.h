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

#ifndef _GENRE_TYPE_H_
#define _GENRE_TYPE_H_

#include "config.h"

#include <kdemacros.h>

#include <QMap>
#include <QString>

class KDE_EXPORT GenreType
{
private:

    /** private constructor */
    GenreType();

public:

    /** Destructor */
    virtual ~GenreType() {}

    /** get the localized name from the id */
    static QString name(int id);

    /** parse the numeric id from an ID3 tag */
    static int fromID3(const QString &tag);

    /** try to find the numeric id from the name */
    static int id(const QString &name);

protected:

    /** fills the map if it is empty */
    static void fill();

private:

    /** map with numeric ids and names */
    static QMap<int, QString> m_map;

};

#endif /* _GENRE_TYPE_H_ */
