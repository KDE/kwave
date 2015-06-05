/*************************************************************************
         OpusCommon.cpp  -  common functions for Opus Codec
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

#include "config.h"

#include <KLocalizedString>

#include "OpusCommon.h"

//***************************************************************************
/**
 * round up to the next supported sample rate
 * @param rate arbitrary sample rate
 * @return next supported rate
 */
int Kwave::opus_next_sample_rate(int rate)
{
    if (rate < 8000)
	return 8000;
    else if (rate <= 12000)
	return 12000;
    else if (rate <= 16000)
	return 16000;
    else if (rate <= 24000)
	return 24000;
    else
	return 48000;
}

//***************************************************************************
QString Kwave::opus_error(int err)
{
    QString msg;

    switch (err)
    {
	case OPUS_OK:
	    msg = QString();
	    break;
	case OPUS_BAD_ARG:
	    msg = i18n("One or more invalid/out of range arguments.");
	    break;
	case OPUS_BUFFER_TOO_SMALL:
	    msg = i18n("The mode struct passed is invalid.");
	    break;
	case OPUS_INTERNAL_ERROR:
	    msg = i18n("An internal error was detected.");
	    break;
	case OPUS_INVALID_PACKET:
	    msg = i18n("The compressed data passed is corrupted.");
	    break;
	case OPUS_UNIMPLEMENTED:
	    msg = i18n("Invalid/unsupported request number.");
	    break;
	case OPUS_INVALID_STATE:
	    msg = i18n("A decoder structure is invalid or already freed.");
	    break;
	case OPUS_ALLOC_FAIL:
	    msg = i18n("Out of memory");
	    break;
	default:
	    msg = i18n("Decoder error: %1", _(opus_strerror(err)));
	    break;
    }
    return msg;
}

//***************************************************************************
//***************************************************************************
