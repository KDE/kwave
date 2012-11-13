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

#ifndef _PLAYBACK_DEVICE_FACTORY_H_
#define _PLAYBACK_DEVICE_FACTORY_H_

// some forward declarations
class PlayBackParam;
class QString;

namespace Kwave
{
    class PlayBackDevice;

    class PlaybackDeviceFactory
    {
    public:
	/** virtual destructor, just to satisfy gcc */
	virtual ~PlaybackDeviceFactory()
	{
	}

	/**
	* Opens and initializes the playback device. If the initialization
	* worked, it returns a valid pointer. On any errors m_device
	* will be 0. If a device was open before, it will be closed.
	* @param name the name of the logical playback device or the name
	*             of the lowlevel device. If null or zero-length, the
	*             default device will be used.
	* @param tracks number of tracks,
	*               if negative use the setting of playback_params
	* @param playback_params points to a structure with playback
	*                        parameters. If null, the default parameters
	*                        of the current signal will be used
	* @return a pointer to an opened PlayBackDevice or null if something
	*         failed
	* @see PlayBackDevice
	*/
	virtual Kwave::PlayBackDevice *openDevice(const QString &name,
	    int tracks = -1,
	    const PlayBackParam *playback_params = 0) = 0;

	/**
	* Returns true if the given device name is supported
	* and can be used for openDevice.
	* @param name the name of a playback device
	* @return true if supported
	* @see openDevice
	*/
	virtual bool supportsDevice(const QString &name) = 0;


    };
}

#endif /* _PLAYBACK_DEVICE_FACTORY_H_ */

//***************************************************************************
//***************************************************************************
