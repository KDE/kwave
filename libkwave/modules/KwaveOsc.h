/***************************************************************************
             KwaveOsc.h  -  simple sine oscillator
                             -------------------
    begin                : Tue Nov 06 2007
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

#ifndef _KWAVE_OSC_H_
#define _KWAVE_OSC_H_

#include "config.h"

#include <QObject>
#include <QVariant>

#include <kdemacros.h>

#include "libkwave/KwaveSampleSource.h"

namespace Kwave {

    class KDE_EXPORT Osc: public Kwave::SampleSource
    {
	Q_OBJECT
	public:
	    /** Constructor */
	    Osc();

	    /** Destructor */
	    virtual ~Osc();

	    /** does the calculation */
	    virtual void goOn();

	signals:
	    /** emits a block with sine wave data */
	    void output(Kwave::SampleArray data);

	public slots:
	    /**
	     * Sets the frequency of the sine wave, normed to the
	     * sample frequency. You should pass the frequency that
	     * you want, divided through the sample frequency.
	     * If you never call this, the frequency will be undefined!
	     */
	    void setFrequency(const QVariant &f);

	    /**
	     * Sets the phase of the sine wave in RAD [0...2*Pi].
	     * The default setting is zero.
	     */
	    void setPhase(const QVariant &p);

	    /**
	     * Sets the amplitude of the sine wave, normed to the
	     * range of [0.0 ... 1.0]. The default is 1.0.
	     */
	    void setAmplitude(const QVariant &a);

	private:

	    /** buffer for output data */
	    Kwave::SampleArray m_buffer;

	    /** current time multiplied by 2*Pi*f */
	    qreal m_omega_t;

	    /** frequency [samples/period] */
	    qreal m_f;

	    /** amplitude [0...1] */
	    qreal m_a;
    };
}

#endif /* _KWAVE_OSC_H_ */
