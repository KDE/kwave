/*************************************************************************
     StandardBitrates.h  -  well known bitrates from ogg/vorbis + MP3
                             -------------------
    begin                : Tue Jun 17 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _STANDARD_BITRATES_H_
#define _STANDARD_BITRATES_H_

#include "config.h"

#include <kdemacros.h>

#include <QList>

class KDE_EXPORT StandardBitrates: public QList<int>
{
public:
    /** Constructor */
    StandardBitrates();

    /** Destructor */
    virtual ~StandardBitrates();

    /** returns a const reference to the list of bitrates */
    static const StandardBitrates &instance();

    /**
     * Returns the standard bitrate that is nearest to the given one
     * @param rate given rate in [bits/second]
     * @return nearest bitrate in [bits/second]
     */
    virtual int nearest(int rate) const;

};

#endif /* _STANDARD_BITRATES_H_ */
