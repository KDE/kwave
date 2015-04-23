/***************************************************************************
    PlaybackDeviceFactory.h  -  interface for playback device factories
			     -------------------
    begin                : Mon May 19 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef PLAYBACK_DEVICE_FACTORY_H
#define PLAYBACK_DEVICE_FACTORY_H

#include <QtCore/QList>

#include "libkwave/PlayBackParam.h"

namespace Kwave
{

    class PlayBackDevice;

    class PlaybackDeviceFactory
    {
    public:
	/** virtual destructor */
	virtual ~PlaybackDeviceFactory() { }

	/**
	 * Create a playback device matching the given playback method.
	 * @param method a playback_method_t (aRts, ALSA, OSS...)
	 * @return a new PlayBackDevice or 0 if failed
	 */
	virtual Kwave::PlayBackDevice *createDevice(
	    Kwave::playback_method_t method) = 0;

	/**
	 * Returns a list of supported playback methods.
	 * @return list of all supported playback methods, should not contain
	 *         "any" or "invalid"
	 */
	virtual QList<Kwave::playback_method_t> supportedMethods() = 0;

    };
}

#endif /* PLAYBACK_DEVICE_FACTORY_H */

//***************************************************************************
//***************************************************************************
