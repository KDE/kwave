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

#ifndef GENRE_TYPE_H
#define GENRE_TYPE_H

#include "config.h"
#include "libkwave_export.h"

#include <KLazyLocalizedString>
#include <QtGlobal>
#include <QMap>
#include <QString>
#include <QStringList>

namespace Kwave
{

    class LIBKWAVE_EXPORT GenreType
    {
    private:

        /** private constructor */
        GenreType();

    public:

        /** Destructor */
        virtual ~GenreType() {}

        /**
         * get the localized name from the id
         * @param id the numeric id of the genre type
         * @param localized if true, return a localized name, otherwise raw (en)
         * @return name of the genre
         */
        static QString name(int id, bool localized);

        /** parse the numeric id from an ID3 tag */
        static int fromID3(const QString &tag);

        /** try to find the numeric id from the name */
        static int id(const QString &name);

        /** returns a list with all known genre types (localized) */
        static QStringList allTypes();

    protected:

        /** fills the map if it is empty */
        static void fill();

    private:

        /** map with numeric ids and names */
        static QMap<int, KLazyLocalizedString> m_map;

    };
}

#endif /* GENRE_TYPE_H */

//***************************************************************************
//***************************************************************************
