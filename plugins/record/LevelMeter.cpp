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

#include <qapplication.h>
#include <qbrush.h>
#include <qcolor.h>
#include <qfont.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <klocale.h>

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
    m_current_fast(), m_current_peak(), m_timer(0), m_pixmap(0),
    m_color_low(Qt::green),
    m_color_normal(Qt::yellow),
    m_color_high(Qt::red)
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
void LevelMeter::drawScale(QPainter &p)
{
    // draw the levels in 3dB steps, like -12dB -9dB  -6dB  -3dB and 0dB
    QFont f = qApp->font(this);
    QFontMetrics fm = p.fontMetrics();
    QRect rect = fm.boundingRect(i18n("-999 dB"));

    const int border = 4;
    const int w = width() - 2*border;
    const int tw = rect.width();
    const int th = rect.height();
    const int y = ((height() - th) / 2) + th;
    int db = 0;
    int right = this->width();
    const QColor textcolor = colorGroup().buttonText();

    QImage img = m_pixmap->convertToImage();
    while (right > tw + border) {
	// find the first position in dB which is not overlapping
	// the last output position
	QString txt;
	int x;
	do {
	    txt = i18n("%1 dB").arg(db);
	    x = (int)((double)w * pow10((double)db/(double)20.0));
	    db -= 3; // one step left == -3dB
	} while ((x > right) && (x >= tw));
	if (x < tw) break;

	// calculate the text position
	int text_width = fm.boundingRect(txt).width();
	x += border;
	x -= text_width + 3;

	// create the text background mask, a rounded rectangle, by drawing
	// directly into the original pixmap. This area is later replaced
	// with the transparent text background, so it is ok to destroy the
	// original pixmap.
	const int r=5;
	QPixmap mask(r+text_width+r, r+th+r);
	mask.fill(colorGroup().background());
	QPainter p2;
	p2.begin(&mask);
	QBrush brush(textcolor);
	p2.setBrush(brush);
	p2.setPen(Qt::NoPen);
	p2.drawRoundRect(0, 0, text_width+2*r, th+2*r,
	                 (200*r)/th, (200*r)/th);
	p2.end();
	QImage img2 = mask.convertToImage();

	// draw the text background back into the image, using a 50%
	// transparency (pixel by pixel as Qt currently does not support
	// drawing transparent rounded rectangles)
	QColor c2 = colorGroup().background();
	int r2,g2,b2;
	c2.getRgb(&r2, &g2, &b2);
	for (int y1=y-th-r; y1 < y+r; y1++) {
	    for (int x1=x-r; x1 < x+text_width+r; x1++) {
		QColor c1 = img2.pixel(x1-(x-r), y1-(y-th-r));
		if (c1 != textcolor)
		    continue;

		c1 = img.pixel(x1, y1);
		int r1,g1,b1;
		c1.getRgb(&r1, &g1, &b1);
		r1 = (r1 + r2) / 2;
		g1 = (g1 + g2) / 2;
		b1 = (b1 + b2) / 2;
		c1.setRgb(r1, g1, b1);
		img.setPixel(x1, y1, c1.rgb());
	    }
	}
	// replace the area in the pixmap that has been destroyed by
	// creating the mask with the image which contains the original
	// pixmap overlaid with 50% transparent text background
	p.drawImage(x-r, y-th-r, img, x-r, y-th-r, text_width+2*r, th+2*r);

	// draw the text, right/center aligned
	p.setPen(textcolor);
	p.drawText(x, y, txt);

	// new right border == one character left from last one
	right = x - th;
    }

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
    if (m_pixmap && m_pixmap->size() != size()) {
	delete m_pixmap;
	m_pixmap = 0;
    }
    if (!m_pixmap) m_pixmap = new QPixmap(size());
    Q_ASSERT(m_pixmap);
    if (!m_pixmap) return;

    p.begin(m_pixmap);

    // fill the background
    p.fillRect(rect(), colorGroup().background());

    const unsigned int border = 4;
    const unsigned int cell = 3;
    const unsigned int w = width() - border * 2 - cell * 2;
    const unsigned int h = (height() - border) / (m_tracks ? m_tracks : 1);

    const unsigned int w_low  = (int)(w * 0.7);  // -3 dB
    const unsigned int w_high = (int)(w * 0.85); // -1.5dB

    for (track=0; track < m_tracks; track++) {
	// show a bar up to the "fast" value
	const unsigned int fast = (unsigned int)(m_current_fast[track] * w);
	for (unsigned int i = 0; i < w; i += cell * 2) {
	    QColor color;
	    if (i >= w_high)
		color = m_color_high;
	    else if (i >= w_low)
		color = m_color_normal;
	    else
		color = m_color_low;

	    p.fillRect(
		border + cell + i,
		border + (track*h),
		cell, h-border,
		(i > fast) ? color.dark() : color
	    );
	}

	// draw the peak value
	unsigned int peak = (unsigned int)(m_current_peak[track] * w);
	QColor peak_color;
	if (peak >= w_high)
	    peak_color = m_color_high;
	else if (peak >= w_low)
	    peak_color = m_color_normal;
	else
	    peak_color = m_color_low;

	p.fillRect(
	    border + cell + peak,
	    border + (track*h),
	    cell, h-border,
	    peak_color.light()
	);
    }

    // draw the scale / dB numbers
    drawScale(p);

    p.end();
    bitBlt(this, 0, 0, m_pixmap);
}

//***************************************************************************
#include "LevelMeter.moc"
//***************************************************************************
//***************************************************************************
