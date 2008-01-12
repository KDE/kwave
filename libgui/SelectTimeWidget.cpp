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
#include <limits.h>

#include <QRadioButton>
#include <QSlider>
#include <QWidget>

#include <klocale.h>
#include <knuminput.h>

#include "IntValidatorProxy.h"
#include "SelectTimeWidget.h"

//***************************************************************************
SelectTimeWidget::SelectTimeWidget(QWidget *widget)
    :QGroupBox(widget), Ui::SelectTimeWidgetBase(),
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

    Q_ASSERT(m_rate);
    Q_ASSERT(m_length);
    Q_ASSERT(rbTime);
    Q_ASSERT(rbSamples);
    Q_ASSERT(rbPercents);
    Q_ASSERT(m_offset < m_length);
    if (!m_rate) m_rate = 1.0;
    if (!m_length) m_length = 1;

    // set range of selection by sample
    edSamples->setSliderEnabled(false);
    edSamples->setRange(0, m_length-m_offset, 1);

    // set range of time controls
    int t = (int)rint((m_length * 1E3) / m_rate);
    sbMilliseconds->setMaximum(t);
    t /= 1000;
    sbSeconds->setMaximum(t);
    t /= 60;
    sbMinutes->setMaximum(t);
    t /= 60;
    sbHours->setMaximum(t);

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
	    sbHours->setValue(t);
	    break;
	}
	case bySamples: {
            unsigned int samples = (unsigned int)rint(m_range);
	    Q_ASSERT(samples <= INT_MAX);
	    if (samples > INT_MAX) samples = INT_MAX;
	    edSamples->setValue((int)samples);
	    break;
	}
	case byPercents: {
	    sbPercents->setValue((int)rint(m_range));
	    break;
	}
    }

    // connect mode controls
    QObject::connect(rbTime, SIGNAL(stateChanged(int)),
                     this, SLOT(modeChanged(int)));
    QObject::connect(rbSamples,SIGNAL(stateChanged(int)),
                     this, SLOT(modeChanged(int)));
    QObject::connect(rbPercents,SIGNAL(stateChanged(int)),
                     this, SLOT(modeChanged(int)));

    connect();

    // connect percentage control
// ### TODO ###
//     IntValidatorProxy *px = new IntValidatorProxy(this);
//     sbPercents->setValidator(px);
//     QObject::connect(px, SIGNAL(valueChanged(int)),
//                      this, SLOT(percentsChanged(int)));
    QObject::connect(sbPercents, SIGNAL(valueChanged(int)),
                     this, SLOT(percentsChanged(int)));

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

    adjustSize();
    setMinimumSize(sizeHint());
}

//***************************************************************************
SelectTimeWidget::~SelectTimeWidget()
{
}

//***************************************************************************
void SelectTimeWidget::connect()
{
    // connect the time controls
    QObject::connect(sbMilliseconds, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
    QObject::connect(sbSeconds, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
    QObject::connect(sbMinutes, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
    QObject::connect(sbHours, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));

    // connect sample count control
    QObject::connect(edSamples, SIGNAL(valueChanged(int)),
                     this, SLOT(samplesChanged(int)));

    // connect the timer for the sample edit
    QObject::connect(&m_timer, SIGNAL(timeout()),
                     this, SLOT(checkNewSampleEdit()));

}

//***************************************************************************
void SelectTimeWidget::disconnect()
{
    // disconnect the time controls
    QObject::disconnect(sbMilliseconds, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
    QObject::disconnect(sbSeconds, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
    QObject::disconnect(sbMinutes, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
    QObject::disconnect(sbHours, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));

    // disconnect sample count control
    QObject::disconnect(edSamples, SIGNAL(valueChanged(int)),
                        this, SLOT(samplesChanged(int)));

    // disconnect the timer for the sample edit
    QObject::disconnect(&m_timer, SIGNAL(timeout()),
                        this, SLOT(checkNewSampleEdit()));

}

//***************************************************************************
void SelectTimeWidget::setMode(Mode new_mode)
{
    // first disable all modes
    rbTime->setChecked(false);
    rbSamples->setChecked(false);
    rbPercents->setChecked(false);

    // then enable the selected one
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
    Q_ASSERT(m_mode == new_mode);

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
	    m_timer.setSingleShot(false);
	    m_timer.start(100);
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
    if (m_mode != byTime) return;
    disconnect();

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
    Q_ASSERT((hours>=0) && (minutes>=0) && (seconds>=0) && (milliseconds>=0));

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
    unsigned int samples = (unsigned int)ceil((double)ms * m_rate * 1E-3);
    Q_ASSERT(samples <= INT_MAX);
    if (samples > INT_MAX) samples = INT_MAX;
    edSamples->setValue((int)samples);
    sbPercents->setValue((int)(100.0 * (double)ms / (m_length/m_rate*1E3)));

    // set range in byTime mode [ms]
    m_range = ms;

    emit valueChanged(samples); // emit the change
    connect();
}

//***************************************************************************
void SelectTimeWidget::checkNewSampleEdit()
{
    static int last_samples = -1;
    if (edSamples->value() != last_samples) {
	last_samples = edSamples->value();
	samplesChanged(last_samples);
    }
}

//***************************************************************************
void SelectTimeWidget::samplesChanged(int)
{
    if (m_mode != bySamples) return;
    disconnect();

    unsigned int max_samples = m_length - m_offset;
    unsigned int samples = edSamples->value();

    // limit the current value
    if (samples > max_samples) samples = max_samples;

    // update the other widgets
    unsigned int t = (unsigned int)rint((double)samples * 1E3 / m_rate);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    double percents = (100.0 * (double)samples) / (double)(m_length);
    sbPercents->setValue((int)rint(percents));

    // update in samples mode
    m_range = samples;

    // re-activate the sample edit timer
    m_timer.stop();
    m_timer.setSingleShot(false);
    m_timer.start(100);

    emit valueChanged(samples); // emit the change
    connect();
}

//***************************************************************************
void SelectTimeWidget::percentsChanged(int p)
{
    if (m_mode != byPercents) return;
    disconnect();

    // get value
    double percents = p;

    // limit to rest of signal
    double max_percents = 100.0 * (double)(m_length-m_offset)/(double)m_length;
    if (percents > max_percents) {
	percents = max_percents;
	p = (int)percents;
    }

    // update in byPercents mode [0...100]
    m_range = percents;
    p = (int)percents;

    if (slidePercents->value() != p) slidePercents->setValue(p);
    if (sbPercents->value() != p) sbPercents->setValue(p);

    // update the other widgets
    unsigned int samples = (unsigned int)((double)m_length*percents/100.0);
    Q_ASSERT(samples <= INT_MAX);
    if (samples > INT_MAX) samples = INT_MAX;
    edSamples->setValue(samples);

    unsigned int t = (unsigned int)ceil(((double)samples * 1E3) / m_rate);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    emit valueChanged((unsigned int)samples); // emit the change
    connect();
}

//***************************************************************************
void SelectTimeWidget::setTitle(const QString title)
{
    QGroupBox::setTitle(title);
}

//***************************************************************************
void SelectTimeWidget::setOffset(unsigned int offset)
{
    m_offset = offset;
    unsigned int max_samples = m_length - m_offset;
    unsigned int samples = edSamples->value();

    // the range of the sample edit should always get updated
    edSamples->setSliderEnabled(false);
    edSamples->setRange(0, m_length-m_offset, 1);

    // no range conflict -> nothing to do
    if (samples <= max_samples) return;

    // limit the length to the rest
    samples = max_samples;

    // update all widgets
    disconnect();

    int t = (int)ceil(((double)samples * 1E3) / m_rate);
    sbMilliseconds->setValue(t);
    t /= 1000;
    sbSeconds->setValue(t);
    t /= 60;
    sbMinutes->setValue(t);
    t /= 60;
    sbHours->setValue(t);

    Q_ASSERT(samples <= INT_MAX);
    if (samples > INT_MAX) samples = INT_MAX;
    edSamples->setValue(samples);

    double percents = 100.0*(double)samples/(double)(m_length);
    sbPercents->setValue((int)percents);

    connect();
}

//***************************************************************************
#include "SelectTimeWidget.moc"
//***************************************************************************
//***************************************************************************
