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

#include "config.h"

#include <math.h>
#include <limits.h>

#include <QRadioButton>
#include <QSlider>
#include <QWidget>

#include <klocale.h>
#include <knuminput.h>

#include "SelectTimeWidget.h"

//***************************************************************************
Kwave::SelectTimeWidget::SelectTimeWidget(QWidget *widget)
    :QGroupBox(widget), Ui::SelectTimeWidgetBase(),
     m_mode(bySamples), m_range(0), m_rate(1.0), m_offset(0), m_length(0),
     m_timer(this)
{
    setupUi(this);
    modeChanged(true);
}

//***************************************************************************
void Kwave::SelectTimeWidget::init(Mode mode, unsigned int range,
                                   double sample_rate, sample_index_t offset,
                                   sample_index_t signal_length)
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
    int t = static_cast<int>(rint((m_length * 1E3) / m_rate));
    sbMilliseconds->setMaximum(t);
    t /= 1000;
    sbSeconds->setMaximum(t);
    t /= 60;
    sbMinutes->setMaximum(t);
    t /= 60;
    sbHours->setMaximum(t);

    // activate the current mode
    setMode(mode);
    m_range = range;

    // set initial values
    switch (m_mode) {
	case byTime: {
	    unsigned int t = m_range;
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
            unsigned int samples = m_range;
	    Q_ASSERT(samples <= INT_MAX);
	    if (samples > INT_MAX) samples = INT_MAX;
	    edSamples->setValue(static_cast<int>(samples));
	    break;
	}
	case byPercents: {
	    sbPercents->setValue(static_cast<int>(m_range));
	    break;
	}
    }

    // connect mode controls
    QObject::connect(rbTime, SIGNAL(toggled(bool)),
                     this, SLOT(modeChanged(bool)));
    QObject::connect(rbSamples,SIGNAL(toggled(bool)),
                     this, SLOT(modeChanged(bool)));
    QObject::connect(rbPercents,SIGNAL(toggled(bool)),
                     this, SLOT(modeChanged(bool)));

    connect();

    // connect percentage control
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
	    percentsChanged(static_cast<int>(m_range));
	    break;
    }

    adjustSize();
    setMinimumSize(sizeHint());
    modeChanged(true);
}

//***************************************************************************
Kwave::SelectTimeWidget::~SelectTimeWidget()
{
}

//***************************************************************************
void Kwave::SelectTimeWidget::connect()
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
void Kwave::SelectTimeWidget::disconnect()
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
void Kwave::SelectTimeWidget::setMode(Mode new_mode)
{
    // enable the selected mode
    switch (new_mode) {
	case byTime:
	    rbTime->setChecked(true);
	    Q_ASSERT(rbTime->isChecked());
	    Q_ASSERT(!rbSamples->isChecked());
	    Q_ASSERT(!rbPercents->isChecked());
	    break;
	case bySamples:
	    rbSamples->setChecked(true);
	    Q_ASSERT(!rbTime->isChecked());
	    Q_ASSERT(rbSamples->isChecked());
	    Q_ASSERT(!rbPercents->isChecked());
	    break;
	case byPercents:
	    rbPercents->setChecked(true);
	    Q_ASSERT(!rbTime->isChecked());
	    Q_ASSERT(!rbSamples->isChecked());
	    Q_ASSERT(rbPercents->isChecked());
	    break;
    }
    m_mode = new_mode;
}

//***************************************************************************
void Kwave::SelectTimeWidget::modeChanged(bool checked)
{
    if (!checked) return; // ignore disabling of radio buttons

    if (rbTime->isChecked()) {
	m_mode = byTime;
	rbSamples->setChecked(false);
	rbPercents->setChecked(false);
	timeChanged(0); // (sets m_range)
    }

    if (rbSamples->isChecked()) {
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

    if (rbPercents->isChecked()) {
	m_mode = byPercents;
	rbTime->setChecked(false);
	rbSamples->setChecked(false);
	percentsChanged(sbPercents->value()); // (sets m_range)
    }

}

//***************************************************************************
void Kwave::SelectTimeWidget::timeChanged(int)
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
        milliseconds +
        (seconds + (minutes +
        (hours * 60)) * 60) * 1000;
    if (ms < 0) ms = 0;

    // limit time
    long int max_ms = static_cast<long int>(
	ceil(((m_length - m_offset) * 1E3) / m_rate));
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
    int samples = timeToSamples(byTime, ms, m_rate, m_length);
    edSamples->setValue(samples);
    int percents = samplesToTime(byPercents, samples, m_rate, m_length);
    sbPercents->setValue(percents);

    // set range in byTime mode [ms]
    m_range = ms;

    emit valueChanged(samples); // emit the change
    connect();
}

//***************************************************************************
void Kwave::SelectTimeWidget::checkNewSampleEdit()
{
    static int last_samples = -1;
    if (edSamples->value() != last_samples) {
	last_samples = edSamples->value();
	samplesChanged(last_samples);
    }
}

//***************************************************************************
void Kwave::SelectTimeWidget::samplesChanged(int)
{
    if (m_mode != bySamples) return;
    disconnect();

    unsigned int max_samples = m_length - m_offset;
    unsigned int samples = edSamples->value();

    // limit the current value
    if (samples > max_samples) samples = max_samples;

    // update the other widgets
    unsigned int t = samplesToTime(byTime, samples, m_rate, m_length);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    int percents = samplesToTime(byPercents, samples, m_rate, m_length);
    sbPercents->setValue(percents);

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
void Kwave::SelectTimeWidget::percentsChanged(int p)
{
    if (m_mode != byPercents) return;
    disconnect();

    // limit to rest of signal
    int max_percents = 100;
    if (m_length)
	max_percents = (100 * static_cast<quint64>(m_length - m_offset)) /
	static_cast<quint64>(m_length);
    if (p > max_percents) {
	p = max_percents;
    }

    // update in byPercents mode [0...100]
    m_range = p;

    if (slidePercents->value() != p) slidePercents->setValue(p);
    if (sbPercents->value() != p) sbPercents->setValue(p);

    // update the other widgets
    int samples = timeToSamples(byPercents, p, m_rate, m_length);
    edSamples->setValue(samples);

    unsigned int t = samplesToTime(byTime, samples, m_rate, m_length);
    sbMilliseconds->setValue(t % 1000);
    t /= 1000;
    sbSeconds->setValue(t % 60);
    t /= 60;
    sbMinutes->setValue(t % 60);
    t /= 60;
    sbHours->setValue(t);

    emit valueChanged(samples); // emit the change
    connect();
}

//***************************************************************************
void Kwave::SelectTimeWidget::setTitle(const QString title)
{
    QGroupBox::setTitle(title);
}

//***************************************************************************
void Kwave::SelectTimeWidget::setOffset(sample_index_t offset)
{
    m_offset = offset;
    sample_index_t max_samples = m_length - m_offset;
    sample_index_t samples = edSamples->value();

    // the range of the sample edit should always get updated
    edSamples->setSliderEnabled(false);
    edSamples->setRange(0, m_length-m_offset, 1);

    // no range conflict -> nothing to do
    if (samples <= max_samples) return;

    // limit the length to the rest
    samples = max_samples;

    // update all widgets
    disconnect();

    int t = static_cast<int>(ceil((static_cast<double>(samples) * 1E3) /
	m_rate));
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

    double percents = 100.0 * static_cast<double>(samples) /
	static_cast<double>(m_length);
    sbPercents->setValue(static_cast<int>(percents));

    connect();
}

//***************************************************************************
sample_index_t Kwave::SelectTimeWidget::samples() const
{
    return (edSamples) ? edSamples->value() : 0;
}

//***************************************************************************
sample_index_t Kwave::SelectTimeWidget::timeToSamples(
    Kwave::SelectTimeWidget::Mode mode, unsigned int time, double rate,
    sample_index_t length)
{
    sample_index_t pos = 0;
    switch (mode) {
	case Kwave::SelectTimeWidget::byTime:
	    // convert from ms to samples
	    pos = static_cast<sample_index_t>(ceil(
		static_cast<double>(time) * rate * 1E-3));
	    break;
	case Kwave::SelectTimeWidget::bySamples:
	    // simple case -> already in samples
	    pos = time;
	    break;
	case Kwave::SelectTimeWidget::byPercents:
	    // by percentage of whole signal
	    pos = static_cast<unsigned int>(rint(
		static_cast<double>(length * (time / 100.0))));
	    break;
    }

    if (pos > INT_MAX) pos = INT_MAX;
    return pos;
}

//***************************************************************************
unsigned int Kwave::SelectTimeWidget::samplesToTime(
    Mode mode, sample_index_t samples, double rate, sample_index_t length)
{
    unsigned int time = 0;

    switch (mode) {
	case Kwave::SelectTimeWidget::byTime:
	    // convert from samples to ms
	    time = static_cast<unsigned int>(
		rint(static_cast<double>(samples) * 1E3 / rate));
	    break;
	case Kwave::SelectTimeWidget::bySamples:
	    // simple case -> already in samples
	    time = samples;
	    break;
	case Kwave::SelectTimeWidget::byPercents:
	    // by percentage of whole signal
	    time = static_cast<unsigned int>(100.0 *
		static_cast<double>(samples) /
		static_cast<double>(length));
	    break;
    }

    return time;
}

//***************************************************************************
#include "SelectTimeWidget.moc"
//***************************************************************************
//***************************************************************************
