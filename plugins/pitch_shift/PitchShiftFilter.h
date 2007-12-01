/***************************************************************************
     PitchShiftFilter.h  -  filter for modifying the "pitch_shift"
                             -------------------
    begin                : Wed Nov 28 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    based on synth_pitch_shift_impl.cc from the aRts project

    copyright (C) 2000 Jeff Tranter <tranter@pobox.com>
              (C) 1999 Stefan Westerfeld <stefan@space.twc.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PITCH_SHIFT_FILTER_H_
#define _PITCH_SHIFT_FILTER_H_

#include "config.h"

#include <qobject.h>
#include <qvariant.h>
#include <qvaluevector.h>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveSampleSource.h"

class PitchShiftFilter: public Kwave::SampleSource
{
    Q_OBJECT
public:

    /** Constructor */
    PitchShiftFilter();

    /** Destructor */
    virtual ~PitchShiftFilter();

    /** does the calculation */
    virtual void goOn();

signals:

    /** emits a block with the filtered data */
    void output(Kwave::SampleArray &data);

public slots:

    /** receives input data */
    void input(Kwave::SampleArray &data);

    /**
     * Sets the speed factor
     * @param speed factor as a double
     */
    void setSpeed(const QVariant &speed);

    /**
     * Sets the frequency parameter
     * @param freq the normed frequency
     */
    void setFrequency(const QVariant &freq);

private:

    /** reset/initialize the filter and buffer */
    void initFilter();

private:

    /** buffer for input */
    Kwave::SampleArray m_buffer;

    /** speed factor */
    double m_speed;

    /** base frequency */
    double m_frequency;

    enum { MAXDELAY = 1000000 };

    QValueVector<float> m_dbuffer;
    float m_lfopos;
    float m_b1pos;
    float m_b2pos;
    float m_b1inc;
    float m_b2inc;
    bool m_b1reset;
    bool m_b2reset;
    int m_dbpos;
};


#endif /* _PITCH_SHIFT_FILTER_H_ */
