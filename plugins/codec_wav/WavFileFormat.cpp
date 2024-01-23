/***************************************************************************
      WavFileFormat.cpp  -  wav file formats
                             -------------------
    begin                : Oct 02 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#include "config.h"

#include <audiofile.h>
#include <stdlib.h>

#include "WavFileFormat.h"

//***************************************************************************
QList<Kwave::Compression::Type> Kwave::audiofileCompressionTypes()
{
    QList<Kwave::Compression::Type> list;

    const long int numCompressionTypes = afQueryLong(
        AF_QUERYTYPE_COMPRESSION, AF_QUERY_ID_COUNT, 0, 0, 0);

    if (numCompressionTypes != 0) {
        int *compressions = static_cast<int *>(afQueryPointer(
            AF_QUERYTYPE_COMPRESSION, AF_QUERY_IDS, 0, 0, 0));

        if (compressions) {
            for (long int index = 0; index < numCompressionTypes; index++) {
                Kwave::Compression::Type compression_type =
                    Kwave::Compression::fromAudiofile(compressions[index]);
                if (!list.contains(compression_type))
                    list.append(compression_type);
            }
            free(compressions);
        }
    }

    return list;
}

//***************************************************************************
//***************************************************************************
