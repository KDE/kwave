/*************************************************************************
         LevelMeter.cpp  -  multi-track audio level meter
                             -------------------
    original copyright   : Copyright 2002 Rik Hemsley (rikkus) <rik@kde.org>

    begin                : Mon Nov 17 2003
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

#include <math.h>

#include <qbrush.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "LevelMeter.h"

/** number of display updates per second */
static const float UPDATES_PER_SECOND = 8.0;

/** lowpass frequency used for rising the current (fast) value [Hz] */
static const float F_FAST_RISE = (20.0);

/** lowpass frequency used for decay of the current (fast) value [Hz] */
static const float F_FAST_DECAY = (0.5);

/** lowpass frequency used for rising the peak values */
static const float F_PEAK_RISE = (F_FAST_RISE);

/** lowpass frequency used for decay of peak values [Hz] */
static const float F_PEAK_DECAY = (0.005);

//***************************************************************************
LevelMeter::LevelMeter(QWidget *parent, const char *name)
    :QWidget(parent, name,
             WRepaintNoErase | WResizeNoErase | WPaintUnclipped),
    m_tracks(0), m_sample_rate(0), m_yf(), m_yp(),
    m_fast_queue(), m_peak_queue(),
    m_current_fast(), m_current_peak(), m_timer(0),
    m_pixmap(0),
    m_empty_color(colorGroup().background()),
    m_value_color(colorGroup().highlight()),
    m_peak_color(colorGroup().foreground())
{
    m_timer = new QTimer(this);
    Q_ASSERT(m_timer);
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(timedUpdate()));
}

//***************************************************************************
LevelMeter::~LevelMeter()
{
    if (m_pixmap) delete m_pixmap;
    m_pixmap = 0;
    setTracks(0);
}

//***************************************************************************
void LevelMeter::paintEvent(QPaintEvent *)
{
    drawContents();
}

//***************************************************************************
void LevelMeter::resizeEvent(QResizeEvent *)
{
    drawContents();
}

//***************************************************************************
void LevelMeter::setTracks(unsigned int tracks)
{
    if (tracks == m_tracks) return;
    m_tracks = tracks;
    reset(); // re-create all arrays etc.
}

//***************************************************************************
void LevelMeter::setSampleRate(double rate)
{
    if ((float)rate == m_sample_rate) return;
    m_sample_rate = (float)rate;
}

//***************************************************************************
void LevelMeter::updateTrack(unsigned int track, QMemArray<sample_t> &buffer)
{
    Q_ASSERT(track < m_tracks);
    if (track >= m_tracks) return;

    // calculate the number of samples per update (approx)
    const unsigned int samples = buffer.size();
    const unsigned int samples_per_update =
        (unsigned int)rint(ceil(m_sample_rate/UPDATES_PER_SECOND));
    unsigned int next_fraction = samples_per_update;
    const unsigned int queue_depth = ((samples / samples_per_update) + 2);

    /* fast update: rise */
    float Fg = F_FAST_RISE / m_sample_rate;
    float n = 1.0 / tan(M_PI * Fg);
    const float a0_fr = 1.0 / (1.0 + n);
    const float b1_fr = (1.0 - n) / (1.0 + n);

    /* fast update: decay */
    Fg = F_FAST_DECAY / m_sample_rate;
    n = 1.0 / tan(M_PI * Fg);
    const float a0_fd = 1.0 / (1.0 + n);
    const float b1_fd = (1.0 - n) / (1.0 + n);

    /* peak value: rise */
    Fg = F_PEAK_RISE / m_sample_rate;
    n = 1.0 / tan(M_PI * Fg);
    const float a0_pr = 1.0 / (1.0 + n);
    const float b1_pr = (1.0 - n) / (1.0 + n);

    /* peak value: decay */
    Fg = F_PEAK_DECAY / m_sample_rate;
    n = 1.0 / tan(M_PI * Fg);
    const float a0_pd = 1.0 / (1.0 + n);
    const float b1_pd = (1.0 - n) / (1.0 + n);

    float yf = m_yf[track];
    float yp = m_yp[track];
    float last_x = yf;
    for (unsigned int t=0; t < samples; ++t) {
	float x = fabs(sample2float(buffer[t])); /* rectifier */

	/* fast value */
	if (x > yf) yf = (a0_fr * x) + (a0_fr * last_x) - (b1_fr * yf); // rise
	yf = (a0_fd * x) + (a0_fd * last_x) - (b1_fd * yf); // decay

	/* peak value */
	if (x > yp) yp = (a0_pr * x) + (a0_pr * last_x) - (b1_pr * yp); // rise
	yp = (a0_pd * x) + (a0_pd * last_x) - (b1_pd * yp); // decay

	// remember x[t-1]
	last_x = x;

	// enqueue new values if limit reached
	if ((t > next_fraction) || (t == samples-1)) {
	    next_fraction += samples_per_update;

	    // merge the last fractional part to the last normal part
	    if ((next_fraction + samples_per_update) > samples)
	       next_fraction = samples-1;

	    enqueue(track, yf, yp, queue_depth);
	}
    }
    m_yf[track] = yf;
    m_yp[track] = yp;
}

//***************************************************************************
void LevelMeter::reset()
{
    if (m_timer && m_timer->isActive()) m_timer->stop();

    m_yf.resize(m_tracks);
    m_yf.fill(0.0);
    m_fast_queue.resize(m_tracks);
    m_current_fast.resize(m_tracks);
    m_current_fast.fill(0.0);

    m_yp.resize(m_tracks);
    m_yp.fill(0.0);
    m_peak_queue.resize(m_tracks);
    m_current_peak.resize(m_tracks);
    m_current_peak.fill(0.0);
}

//***************************************************************************
void LevelMeter::enqueue(unsigned int track, float fast, float peak,
                         unsigned int queue_depth)
{
    Q_ASSERT(track < m_tracks);
    Q_ASSERT(m_peak_queue.size() == m_fast_queue.size());
    Q_ASSERT(m_fast_queue.size() >= m_tracks);
    Q_ASSERT(m_peak_queue.size() >= m_tracks);
    if ((track >= m_tracks) || (m_fast_queue.size() < m_tracks) ||
        (m_peak_queue.size() < m_tracks)) return;
    Q_ASSERT(m_peak_queue[track].size() == m_fast_queue[track].size());
    if (m_peak_queue[track].size() != m_fast_queue[track].size()) return;

    // remove old entries
    while (m_fast_queue[track].size() > queue_depth) {
//	qDebug("LevelMeter::enqueue(): purging old entry (%u/%u)",
//	       m_fast_queue.size(), queue_depth);
	m_fast_queue[track].erase(&m_fast_queue[track].first());
	m_peak_queue[track].erase(&m_peak_queue[track].first());
    }

    // append to the queue's end
    m_fast_queue[track].append(fast);
    m_peak_queue[track].append(peak);

    // restart the timer if necessary
   if (m_timer && !m_timer->isActive())
      m_timer->start((int)(1000 / UPDATES_PER_SECOND), false);
}

//***************************************************************************
bool LevelMeter::dequeue(unsigned int track, float &fast, float &peak)
{
    Q_ASSERT(m_peak_queue.size() == m_fast_queue.size());
    Q_ASSERT(m_fast_queue.size() >= m_tracks);
    Q_ASSERT(m_peak_queue.size() >= m_tracks);
    if ((track >= m_tracks) || (m_fast_queue.size() < m_tracks) ||
        (m_peak_queue.size() < m_tracks)) return false;
    Q_ASSERT(m_peak_queue[track].size() == m_fast_queue[track].size());
    if (m_peak_queue[track].size() != m_fast_queue[track].size())
        return false;

    // check if the queues are empty
    if (m_fast_queue[track].isEmpty()) return false;
    if (m_peak_queue[track].isEmpty()) return false;

    // get the values from the front of the queue
    fast = m_fast_queue[track].first();
    peak = m_peak_queue[track].first();

    // remove the values from the front of the queue
    m_fast_queue[track].erase(&m_fast_queue[track].first());
    m_peak_queue[track].erase(&m_peak_queue[track].first());

    return true;
}

//***************************************************************************
void LevelMeter::timedUpdate()
{
    float fast;
    float peak;
    bool need_update = false;

    for (unsigned int track=0; track < m_tracks; track++) {
	if (dequeue(track, fast, peak)) {
	    // set the new "current" values
	    m_current_fast[track] = fast;
	    m_current_peak[track] = peak;

	    // remember that we have to update the display
	    need_update = true;
	}
    }

    // refresh the display if needed
    if (need_update) drawContents();
}

//***************************************************************************
/*
  Original idea:
  Copyright 2002 Rik Hemsley (rikkus) <rik@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
*/
void LevelMeter::drawContents()
{
    QPainter p;
    unsigned int track;

    Q_ASSERT(width() > 0);
    Q_ASSERT(height() > 0);

    // if pixmap has to be resized ...
    if (!m_pixmap) m_pixmap = new QPixmap(size());
    Q_ASSERT(m_pixmap);
    if (!m_pixmap) return;

    p.begin(m_pixmap);
    p.fillRect(rect(), m_empty_color);

    const unsigned int border = 4;
    const unsigned int cell = 3;
    const unsigned int w = width() - border * 2 - cell * 2;
    const unsigned int h = (height() - border) / (m_tracks ? m_tracks : 1);

    for (track=0; track < m_tracks; track++) {
	// show a bar up to the "fast" value
	const unsigned int fast = (unsigned int)(m_current_fast[track] * w);
	for (unsigned int i = 0; i < w; i += cell * 2) {
	    p.fillRect(
		border + cell + i,
		border + (track*h),
		cell, h-border,
		(i < fast) ? m_value_color : m_empty_color
	    );
	}

	// draw the peak value
	unsigned int peak = (unsigned int)(m_current_peak[track] * w);
	p.fillRect(
	    border + cell + peak,
	    border + (track*h),
	    cell, h-border,
	    m_peak_color
	);
    }

    p.end();
    bitBlt(this, 0, 0, m_pixmap);
}

//***************************************************************************
//***************************************************************************
