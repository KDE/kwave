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
#include "libkwave/SampleReader.h"
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

/**
 * If the zoom factor is below this margin, an interpolation
 * will be used.
 */
#define INTERPOLATION_ZOOM 0.10

//***************************************************************************
TrackPixmap::TrackPixmap(Track &track)
    :QPixmap(), m_track(track), m_offset(0), m_zoom(0.0),
    m_minmax_mode(false),
    m_sample_buffer(), m_min_buffer(), m_max_buffer(),
    m_modified(false), m_valid(0), m_lock_buffer(),
    m_interpolation_order(0), m_interpolation_alpha(0)
{
    // set the default colors
    m_color_background   = black;
    m_color_sample       = white;
    m_color_interpolated = lightGray;
    m_color_zero         = green;

    // connect all the notification signals of the track
    connect(&track, SIGNAL(sigSamplesInserted(Track&,unsigned int,
	unsigned int)), this, SLOT(slotSamplesInserted(Track&,
	unsigned int, unsigned int)));
    connect(&track, SIGNAL(sigSamplesDeleted(Track&,unsigned int,
	unsigned int)), this, SLOT(slotSamplesDeleted(Track&,
	unsigned int, unsigned int)));
    connect(&track, SIGNAL(sigSamplesModified(Track&,unsigned int,
	unsigned int)), this, SLOT(slotSamplesModified(Track&,
	unsigned int, unsigned int)));
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
	ASSERT(buflen == m_min_buffer.size());
	ASSERT(buflen == m_max_buffer.size());
	if ((buflen != m_min_buffer.size()) || (buflen != m_max_buffer.size())) {
	    debug("TrackPixmap::setOffset(): buflen = %u", buflen);
	    debug("TrackPixmap::setOffset(): min_buffer : %u", m_min_buffer.size());
	    debug("TrackPixmap::setOffset(): max_buffer : %u", m_max_buffer.size());
	}
	
	// check for misaligned offset changes
	if ((  offset % (unsigned int)ceil(m_zoom)) !=
	    (m_offset % (unsigned int)ceil(m_zoom))) {

#ifdef CURRENTLY_UNUSED
// this will become interesting later, with the offset/zoom optimizations
	    warning("TrackPixmap::setOffset(): oh nooo, "\
	    	"offset %u is misaligned by %u sample(s), please fix this!",
	    	offset, offset % pixels2samples(1));
	    warning("TrackPixmap::setOffset(): "\
	    	"now I have to throw away the whole buffer :-((");
	
	    debug("TrackPixmap::setOffset(%u): "\
		"misaligned->invalidating buffer", offset);
#endif
	    invalidateBuffer();
	} else if (offset > m_offset) {
	    // move left
	    diff = samples2pixels(offset - m_offset);
//	    debug("TrackPixmap::setOffset(): moving left (min/max): %u",diff);
	    ASSERT(diff);
	    ASSERT(buflen);
	    if (diff && buflen) {
		for (src = diff, dst = 0; src < buflen; ++dst, ++src) {
		    m_min_buffer[dst] = m_min_buffer[src];
		    m_max_buffer[dst] = m_max_buffer[src];
		    m_valid[dst] = m_valid[src];
		}
		while (dst < buflen) m_valid.clearBit(dst++);
	    }
	} else {
	    // move right
	    diff = samples2pixels(m_offset - offset);
//	    debug("TrackPixmap::setOffset(): moving right (min/max): %u",diff);
	    ASSERT(diff);
	    ASSERT(buflen);
	    if (diff && buflen) {
		for (dst=buflen-1, src=dst-diff; dst>=diff; --dst, --src) {
		    m_min_buffer[dst] = m_min_buffer[src];
		    m_max_buffer[dst] = m_max_buffer[src];
		    m_valid[dst] = m_valid[src];
		}
		diff = dst+1;
		while (diff--) m_valid.clearBit(dst--);
	    }
	}
    } else {
	// move content of sample buffer
	// one buffer element = one sample
	ASSERT(buflen == m_sample_buffer.size());
	
	if (offset > m_offset) {
	    // move left
//	    debug("TrackPixmap::setOffset(): moving left (normal)"); // ###
	    diff = offset - m_offset;
	    for (src=diff, dst=0; src<buflen; ++dst, ++src) {
		m_sample_buffer[dst] = m_sample_buffer[src];
		m_valid[dst] = m_valid[src];
	    }
	    while (dst < buflen) m_valid[dst++] = 0;
	} else {
	    // move right
//	    debug("TrackPixmap::setOffset(): moving right (normal)"); // ###
	    diff = m_offset - offset;
	    ASSERT(buflen);
	    if (buflen) {
		for (dst=buflen-1, src=dst-diff; dst>=diff; --dst, --src) {
		    m_sample_buffer[dst] = m_sample_buffer[src];
		    m_valid[dst] = m_valid[src];
		}
		diff = dst+1;
		while (diff--) m_valid.clearBit(dst--);
	    }
	}
    }

    m_offset = offset;
    m_modified = true;
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
    while (oldlen < buflen) m_valid.clearBit(oldlen++);
}

//***************************************************************************
void TrackPixmap::setZoom(double zoom)
{
    MutexGuard lock(m_lock_buffer);

    if (zoom == m_zoom) return; // no change

//    debug("TrackPixmap::setZoom(%0.3f)", zoom);
    if ((zoom > 1.0) && !m_minmax_mode) {
	// switch to min/max mode
//	debug("TrackPixmap::setZoom(): switch to min/max mode");
	invalidateBuffer();
	m_minmax_mode = true;
    } else if ((zoom <= 1.0) && m_minmax_mode) {
	// switch to normal mode
//	debug("TrackPixmap::setZoom(): switch to normal mode");
	invalidateBuffer();
	m_minmax_mode = false;
    }

    // take the new zoom and resize the buffer
    m_zoom = zoom;
    if (m_minmax_mode) {
	// TODO: some clever caching instead of throwing away everything
	resizeBuffer();
	invalidateBuffer();
    } else {
	resizeBuffer();
    }

    m_modified = true;
}

//***************************************************************************
void TrackPixmap::resize(int width, int height)
{
    MutexGuard lock(m_lock_buffer);

    int old_width = QPixmap::width();
    int old_height = QPixmap::height();
    if ((old_width == width) && (old_height == height)) return; // no change

    QPixmap::resize(width, height);
    if (width != old_width) resizeBuffer();

    m_modified = true;
}

//***************************************************************************
void TrackPixmap::invalidateBuffer()
{
    for (unsigned int i=0; i < m_valid.size(); ++i) {
	m_valid.clearBit(i);
    }
    m_modified = true;
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

    // ### FIXME:
    // work-around for missing extra buffer, delete the whole buffer
    // instead. this should not do any harm, in this mode we only
    // have few samples and redrawing will be fast
    if (m_zoom < INTERPOLATION_ZOOM) invalidateBuffer();

    while (first < buflen) {
	// find the first invalid index
	for (first=last; (first < buflen) && m_valid.testBit(first);
		++first);
	
	// break if the first index is out of range
	if (first >= buflen) return false; // buffer is already ok
	
	// find the last invalid index
	for (last=first; (last < buflen) && !m_valid[last]; ++last);
	
	if (last >= buflen) last = buflen-1;
	if ((last > first) && (m_valid[last])) --last;
	
	// fill our array(s) with fresh sample data
	if (m_minmax_mode) {
	    // indices are in pixels, convert to samples
	
	    // first sample in first pixel
	    unsigned int s1 = m_offset +
		(unsigned int)floor(first * m_zoom);
	    // last sample of last pixel
	    unsigned int s2 = m_offset +
		(unsigned int)floor((last+1) * m_zoom)-1;

	    // open a reader for the whole modified range	
	    SampleReader *in = m_track.openSampleReader(s1, s2);
	    ASSERT(in);
	    if (!in) break;
	
	    // allocate a buffer for one more sample (pixels2samples may
	    // vary by +/-1 !
	    QArray<sample_t> buffer(ceil(m_zoom)/*pixels2samples(1) + 1*/);
	    sample_t min;
	    sample_t max;
	
	    while (first <= last) {
		// NOTE: s2 is exclusive!
		s2 = m_offset + (unsigned int)floor((first+1)*m_zoom);
		
		// get min/max for interval [s1...s2[
		unsigned int count = in->read(buffer, 0, s2-s1);
		if (count) {
		    unsigned int pos;
		    min = SAMPLE_MAX;
		    max = SAMPLE_MIN;
		    for (pos=0; pos < count; pos++) {
			register sample_t val = buffer[pos];
			if (val < min) min = val;
			if (val > max) max = val;
		    }
		} else {
		    // no data available, set zeroes
		    min = max = 0;
		}
		
		m_min_buffer[first] = min;
		m_max_buffer[first] = max;
		m_valid.setBit(first);
		
		// advance to the next position
		++first;
		s1 = s2;
	    }

	    delete in;
	} else {
	    // each index is one sample
	    SampleReader *in = m_track.openSampleReader(
		m_offset+first, m_offset+last);
	    ASSERT(in);
	    if (!in) break;
	
	    // read directly into the buffer
	    unsigned int count = in->read(m_sample_buffer, first,
		last-first+1);
	    while (count--) m_valid.setBit(first++);
	
	    // fill the rest with zeroes
	    while (first <= last) {
		m_valid.setBit(first);
		m_sample_buffer[first++] = 0;
	    }
	
	    delete in;
	}
	
	ASSERT(first >= last);
        ++last;
    }

    for (first=0; first < m_valid.size(); first++) {
	if (!m_valid[first]) warning("TrackPixmap::validateBuffer(): "\
		"still invalid index: %u", first);
    }

    return true;
}

//***************************************************************************
void TrackPixmap::repaint()
{
    MutexGuard lock(m_lock_buffer);

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
	    drawOverview(p, h>>1, h, 0, w-1);
	} else {
	    if (m_zoom < INTERPOLATION_ZOOM) {
		drawInterpolatedSignal(p, w, h>>1, h);
	    } else {
		drawPolyLineSignal(p, w, h>>1, h);
	    }
	}
	
	// draw the zero-line
	p.setPen(m_color_zero);
	p.drawLine(0, h>>1, w-1, h>>1);
    }

    // now we are no longer "modified"
    m_modified = false;
}

//***************************************************************************
bool TrackPixmap::isModified()
{
    MutexGuard lock(m_lock_buffer);
    return m_modified;
}

//***************************************************************************
void TrackPixmap::drawOverview(QPainter &p, int middle, int height,
	int first, int last)
{
    ASSERT(m_minmax_mode);
    ASSERT(width() <= (int)m_min_buffer.size());
    ASSERT(width() <= (int)m_max_buffer.size());

    // scale_y: pixels per unit
    double scale_y = (double)height / (1 << 24);
    int max = 0, min = 0;

    p.setPen(m_color_sample);
    for (int i = first; i <= last; i++) {
	ASSERT(m_valid[i]);
	max = (int)(m_max_buffer[i] * scale_y);
	min = (int)(m_min_buffer[i] * scale_y);
	p.drawLine(i, middle - max, i, middle - min);
    }
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
void TrackPixmap::drawInterpolatedSignal(QPainter &p, int width,
	int middle, int height)
{
    register float y;
    register float *sig;
    float *sig_buffer;
    float scale_y;
    int i;
    register int k;
    int N;
    int sample;
    int x;
    int buflen = m_valid.size();

//    debug("TrackPixmap::drawInterpolatedSignal()");

    ASSERT(m_zoom);
    if (m_zoom == 0.0) return;

    // scale_y: pixels per unit
    scale_y = (float)height / (float)((SAMPLE_MAX+1)<<1);

    // N: order of the filter, at least 2 * (1/m_zoom)
    N = INTERPOLATION_PRECISION * samples2pixels(1);
    N |= 0x01;    // make N an odd number !

    // re-calculate the interpolation's filter and buffers
    // if the current order has changed
    if (m_interpolation_order != N) {
	calculateInterpolation();
	N = m_interpolation_order;
    }

    ASSERT(m_interpolation_alpha);
    if (!m_interpolation_alpha) return;

    // buffer for intermediate resampled data
    sig_buffer = new float[width + N + 2];
    ASSERT(sig_buffer);
    if (!sig_buffer) return;

    // array with sample points
    QPointArray *points = new QPointArray(width);
    ASSERT(points);
    if (!points) {
	delete[] sig_buffer;
	return;
    }

    // fill the sample buffer with zeroes
    for (i = 0; i < width + N + 2; i++)
	sig_buffer[i] = 0.0;

    // resample
    sample = -2;    // start some samples left of the window
    x = samples2pixels(sample);
    sig = sig_buffer + (N / 2);
    while (x <= width + N / 2) {
	if ((x >= -N / 2) && (sample > 0) && (sample < buflen)) {
	    sig[x] = m_sample_buffer[sample] * scale_y;
	}
	sample++;
	x = samples2pixels(sample);
    }

    // pass the signal data through the filter
    for (i = 0; i < width; i++) {
	sig = sig_buffer + (i + N);
	y = 0.0;
	for (k = 0; k <= N; k++)
	    y += *(sig--) * m_interpolation_alpha[k];
	
	points->setPoint(i, i, middle - (int)y);
    }

    // display the filter's interpolated output
    p.setPen(m_color_interpolated);
    p.drawPolyline(*points, 0, i);

    // display the original samples
    sample = 0;
    x = samples2pixels(sample);
    sig = sig_buffer + (N / 2);
    p.setPen(m_color_sample);
    i = 0;
    while (x < width) {
	if ((x >= 0) && (x < width)) {
	    // mark original samples
	    points->setPoint(i++, x, middle - (int)sig[x]);
	}
	sample++;
	x = samples2pixels(sample);
    }
    p.drawPoints(*points, 0, i);

    delete[] sig_buffer;
    delete points;
}

//***************************************************************************
void TrackPixmap::drawPolyLineSignal(QPainter &p, int width,
	int middle, int height)
{
    double scale_y;
    int y;
    int i;
    int n;
    unsigned int sample;
    int x;
    unsigned int buflen = m_sample_buffer.size();

    // scale_y: pixels per unit
    scale_y = (double)height / (double)((SAMPLE_MAX+1)<<1);

    // array with sample points
    QPointArray *points = new QPointArray(width + 1);
    ASSERT(points);
    if (!points) return;

    // display the original samples
    sample = 0;
    x = samples2pixels(sample);
    i = 0;
    while (x < width) {
	// mark original samples
	sample_t value = (sample < buflen) ? m_sample_buffer[sample] : 0;
	y = (int)(value * scale_y);
	points->setPoint(i++, x, middle - y);

	sample++;
	x = samples2pixels(sample);
    }

    // set "n" to the number of displayed original samples
    n = i;

    // interpolate the rest of the display if necessary
    if (samples2pixels(sample - 1) <= width) {
	int x1;
	int x2;
	float y1;
	float y2;
	
	x1 = samples2pixels(sample - 1);
	x2 = samples2pixels(sample);
	
	y1 = ((sample) && (sample <= buflen)) ?
	    (int)(scale_y * m_sample_buffer[sample-1]) : 0.0;
	y2 = (sample < buflen) ?
	    (int)(scale_y * m_sample_buffer[sample]) : 0.0;
	
	x = width - 1;
	y = (int)((float)(x - x1) * (float)(y2 - y1) / (float)(x2 - x1));
	
	points->setPoint(i++, x, middle - y);
    }

    // show the poly-line
    p.setPen(darkGray);
    p.drawPolyline(*points, 0, i);

    // show the original points
    p.setPen(white);
    p.drawPoints(*points, 0, n);

    delete points;
}

//***************************************************************************
void TrackPixmap::slotSamplesInserted(Track &, unsigned int offset,
                                      unsigned int length)
{
    {
	MutexGuard lock(m_lock_buffer);
	
	convertOverlap(offset, length);
	if (!length) return; // false alarm
	
	ASSERT(offset < m_valid.size());
	ASSERT(offset + length <= m_valid.size());
	
	// mark all positions from here to right end as "invalid"
	while (offset < m_valid.size()) m_valid.clearBit(offset++);
	
	// repaint of the signal is needed
	m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void TrackPixmap::slotSamplesDeleted(Track &, unsigned int offset,
                                     unsigned int length)
{
    {
	MutexGuard lock(m_lock_buffer);
	
	convertOverlap(offset, length);
	if (!length) return; // false alarm
	
	ASSERT(offset < m_valid.size());
	ASSERT(offset + length <= m_valid.size());
	
	// mark all positions from here to right end as "invalid"
	while (offset < m_valid.size()) m_valid.clearBit(offset++);
	
	// repaint of the signal is needed
	m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void TrackPixmap::slotSamplesModified(Track &, unsigned int offset,
                                      unsigned int length)
{
    {
	MutexGuard lock(m_lock_buffer);
	
	convertOverlap(offset, length);
	if (!length) return; // false alarm
	
	ASSERT(offset < m_valid.size());
	ASSERT(offset + length <= m_valid.size());
	
	// mark all overlapping positions as "invalid"
	while (length--) m_valid.clearBit(offset++);
	
	// repaint of the signal is needed
	m_modified = true;
    }

    // notify our owner about changed data -> screen refresh?
    emit sigModified();
}

//***************************************************************************
void TrackPixmap::convertOverlap(unsigned int &offset, unsigned int &length)
{
    ASSERT(m_zoom);
    if (m_zoom == 0.0) length = 0;

    if (!length) return;
    if ((offset + length) <= m_offset) {
	length = 0;
	return; // not yet in view
    }

    unsigned int buflen = m_valid.size();

    // calculate the length
    if (m_minmax_mode) {
	// attention: round up the length int this mode!
	if (offset >= m_offset + (unsigned int)ceil(buflen * m_zoom)) {
	    length = 0; // out of view
	    return;
	} else {
	    length = (unsigned int)ceil(length / m_zoom);
	}
    } else {
	if (offset >= m_offset+buflen) {
	    length = 0; // out of view
	    return;
	}
    }

    // convert the offset
    offset = (offset > m_offset) ? offset - m_offset : 0;
    if (m_minmax_mode) {
	// attention: round down in this mode!
	unsigned int ofs = (unsigned int)floor(offset / m_zoom);
	
	// if offset was rounded down, increment length
	if (ofs != (unsigned int)ceil(offset / m_zoom)) length++;
	offset = ofs;
    }

    // limit the offset (maybe something happened when rounding)
    if (offset >= buflen) offset = buflen-1;

    // limit the length to the end of the buffer
    if (offset+length > buflen) length = buflen-offset;

    ASSERT(length);
}

//***************************************************************************
//***************************************************************************
