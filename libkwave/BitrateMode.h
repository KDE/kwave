/***************************************************************************
          BitrateMode.h  -  mode for bitrate handling
                             -------------------
    begin                : Wed Jan 09 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#ifndef BITRATE_MODE_H
#define BITRATE_MODE_H

#include "config.h"

namespace Kwave
{
    typedef enum {
	BITRATE_MODE_NONE     = 0, /**< no preference */
	BITRATE_MODE_ABR      = 1, /**< average bitrate */
	BITRATE_MODE_VBR      = 2, /**< variable bitrage */
	BITRATE_MODE_CVBR     = 3, /**< constant variable bitrage */
	BITRATE_MODE_CBR      = 4, /**< constant bitrate */
	BITRATE_MODE_CBR_HARD = 5  /**< hard constant bitrate */
    } bitrate_mode_t;
}

#endif /* BITRATE_MODE_H */

//***************************************************************************
//***************************************************************************
