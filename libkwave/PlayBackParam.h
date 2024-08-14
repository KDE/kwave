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

#ifndef PLAY_BACK_PARAM_H
#define PLAY_BACK_PARAM_H

#include "libkwave_export.h"

#include <QtGlobal>
#include <QString>

namespace Kwave
{

    /**
     * enum for the known playback methods
     * (sorted, preferred first)
     */
    typedef enum {
        PLAYBACK_NONE = 0,   /**< none selected */
        PLAYBACK_JACK,       /**< Jack sound daemon */
        PLAYBACK_QT_AUDIO,   /**< Qt Multimedia */
        PLAYBACK_PULSEAUDIO, /**< PulseAudio Sound Server */
        PLAYBACK_ALSA,       /**< ALSA native */
        PLAYBACK_OSS,        /**< OSS native or ALSA OSS emulation */
        PLAYBACK_INVALID     /**< (keep this the last entry, EOL delimiter) */
    } playback_method_t;

    /** post-increment operator for the playback method */
    inline Kwave::playback_method_t &operator ++(Kwave::playback_method_t &m) {
        return (m = (m < Kwave::PLAYBACK_INVALID) ?
            static_cast<Kwave::playback_method_t>(static_cast<int>(m) + 1) : m);
    }

    /**
     * A class that contains all necessary parameters for
     * setting up (initializing) a playback device.
     */
    class LIBKWAVE_EXPORT PlayBackParam
    {
    public:
        /** Default constructor */
        PlayBackParam()
            :rate(44100), channels(2), bits_per_sample(16),
            device(), bufbase(10),
            method(Kwave::PLAYBACK_NONE)
        {
        }

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
        Kwave::playback_method_t method;

    };
}

#endif /* PLAY_BACK_PARAM_H */

//***************************************************************************
//***************************************************************************
