/***************************************************************************
                Delay.h  -  delay line for small delays
                             -------------------
    begin                : Sun Nov 11 2007
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

#ifndef DELAY_H
#define DELAY_H

#include "config.h"

#include <QtGlobal>
#include <QObject>
#include <QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{

    class Q_DECL_EXPORT Delay: public Kwave::SampleSource
    {
	Q_OBJECT
	public:
	    /** Constructor */
	    Delay();

	    /** Destructor */
	    virtual ~Delay();

	    /** does the calculation */
            virtual void goOn() Q_DECL_OVERRIDE;

	signals:
	    /** emits a block with delayed wave data */
	    void output(Kwave::SampleArray data);

	public slots:

	    /** receives input data */
	    void input(Kwave::SampleArray data);

	    /**
	     * Sets the delay time, normed to samples.
	     * The default setting is zero.
	     */
	    void setDelay(const QVariant &d);

	private:

	    /** buffer for delaying data */
	    Kwave::SampleFIFO m_fifo;

	    /** buffer for output data */
	    Kwave::SampleArray m_out_buffer;

	    /** delay [samples] */
	    unsigned int m_delay;
    };
}

#endif /* DELAY_H */

//***************************************************************************
//***************************************************************************
