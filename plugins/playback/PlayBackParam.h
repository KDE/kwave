/***************************************************************************
        PlayBackParam.h -  typedef for a struct with parameters for playback
			     -------------------
    begin                : Tue May 15 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _PLAY_BACK_PARAM_H_
#define _PLAY_BACK_PARAM_H_

#include <qstring.h>

/**
 * The playback_param_t struct contains all necessary parameters for
 * setting up (initializing) a playback device.
 */
typedef struct {
    /** Sample rate [samples/second] */
    double rate;

    /** Number of channels. */
    unsigned int channels;

    /** Resolution [bits/sample] */
    unsigned int bits_per_sample;

    /** Path to the output device */
    QString device;

    /** base of the buffer size (buffer size will be 2^bufbase) */
    unsigned int bufbase;

} playback_param_t;

#endif /* _PLAY_BACK_PARAM_H_ */
