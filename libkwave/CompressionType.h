/***************************************************************************
      CompressionType.h  -  Map for all known compression types
                             -------------------
    begin                : Mon Jul 29 2002
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

#ifndef _COMPRESSION_TYPE_H_
#define _COMPRESSION_TYPE_H_

#include "config.h"

#include <audiofile.h>

#include "libkwave/Compression.h"

namespace Kwave
{

    namespace CompressionType
    {
	/** extended compression types, not from libaudiofile */
	enum {
	    NONE         = 0,
	    G711_ULAW    = AF_COMPRESSION_G711_ULAW,
	    G711_ALAW    = AF_COMPRESSION_G711_ALAW,
	    MS_ADPCM     = AF_COMPRESSION_MS_ADPCM,
	    GSM          = AF_COMPRESSION_GSM,
	    MPEG_LAYER_I = 600,
	    MPEG_LAYER_II,
	    MPEG_LAYER_III,
	    OGG_VORBIS,
	    OGG_OPUS,
	    FLAC
	};
    }
}

#endif /* _COMPRESSION_TYPE_H_ */

//***************************************************************************
//***************************************************************************
