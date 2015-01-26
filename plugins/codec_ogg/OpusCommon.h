/*************************************************************************
           OpusCommon.h  -  common functions for Opus Codec
                             -------------------
    begin                : Tue Jan 08 2013
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

#ifndef _OPUS_COMMON_H_
#define _OPUS_COMMON_H_

#include "config.h"
#ifdef HAVE_OGG_OPUS

#include <opus/opus.h>

#include <QtCore/QString>

#include "libkwave/String.h"

namespace Kwave
{

    /**
     * round up to the next supported sample rate
     * @param rate arbitrary sample rate
     * @return next supported rate
     */
    int opus_next_sample_rate(int rate);

    /**
     * Transforms an error code from the Opus library into a QString
     * @param err one of the OPUS_... error codes
     * @return a QString with the error code (localized)
     */
    QString opus_error(int err);

}

#endif /* HAVE_OGG_OPUS */

#endif /* _OPUS_COMMON_H_ */

//***************************************************************************
//***************************************************************************
