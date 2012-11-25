/*************************************************************************
         RecordState.h  -  state of the audio recorder
                             -------------------
    begin                : Sat Oct 04 2003
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

#ifndef _RECORD_STATE_H_
#define _RECORD_STATE_H_

#include "config.h"

namespace Kwave
{
    typedef enum
    {

	/** Settings are not valid yet, cannot open device etc... */
	REC_UNINITIALIZED = 0,

	/** Empty, nothing recorded yet and no recording started */
	REC_EMPTY,

	/**
	 * Buffering data for pre-recording, if pre-recording is enabled.
	 * Otherwise the state will advance to REC_WAITING_FOR_TRIGGER
	 * immediately.
	 */
	REC_BUFFERING,

	/**
	 * Pre-recording data into a FIFO. If a trigger has been set, this
	 * is equal to REC_WAITING_FOR_TRIGGER and the trigger will start
	 * the recording.
	 */
	REC_PRERECORDING,

	/**
	 * Waiting for reaching the trigger if a trigger has been set and
	 * enabled. Otherwise the state will advance to REC_RECORDING
	 * immediately.
	 */
	REC_WAITING_FOR_TRIGGER,

	/**
	 * Recording is in progress, producing real data. Can change to
	 * REC_PAUSED or REC_DONE.
	 */
	REC_RECORDING,

	/**
	 * Recording is paused, normally for waiting until the user presses
	 * "continue". After pause either the previous state will be set
	 * again or the recording can be stopped (change to REC_DONE) or
	 * canceled (change to REC_EMPTY).
	 */
	REC_PAUSED,

	/**
	 * Recording done, data was produced.
	 */
	REC_DONE

    } RecordState;
}

#endif /* _RECORD_STATE_H_ */

//***************************************************************************
//***************************************************************************
