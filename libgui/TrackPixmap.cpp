/***************************************************************************
        TrackPixmap.cpp  -  buffered pixmap for displaying a kwave track
                             -------------------
    begin                : Tue Mar 20 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#include <qpainter.h>

#include "mt/MutexGuard.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Track.h"
#include "libgui/TrackPixmap.h"

/**
 * This factor determines how many times the order is over than
 * the minimum required order. Higher values give less problems with
 * aliasing but some amplitude errors. Lower values make less
 * amplitude errors but more aliasing problems.
 * This should be a good compromise...
 */
#define INTERPOLATION_PRECISION 4

/**
 * math.h didn't define PI :-(
 */
#define PI 3.14159265358979323846264338327

//***************************************************************************
TrackPixmap::TrackPixmap(Track &track)
    :QPixmap(), m_track(track), m_offset(0), m_zoom(0.0),
    m_minmax_mode(false),
    m_sample_buffer(), m_min_buffer(), m_max_buffer(),
    m_valid(0), m_lock_buffer(),
    m_interpolation_order(0), m_interpolation_alpha(0)
{
    m_color_background = black;
    m_color_sample = white;
    m_color_interpolated = lightGray;
    m_color_zero = green;
}

//***************************************************************************
TrackPixmap::~TrackPixmap()
{
    MutexGuard lock(m_lock_buffer);

    if (m_interpolation_alpha) delete[] m_interpolation_alpha;
    m_interpolation_alpha = 0;
}

//***************************************************************************
void TrackPixmap::setOffset(unsigned int offset)
{
    MutexGuard lock(m_lock_buffer);
    if (offset == m_offset) return; // no change

    unsigned int diff;
    unsigned int src;
    unsigned int dst;
    unsigned int buflen = m_valid.size();

    if (m_minmax_mode) {
	// move content of min and max buffer
	// one buffer element = one screen pixel
	ASSERT(buflen == m_min_buffer.size() == m_max_buffer.size());

	// check for misaligned offset
	if (offset % pixels2samples(1)) {
	    warning("TrackPixmap::setOffset(): oh nooo, "\
	    	"offset %u is misaligned by %u sample(s), please fix this!",
	    	offset, offset % pixels2samples(1));
	    warning("TrackPixmap::setOffset(): "\
	    	"now I have to throw away the whole buffer :-((");
	    invalidateBuffer();
	} else if (offset > m_offset) {
	    // move left
	    diff = samples2pixels(offset - m_offset);
	    for (src = diff, dst = 0; src < buflen; ++dst, ++src) {
		m_min_buffer[dst] = m_min_buffer[src];
		m_max_buffer[dst] = m_max_buffer[src];
		m_valid[dst] = m_valid[src];
	    }
	    while (dst < buflen) m_valid.clearBit(dst++);
	} else {
	    // move right
	    diff = samples2pixels(m_offset - offset);
	    for (dst = buflen-1, src = dst-diff; dst >= diff; --dst, --src) {
		m_min_buffer[dst] = m_min_buffer[src];
		m_max_buffer[dst] = m_max_buffer[src];
		m_valid[dst] = m_valid[src];
	    }
	    while (diff--) m_valid[dst--] = 0;
	}
    } else {
	// move content of sample buffer
	// one buffer element = one sample
	ASSERT(buflen == m_sample_buffer.size());
	
	if (offset > m_offset) {
	    // move left
	    diff = offset - m_offset;
	    for (src = diff, dst = 0; src < buflen; ++dst, ++src) {
		m_sample_buffer[dst] = m_sample_buffer[src];
		m_valid[dst] = m_valid[src];
	    }
	    while (dst < buflen) m_valid[dst++] = 0;
	} else {
	    // move right
	    diff = m_offset - offset;
	    for (dst= buflen-1, src = dst-diff; dst >= diff; --dst, --src) {
		m_sample_buffer[dst] = m_sample_buffer[src];
		m_valid[dst] = m_valid[src];
	    }
	    while (diff--) m_valid.clearBit(dst--);
	}
    }

    m_offset = offset;
    repaint();
}

//***************************************************************************
void TrackPixmap::resizeBuffer()
{
    unsigned int buflen;
    unsigned int oldlen = m_valid.size();
    if (m_minmax_mode) {
	// one buffer index == one screen pixel
	buflen = width();
	m_min_buffer.resize(buflen);
	m_max_buffer.resize(buflen);
    } else {
	// one buffer index == one sample
	buflen = pixels2samples(width());
	m_sample_buffer.resize(buflen);
    }
    m_valid.resize(buflen);
    while (oldlen < buflen) m_valid[oldlen++] = 0;
}

//***************************************************************************
void TrackPixmap::setZoom(double zoom)
{
    MutexGuard lock(m_lock_buffer);

    if (zoom == m_zoom) return; // no change

    if ((zoom > 1.0) && !m_minmax_mode) {
	// switch to min/max mode
	debug("TrackPixmap::setZoom(): switch to min/max mode");
	invalidateBuffer();
	resizeBuffer();
	m_minmax_mode = true;
    } else if ((zoom <= 1.0) && m_minmax_mode) {
	// switch to normal mode
	debug("TrackPixmap::setZoom(): switch to normal mode");
	invalidateBuffer();
	resizeBuffer();
	m_minmax_mode = false;
    }

    // take the new zoom and resize the buffer
    m_zoom = zoom;
    resizeBuffer();

    repaint();
}

//***************************************************************************
//	    if (!m_signal_manager.isEmpty()) {
//		if (m_zoom < 0.1) {
//		    drawInterpolatedSignal(i, zero, track_height);
//		} else if (m_zoom <= 1.0)
//		    drawPolyLineSignal(i, zero, track_height);
//		else
//		    drawOverviewSignal(i, zero, track_height,
//		                       0, m_zoom*width);
//	    }
//
//	    // draw the baseline
//	    p.setPen(green);
//	    p.drawLine(0, zero, width, zero);
//	    p.setPen(white);
//	    zero += track_height;

//***************************************************************************
void TrackPixmap::resize(int width, int height)
{
    MutexGuard lock(m_lock_buffer);

    int old_width = QPixmap::width();
    int old_height = QPixmap::height();
    if ((old_width == width) && (old_height == height)) return; // no change

//    debug("TrackPixmap::resize(%d, %d)", width, height); // ###

    QPixmap::resize(width, height);
    if (width != old_width) resizeBuffer();

    repaint();
}

//***************************************************************************
void TrackPixmap::invalidateBuffer()
{
    for (unsigned int i=0; i < m_valid.size(); ++i) {
	m_valid.clearBit(i);
    }
}

//***************************************************************************
bool TrackPixmap::validateBuffer()
{
    unsigned int first = 0;
    unsigned int last = 0;
    unsigned int buflen = m_valid.size();

    if (m_minmax_mode) {
	ASSERT(m_min_buffer.size() == buflen);
	ASSERT(m_max_buffer.size() == buflen);
    } else {
	ASSERT(m_sample_buffer.size() == buflen);
    }

    while (first < buflen) {
	// find the first invalid index
	for (first=last; (first < buflen) && m_valid.testBit(first);
		++first);
	
	// break if the first index is out of range
	if (first >= buflen) return false; // buffer is already ok
	
	// find the last invalid index
	for (last=first; (last < buflen) && !m_valid.testBit(last);
		++last);
	if (last != first) --last;
	
	unsigned int offset;
	unsigned int length;
	if (m_minmax_mode) {
	    // one index == one pixel
	    offset = m_offset + pixels2samples(first);
	    length = pixels2samples(last-first+1);
	} else {
	    // one index == one sample
	    offset = m_offset + first;
	    length = last-first+1;
	}
	
	{   // lock the range that we need for shared read-only access
	    SampleLock lock(m_track, offset, length,
	                SampleLock::ReadShared);

	    // fill our array(s) with fresh sample data
	    // ###
	    debug("TrackPixmap::validateBuffer(): locked at %u, %u samples",
	    	offset, length);
	    	
	    if (m_minmax_mode) {
	    } else {
	    }
	
	    unsigned int pos;
	    for (pos = 0; pos < m_valid.size(); pos++) {
		double v = (1 << 23) * sin((double)pos*2.0*3.1415926535 /
			(double)m_valid.size());
		if (m_minmax_mode) {
		    m_min_buffer[pos] = (sample_t)(-v);
		    m_max_buffer[pos] = (sample_t)(+v);
		} else {
		    m_sample_buffer[pos] = (sample_t)v;
		}
	    }
	
	    // make the buffer valid
	    for (;first<=last;m_valid.setBit(first++));
	}

        ++last;
    }

    return true;
}

//***************************************************************************
void TrackPixmap::repaint()
{
    int w = width();
    int h = height();

    if (!w || !h) return; // not valid yet

    QPainter p(this);
    p.setRasterOp(CopyROP);
    p.fillRect(0, 0, w, h, m_color_background);

    if (m_zoom ) {
	// first make the buffer valid
	validateBuffer();
	
	// then draw the samples
	if (m_minmax_mode) {
	    drawOverview(h>>1, h, 0, w-1);
	} else {
	}
	// ###
	debug("TrackPixmap::repaint()");
	
	// draw the green zero-line
	p.setPen(m_color_zero);
	p.drawLine(0, h>>1, w-1, h>>1);
    }

    p.flush();
    p.end();
}

//***************************************************************************
void TrackPixmap::drawOverview(int middle, int height, int first, int last)
{
    ASSERT(m_minmax_mode);
    ASSERT(width() <= (int)m_min_buffer.size());
    ASSERT(width() <= (int)m_max_buffer.size());

    debug("TrackPixmap::drawOverview()");

    // scale_y: pixels per unit
    double scale_y = (double)height / (1 << 24);
    int max = 0, min = 0;

    QPainter p(this);
    p.setPen(m_color_sample);
    for (int i = first; i <= last; i++) {
	max = (int)(m_max_buffer[i] * scale_y);
	min = (int)(m_min_buffer[i] * scale_y);
	p.drawLine(i, middle - max, i, middle - min);
    }

    p.end();
}

//***************************************************************************
void TrackPixmap::calculateInterpolation()
{
    float f;
    float Fg;
    int k;
    int N;

//    debug("TrackPixmap::calculateInterpolation()");

    // remove all previous coefficents and signal buffer
    if (m_interpolation_alpha != 0) {
	delete[] m_interpolation_alpha;
	m_interpolation_alpha = 0;
    }

    ASSERT(m_zoom != 0.0);
    if (m_zoom == 0.0) return;

    // offset: index of first visible sample (left) [0...length-1]
    // m_zoom: number of samples / pixel

    // approximate the 3dB frequency of the low pass as
    // Fg = f_g / f_a
    // f_a: current "sample rate" of display (pixels) = 1.0
    // f_g: signal rate = (m_zoom/2)
    Fg = m_zoom / 2;

    // N: order of the filter, at least 2 * (1/m_zoom)
    N = (int)(INTERPOLATION_PRECISION / m_zoom);
    N |= 0x01;    // make N an odd number !

    // allocate a buffer for the coefficients
    m_interpolation_alpha = new float[N + 1];
    m_interpolation_order = N;

    ASSERT(m_interpolation_alpha);
    if (!m_interpolation_alpha) return;

    // calculate the raw coefficients and
    // apply a Hamming window
    //
    //                    sin( (2k-N) * Pi * Fg )                       2kPi
    // alpha_k = 2 * Fg * ----------------------- * [ 0,54 - 0,46 * cos ---- ]
    //                      (2k - N) * Pi * Fg                            N
    //
    f = 0.0;    // (store the sum of all coefficients in "f")
    for (k = 0; k <= N; k++) {
	m_interpolation_alpha[k] =
	    sin((2 * k - N) * PI * Fg) / ((2 * k - N) * PI * Fg);
	m_interpolation_alpha[k] *= (0.54 - 0.46 * cos(2 * k * PI / N));
	f += m_interpolation_alpha[k];
    }
    // norm the coefficients to 1.0 / m_zoom
    f *= m_zoom;
    for (k = 0; k <= N; k++)
	m_interpolation_alpha[k] /= f;

}

//***************************************************************************
void TrackPixmap::drawInterpolatedSignal(int /*channel*/, int /*middle*/, int /*height*/)
{
//    register float y;
//    register float *sig;
//    float *sig_buffer;
//    float scale_y;
//    int i;
//    register int k;
//    int N;
//    int length;
//    int sample;
//    int x;
//
////    debug("TrackPixmap::drawInterpolatedSignal");
//
//    ASSERT(signalmanage);
//    if (!signalmanage) return;
//    length = signalmanage->getLength();
//    if (!length) return;
//
//    // scale_y: pixels per unit
//    scale_y = height * zoomy / (1 << 24);
//
//    // N: order of the filter, at least 2 * (1/m_zoom)
//    N = INTERPOLATION_PRECISION * samples2pixels(1);
//    N |= 0x01;    // make N an odd number !
//
//    // re-calculate the interpolation's filter and buffers
//    // if the current order has changed
//    if (m_interpolation_order != N) {
//	calculateInterpolation();
//	N = m_interpolation_order;
//    }
//
//    ASSERT(m_interpolation_alpha);
//    if (!m_interpolation_alpha) return;
//
//    // buffer for intermediate resampled data
//    sig_buffer = new float[width + N + 2];
//    ASSERT(sig_buffer);
//    if (!sig_buffer) return;
//
//    // array with sample points
//    QPointArray *points = new QPointArray(width);
//    ASSERT(points);
//    if (!points) {
//	delete[] sig_buffer;
//	return;
//    }
//
//    // fill the sample buffer with zeroes
//    for (i = 0; i < width + N + 2; i++)
//	sig_buffer[i] = 0.0;
//
//    // resample
//    sample = -2;    // start some samples left of the window
//    x = samples2pixels(sample);
//    sig = sig_buffer + (N / 2);
//    while (x <= width + N / 2) {
//	if ((x >= -N / 2) && (m_offset + sample < length)) {
//	    sig[x] = signalmanage->singleSample(channel, m_offset + sample) *
//		     scale_y;
//	}
//	sample++;
//	x = samples2pixels(sample);
//    }
//
//    // pass the signal data through the filter
//    for (i = 0; i < width; i++) {
//	sig = sig_buffer + (i + N);
//	y = 0.0;
//	for (k = 0; k <= N; k++)
//	    y += *(sig--) * m_interpolation_alpha[k];
//
//	points->setPoint(i, i, middle - (int)y);
//    }
//
//    // display the filter's interpolated output
//    p.setPen(darkGray);
//    p.drawPolyline(*points, 0, i);
//
//    // display the original samples
//    sample = 0;
//    x = samples2pixels(sample);
//    sig = sig_buffer + (N / 2);
//    p.setPen(white);
//    i = 0;
//    while (x < width) {
//	if ((x >= 0) && (x < width)) {
//	    // mark original samples
//	    points->setPoint(i++, x, middle - (int)sig[x]);
//	}
//	sample++;
//	x = samples2pixels(sample);
//    }
//    p.drawPoints(*points, 0, i);
//
//    delete[] sig_buffer;
//    delete points;
}

//***************************************************************************
void TrackPixmap::drawPolyLineSignal(int /*channel*/, int /*middle*/, int /*height*/)
{
//    float scale_y;
//    int y;
//    int i;
//    int n;
//    int sample;
//    int x;
//
////    debug("TrackPixmap::drawPolyLineSignal");
//
//    ASSERT(signalmanage);
//    if (!signalmanage) return;
//
//    // scale_y: pixels per unit
//    scale_y = height * zoomy / (1 << 24);
//
//    // array with sample points
//    QPointArray *points = new QPointArray(width + 1);
//    ASSERT(points);
//    if (!points) return;
//
//    // display the original samples
//    sample = 0;
//    x = samples2pixels(sample);
//    i = 0;
//    while (x < width) {
//	// mark original samples
//	y = (int)(signalmanage->singleSample(channel, m_offset + sample) *
//		  scale_y);
//	points->setPoint(i++, x, middle - y);
//
//	sample++;
//	x = samples2pixels(sample);
//    }
//
//    // set "n" to the number of displayed original samples
//    n = i;
//
//    // interpolate the rest of the display if necessary
//    if (samples2pixels(sample - 1) < width - 1) {
//	int x1;
//	int x2;
//	float y1;
//	float y2;
//
//	x1 = samples2pixels(sample - 1);
//	x2 = samples2pixels(sample);
//	y1 = (int)(signalmanage->singleSample(channel, m_offset + sample - 1) *
//		   scale_y);
//	y2 = (int)(signalmanage->singleSample(channel, m_offset + sample) *
//		   scale_y);
//
//	x = width - 1;
//	y = (int)((float)(x - x1) * (float)(y2 - y1) / (float)(x2 - x1));
//
//	points->setPoint(i++, x, middle - y);
//    }
//
//    // show the poly-line
//    p.setPen(darkGray);
//    p.drawPolyline(*points, 0, i);
//
//    // show the original points
//    p.setPen(white);
//    p.drawPoints(*points, 0, n);
//
//    delete points;
}

//***************************************************************************
//***************************************************************************
