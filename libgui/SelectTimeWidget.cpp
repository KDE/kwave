/***************************************************************************
   SelectTimeWidget.cpp  -  widget for selecting a time or range
                             -------------------
    begin                : Thu Jan 16 2003
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <math.h>

#include <qradiobutton.h>
#include <qslider.h>

#include <klocale.h>
#include <knuminput.h>

#include "IntValidatorProxy.h"
#include "SelectTimeWidget.h"

//***************************************************************************
SelectTimeWidget::SelectTimeWidget(QWidget *widget, const char *name)
    :SelectTimeWidgetBase(widget, name),
     m_mode(bySamples), m_range(0), m_rate(1.0), m_offset(0), m_length(0),
     m_timer(this)
{
}

//***************************************************************************
void SelectTimeWidget::init(Mode mode, double range, double sample_rate,
                            unsigned int offset, unsigned int signal_length)
{
    m_mode  = mode;
    m_range = range;
    m_rate  = sample_rate;
    m_offset = offset;
    m_length = signal_length;
    
    ASSERT(m_rate);
    ASSERT(m_length);
    ASSERT(rbTime);
    ASSERT(rbSamples);
    ASSERT(rbPercents);
    if (!m_rate) m_rate = 1.0;
    if (!m_length) m_length = 1;

    // set range of selection by sample
    edSamples->setPrecision(0);
    edSamples->setRange(0.0, (double)(m_length-m_offset), 1.0, false);

    // set range of time controls
    int t = (int)ceil(((m_length - m_offset)*1E3) / m_rate);
    sbMilliseconds->setMaxValue(t);
    t /= 1000;
    sbSeconds->setMaxValue(t);
    t /= 60;
    sbMinutes->setMaxValue(t);
    t /= 60;
    sbHours->setMaxValue(t);

    // activate the current mode
    setMode(m_mode);
    m_range = range;

    // set initial values
    switch (m_mode) {
	case byTime: {
	    unsigned int t = (unsigned int)ceil(m_range);
	    sbMilliseconds->setValue(t % 1000);
	    t /= 1000;
	    sbSeconds->setValue(t % 60);
	    t /= 60;
	    sbMinutes->setValue(t % 60);
	    t /= 60;
	    sbMinutes->setValue(t % 60);
	    t /= 60;
	    sbHours->setValue(t);
	    break;
	}
	case bySamples: {
	    edSamples->setValue(m_range);
	    break;
	}
	case byPercents: {
	    sbPercents->setValue((int)rint(m_range));
	    break;
	}
    }

    // connect mode controls
    connect(rbTime, SIGNAL(stateChanged(int)), this, SLOT(modeChanged(int)));
    connect(rbSamples,SIGNAL(stateChanged(int)),this,SLOT(modeChanged(int)));
    connect(rbPercents,SIGNAL(stateChanged(int)),this,SLOT(modeChanged(int)));

    // connect the time controls
    connect(sbMilliseconds, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbSeconds, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbMinutes, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(sbHours, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));

    // connect sample rate control
    connect(edSamples, SIGNAL(valueChanged(double)),
            this, SLOT(samplesChanged(double)));

    // connect percentage control
    IntValidatorProxy *px = new IntValidatorProxy(this);
    sbPercents->setValidator(px);
    connect(px, SIGNAL(valueChanged(int)),
            this, SLOT(percentsChanged(int)));
    connect(sbPercents, SIGNAL(valueChanged(int)),
            this, SLOT(percentsChanged(int)));

    // connect the timer for the sample edit
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkNewSampleEdit()));

    // update all controls
    switch (m_mode) {
	case byTime:
	    timeChanged(0);
	    break;
	case bySamples:
	    samplesChanged(0);
	    break;
	case byPercents:
	    percentsChanged((int)m_range);
	    break;
    }

    setFixedSize(sizeHint());
}

//***************************************************************************
SelectTimeWidget::~SelectTimeWidget()
{
}

//***************************************************************************
void SelectTimeWidget::setMode(Mode new_mode)
{
    // first disable all modes
    rbTime->setChecked(false);
    rbSamples->setChecked(false);
    rbPercents->setChecked(false);

    // then enable the selected one
    m_mode = new_mode;
    switch (new_mode) {
	case byTime:
	    rbTime->setChecked(true);
	    break;
	case bySamples:
	    rbSamples->setChecked(true);
	    break;
	case byPercents:
	    rbPercents->setChecked(true);
	    break;
    }

}

//***************************************************************************
void SelectTimeWidget::modeChanged(int enable)
{
    if (!enable) return; // ignore disabling of radio buttons

    if (rbTime->isChecked() && (m_mode != byTime)) {
	m_mode = byTime;
	rbSamples->setChecked(false);
	rbPercents->setChecked(false);
	timeChanged(0); // (sets m_range)
    }

    if (rbSamples->isChecked() && (m_mode != bySamples)) {
	m_mode = bySamples;
	rbTime->setChecked(false);
	rbPercents->setChecked(false);
	samplesChanged(0); // (sets m_range)
	
	if (rbTime->isChecked()) {
	    m_timer.stop();
	} else {
	    // activate the sample edit timer
	    m_timer.start(100, false);
	}

    }

    if (rbPercents->isChecked() && (m_mode != byPercents)) {
	m_mode = byPercents;
	rbTime->setChecked(false);
	rbSamples->setChecked(false);
	percentsChanged(sbPercents->value()); // (sets m_range)
    }

}

//***************************************************************************
void SelectTimeWidget::timeChanged(int)
{
    static bool recursive = false;
    if (recursive) return; // don't do recursive processing
    if (m_mode != byTime) return;
    recursive = true;

    // get current time and correct wrap-overs
    int milliseconds = sbMilliseconds->value();
    int seconds = sbSeconds->value();
    int minutes = sbMinutes->value();
    int hours = sbHours->value();

    if (milliseconds < 0) {
	milliseconds = 999;
	seconds--;
    }
    if (seconds < 0) {
	seconds = 59;
	minutes--;
    }
    if (minutes < 0) {
	minutes = 59;
	hours--;
    }
    if (hours < 0) {
	hours = 0;
	minutes = 0;
	seconds = 0;
	milliseconds = 0;
    }
    ASSERT((hours>=0) && (minutes>=0) && (seconds>=0) && (milliseconds>=0));

    int ms =
        (int)milliseconds +
        ((int)seconds + ((int)minutes +
        ((int)hours * 60L)) * 60L) * 1000L;
    if (ms < 0) ms = 0;

    // limit time
    long int max_ms = (long int)ceil(((m_length - m_offset)*1E3) / m_rate);
    if (ms > max_ms) ms = max_ms;
    long int t = ms;

    milliseconds = t % 1000L;
    t /= 1000L;
    seconds = t % 60L;
    t /= 60L;
    minutes = t % 60L;
    hours = t / 60L;

    sbMilliseconds->setValue(milliseconds);
    sbSeconds->setValue(seconds);
    sbMinutes->setValue(minutes);
    sbHours->setValue(hours);

    // update the other widgets
    edSamples->setValue(ceil((double)ms * m_rate * 1E-3));
    sbPercents->setValue((int)(100.0 * (double)ms / (m_length/m_rate*1E3)));

    // set range in byTime mode [ms]
    m_range = ms;

    recursive = false;
}

//***************************************************************************
void SelectTimeWidget::checkNewSampleEdit()
{
    static double last_samples = -1.0;
    if (edSamples->value() != last_samples) {
	last_samples = edSamples->value();
	samplesChanged(last_samples);
    }
}

//***************************************************************************
void SelectTimeWidget::samplesChanged(double)
{
    if (m_mode != bySamples) return;

    unsigned int max_samples = m_length - m_offset;
    unsigned int samples = (unsigned int)ceil(edSamples->value());

    // limit the current value
    if (samples > max_samples) samples = max_samples;

    // update the other widgets
    unsigned int t = (unsigned int)ceil((double)samples * 1E3 / m_rate);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    double percents = 100.0*(double)samples/(double)(m_length-m_offset);
    sbPercents->setValue((int)percents);

    // update in samples mode
    m_range = samples;

    // re-activate the sample edit timer
    m_timer.stop();
    m_timer.start(100, false);
}

//***************************************************************************
void SelectTimeWidget::percentsChanged(int p)
{
    if (m_mode != byPercents) return;

    // get value
    double percents = p;;
    if (slidePercents->value() != p) slidePercents->setValue(p);

    // limit to rest of signal
    double max_percents = 100.0 * (double)(m_length-m_offset)/(double)m_length;
    if (percents > max_percents) percents = max_percents;

    // update the other widgets
    double samples = (double)m_length * percents / 100.0;
    edSamples->setValue(samples);

    unsigned int t = (unsigned int)ceil(samples * 1E3 / m_rate);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    // update in byPercents mode [0...100]
    m_range = percents;
}

//***************************************************************************
//***************************************************************************
