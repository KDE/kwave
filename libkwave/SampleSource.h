/*************************************************************************
    SampleSource.h  -  base class with a generic sample source
                             -------------------
    begin                : Sun Oct 07 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _SAMPLE_SOURCE_H_
#define _SAMPLE_SOURCE_H_

#include "config.h"

#include <QObject>

#include <kdemacros.h>
#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>

#include "libkwave/SampleArray.h"
#include "libkwave/modules/KwaveStreamObject.h"


namespace Kwave {
    class KDE_EXPORT SampleSource: public Kwave::StreamObject
    {
        Q_OBJECT
    public:

	/**
	 * Constructor
	 *
	 * @param parent a parent object, passed to QObject (optional)
	 */
	SampleSource(QObject *parent=0);

	/** Destructor */
	virtual ~SampleSource();

	/**
	 * Each KwaveSampleSource has to derive this method for producing
	 * sample data. It then should emit a signal like this:
	 * "output(SampleArray data)"
	 */
	virtual void goOn() = 0;

	/**
	 * enqueues the call to the goOn() function into a KDE
	 * thread weaver.
	 * @param weaver a thread weaver into which we enqueue
	 * @return a job that processes the work
	 */
	virtual ThreadWeaver::Job *enqueue(ThreadWeaver::Weaver *weaver);

	/**
	 * Returns true if the end of the source has been reached,
	 * e.g. at EOF of an input stream. The default implementation
	 * always returns false, which means that the source is always
	 * able to produce data (useful for signal generators).
	 *
	 * @return true if it can produce more sample data, otherwise false
	 */
	virtual bool done() const { return false; }

    };
}

#endif /* _SAMPLE_SOURCE_H_ */
