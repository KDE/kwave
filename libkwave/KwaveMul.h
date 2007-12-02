/***************************************************************************
             KwaveMul.h  -  multiplier
                             -------------------
    begin                : Thu Nov 01 2007
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

#ifndef _KWAVE_MUL_H_
#define _KWAVE_MUL_H_

#include "config.h"

#include "libkwave/KwaveSampleSource.h"
#include "libkwave/KwaveSampleSink.h"

class QVariant;

namespace Kwave {

    class Mul: public Kwave::SampleSource
    {
	Q_OBJECT
	public:
	    /** Constructor */
	    Mul();

	    /** Destructor */
	    virtual ~Mul();

	    /** does the calculation */
	    virtual void goOn();

	signals:
	    /** emits a block with the interpolated curve */
	    void output(Kwave::SampleArray &data);

	public slots:

	    /** receives input data for input A */
	    void input_a(Kwave::SampleArray &data);

	    /** receives input data for input B */
	    void input_b(Kwave::SampleArray &data);

	    /** sets input A to a constant value (as float) */
	    void set_a(const QVariant &a);

	    /** sets input B to a constant value (as float) */
	    void set_b(const QVariant &b);

	private:

	    /** buffer for input A */
	    Kwave::SampleArray m_buffer_a;

	    /** buffer for input B */
	    Kwave::SampleArray m_buffer_b;

	    /** buffer for output data */
	    Kwave::SampleArray m_buffer_x;

	    /** number of calls to input_a */
	    unsigned int m_count_a;

	    /** number of calls to input_b */
	    unsigned int m_count_b;

	    /** if true, input A is a constant */
	    bool m_a_is_const;

	    /** if true, input B is a constant */
	    bool m_b_is_const;

	    /** if m_a_is_const is set, the value of A */
	    float m_value_a;

	    /** if m_b_is_const is set, the value of B */
	    float m_value_b;

    };
}

#endif /* _KWAVE_MUL_H_ */
