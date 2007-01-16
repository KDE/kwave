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

#ifndef _WAV_FORMAT_MAP_H_
#define _WAV_FORMAT_MAP_H_

#include "config.h"
#include <qmap.h>
#include <qstring.h>
#include "WavFileFormat.h"

class WavFormatMap: public QMap<wav_format_id, QString>
{
public:
    /** Constructor, fills the map with all known types */
    WavFormatMap();

    /** Destructor */
    virtual ~WavFormatMap() {};

    /** Returns the name of an id, or "unknown" if not found. */
    const QString &findName(unsigned int id);

};

#endif /* _WAV_FORMAT_MAP_H_ */
