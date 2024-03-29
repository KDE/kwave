/***************************************************************************
         WavFormatMap.h  -  list of known wav file formats
                             -------------------
    begin                : Apr 28 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WAV_FORMAT_MAP_H
#define WAV_FORMAT_MAP_H

#include "config.h"

#include <QMap>
#include <QString>

#include "WavFileFormat.h"

namespace Kwave
{
    /**
     * list of known wav file formats
     */
    class WavFormatMap: public QMap<Kwave::wav_format_id, QString>
    {
    public:
        /** Constructor, fills the map with all known types */
        WavFormatMap();

        /** Destructor */
        virtual ~WavFormatMap() {}

        /** Returns the name of an id, or "unknown" if not found. */
        const QString &findName(unsigned int id);

    };
}

#endif /* WAV_FORMAT_MAP_H */

//***************************************************************************
//***************************************************************************
