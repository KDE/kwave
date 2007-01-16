/***************************************************************************
         PlayBackParam.h -  class with parameters for playback
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
 * enum for the known playback methods
 */
typedef enum {
    PLAYBACK_NONE = 0, /**< none selected */
    PLAYBACK_ARTS,     /**< aRts sound daemon */
    PLAYBACK_OSS,      /**< OSS native or ALSA OSS emulation */
    PLAYBACK_ALSA,     /**< ALSA native */
    PLAYBACK_JACK,     /**< Jack sound daemon */
    PLAYBACK_INVALID   /**< (keep this the last entry, EOL delimiter) */
} playback_method_t;

/** post-increment operator for the playback method */
inline playback_method_t &operator ++(playback_method_t &m) {
    return (m = (m < PLAYBACK_INVALID) ?
                (playback_method_t)((int)(m) + 1) : m);
}

/**
 * A class that contains all necessary parameters for
 * setting up (initializing) a playback device.
 */
class PlayBackParam
{
public:
    /** Default constructor */
    PlayBackParam()
        :rate(44100), channels(2), bits_per_sample(16),
        device("/dev/dsp"), bufbase(10),
        method(PLAYBACK_ARTS)
    {
    };

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

    /** method/class to use for playback */
    playback_method_t method;

};

#endif /* _PLAY_BACK_PARAM_H_ */
