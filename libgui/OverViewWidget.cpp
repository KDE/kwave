/***************************************************************************
     OverViewWidget.cpp  -  horizontal slider with overview over a signal
                             -------------------
    begin                : Tue Oct 21 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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
#include "math.h"

#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"

#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "kwave/SignalManager.h"

#include "OverViewWidget.h"

#define TIMER_INTERVAL 100        /* ms */
#define MIN_SLIDER_WIDTH m_height /* pixel */
#define CACHE_SIZE 8192           /* number of cache entries */

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

#define BAR_BACKGROUND    colorGroup().mid()
#define BAR_FOREGROUND    colorGroup().light()
#define SLIDER_BACKGROUND colorGroup().background()
#define SLIDER_FOREGROUND colorGroup().mid()

//*****************************************************************************
OverViewWidget::OverViewWidget(SignalManager &signal, QWidget *parent)
    :QWidget(parent), m_signal(signal), m_min(), m_max(), m_state(),
     m_count(0), m_scale(1), m_lock()
{
    m_bitmap = 0;
    m_dir = 0;
    m_grabbed = 0;
    m_height = 0;
    m_mouse_pos = 0;
    m_pixmap = 0;
    m_redraw = false;
    m_slider_width = 0;
    m_view_length = 0;
    m_view_offset = 0;
    m_view_width = 0;
    m_width = 0;

    // connect to the signal manager
    SignalManager *sig = &signal;
    ASSERT(sig);
    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track &)),
            this, SLOT(slotTrackInserted(unsigned int, Track &)));
    connect(sig, SIGNAL(sigTrackDeleted(unsigned int)),
            this, SLOT(slotTrackDeleted(unsigned int)));
    connect(sig, SIGNAL(sigSamplesDeleted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesDeleted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesInserted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesInserted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesModified(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesModified(unsigned int, unsigned int,
	unsigned int)));

    setBackgroundMode(NoBackground); // this avoids flicker :-)
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(increase()));
}

//*****************************************************************************
OverViewWidget::~OverViewWidget()
{
    m_timer.stop();
    if (m_pixmap) delete m_pixmap;
    if (m_bitmap) delete m_bitmap;
}

//*****************************************************************************
void OverViewWidget::mousePressEvent(QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    int x = offset2pixels(m_view_offset);
    m_mouse_pos = e->x();

    if (m_mouse_pos > x + m_slider_width) {
	// clicked at the right
	m_grabbed = -1;
	m_dir = (m_view_width / 2);
	m_timer.stop();
	m_timer.start(TIMER_INTERVAL);
    } else if (m_mouse_pos < x) {
	// clicked at the left
	m_grabbed = -1;
	m_dir = -1 * (m_view_width / 2);
	m_timer.stop();
	m_timer.start(TIMER_INTERVAL);
    } else {
	// clicked into the slider
	m_grabbed = m_mouse_pos - x;
    }
}

//****************************************************************************
void OverViewWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    unsigned int old_offset = m_view_offset;

    ASSERT(e);
    if (!e) return;

    m_mouse_pos = e->x();

    // change to "grabbed mode" if the visible are wandered under
    // the last mouse/cursor position
    m_timer.stop();
	
    // try to put the center of the current view under the cursor
    unsigned int mpos = pixels2offset(m_mouse_pos);
    m_view_offset = (mpos > (m_view_width >> 1)) ?
	(mpos - (m_view_width >> 1)) : 0;
    if (m_view_offset > m_view_length - m_view_width)
	m_view_offset = m_view_length - m_view_width;
	
    m_grabbed = m_mouse_pos - offset2pixels(m_view_offset);

    if (m_view_offset != old_offset) {
	repaint(false);
	emit valueChanged(m_view_offset);
    }
}

//****************************************************************************
void OverViewWidget::increase()
{
    unsigned int old_offset = m_view_offset;

    // advance the current offset
    if ((m_dir < 0) && (m_view_offset <= static_cast<unsigned int>(-m_dir)))
	m_view_offset = 0;
    else
	m_view_offset += m_dir;

    // do some range checking
    if (m_view_offset > m_view_length - m_view_width)
	m_view_offset = m_view_length - m_view_width;

    if ((m_mouse_pos >= offset2pixels(m_view_offset)) &&
        (m_mouse_pos <  offset2pixels(m_view_offset) +
         offset2pixels(m_view_width)))
    {
	// change to "grabbed mode" if the visible are wandered under
	// the last mouse/cursor position
	m_timer.stop();
	
	// try to put the center of the current view under the cursor
	unsigned int mpos = pixels2offset(m_mouse_pos);
	m_view_offset = (mpos > (m_view_width >> 1)) ?
	    (mpos - (m_view_width >> 1)) : 0;
	if (m_view_offset > m_view_length - m_view_width)
	    m_view_offset = m_view_length - m_view_width;
	
	m_grabbed = m_mouse_pos - offset2pixels(m_view_offset);
    }

    if (m_view_offset != old_offset) {
	repaint(false);
	emit valueChanged(m_view_offset);
    }
}

//****************************************************************************
void OverViewWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_grabbed = -1;
    m_timer.stop();
}

//****************************************************************************
int OverViewWidget::offset2pixels(unsigned int offset)
{
    int res;
    // how big would the slider be with normal translation
    int slider_width = (m_view_length) ? (int)((float)(m_view_width) *
	(float)(m_width) / (float)(m_view_length)) : 0;

    if ((slider_width) >= MIN_SLIDER_WIDTH) {
	// slider is big enough -> normal translation
	
	//    0...view_length-1]        [width - 1]        [view_length-1]
	res=(int)((float)offset*(float)(m_width-1)/(float)(m_view_length-1));
    } else {
	// slider is at minimum size -> special translation
	int max_pixel  = (m_width - MIN_SLIDER_WIDTH);
	int max_offset = (m_view_length - m_view_width);
	if (max_offset <= 1) return (m_width-1); // zoomed to all

	//   [0...view_length-1]
 	res=(int)((float)offset*(float)(max_pixel-1)/(float)(max_offset-1));
    }

    return min(res, m_width-1);
}

//****************************************************************************
unsigned int OverViewWidget::pixels2offset(int pixels)
{
    unsigned int res;
    if (m_width <= 1) return 0;

    int slider_width = (m_view_length) ? (int)((float)m_view_width *
	(float)m_width / (float)m_view_length) : 0;
	
    if ((slider_width) >= MIN_SLIDER_WIDTH) {
	res = (unsigned int)((float)pixels*(float)(m_view_length-1) /
	                     (float)(m_width-1));
    } else {
	if (m_width <= MIN_SLIDER_WIDTH) return 0;
	unsigned int max_pixel  = (m_width - MIN_SLIDER_WIDTH);
	unsigned int max_offset = (m_view_length - m_view_width);
	res = (unsigned int)((float)pixels * (float)(max_offset-1) /
	                     (float)(max_pixel-1));
    }

    return min(res, m_view_length-1);
}

//****************************************************************************
void OverViewWidget::mouseMoveEvent( QMouseEvent *e)
{
    ASSERT(e);
    ASSERT(m_width);
    if (!e) return;
    if (!m_width) return;

    if (m_grabbed > 0) {
	unsigned int old_offset = m_view_offset;
	int pos = e->x() - m_grabbed;
	if (pos < 0) pos = 0;
	if (pos > m_width) pos = m_width;

	m_view_offset = pixels2offset(pos);

	// do some range checking
	if (m_view_offset > m_view_length - m_view_width)
	    m_view_offset = m_view_length - m_view_width;

	if (m_view_offset != old_offset) {
	    repaint(false);
	    emit valueChanged(m_view_offset);
	}
    }
}

//*****************************************************************************
void OverViewWidget::resizeEvent(QResizeEvent *)
{
    setRange(m_view_offset, m_view_width, m_view_length);
    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::setRange(unsigned int new_pos, unsigned int new_width,
	unsigned int new_length)
{
    if ( (new_pos == m_view_offset) && (new_length == m_view_length) &&
	(new_width == m_view_width) && (width() == m_width)) return;

//    debug("OverViewWidget::setRange(%u,%u,%u)",new_pos,new_width,new_length);

    if ( (m_view_length == new_length) && (m_view_width == new_width) &&
	(width() == m_width))
    {
	m_view_offset = new_pos;
	repaint(false);
    } else {
	m_width = width();
	m_view_width = min(new_width, new_length);
	m_view_offset = min(new_pos, new_length-new_width);
	m_view_length = new_length;
	m_slider_width = offset2pixels(m_view_width);
//	debug("OverViewWidget::setRange(): slider_width=%d",m_slider_width);
	m_slider_width = max(m_slider_width, MIN_SLIDER_WIDTH);
	m_slider_width = min(m_slider_width, m_width-1);
	m_redraw=true;
	repaint(false);
    }

}

//*****************************************************************************
void OverViewWidget::setValue(unsigned int newval)
{
    if (m_view_offset != newval) {
	m_view_offset = newval;
	repaint (false);
    }
}

//*****************************************************************************
void OverViewWidget::paintEvent(QPaintEvent *)
{
//    debug("OverViewWidget::paintEvent(QPaintEvent *)");
    QPainter p;

    // if pixmap has to be resized ...
    if ((height() != m_height) || (width() != m_width) || m_redraw) {
	m_redraw = false;
	m_height = rect().height();
	m_width = rect().width();
	if (m_pixmap) delete m_pixmap;
	m_pixmap = new QPixmap(size());
    }
    if (!m_bitmap) refreshBitmap();
    ASSERT(m_pixmap);
    if (!m_pixmap) return;

    // --- background of the widget ---
    p.begin(m_pixmap);
    m_pixmap->fill(BAR_BACKGROUND);
    if (m_bitmap) {
	QBrush brush;
	brush.setPixmap(*m_bitmap);
	brush.setColor(BAR_FOREGROUND);
	
	p.setBrush(brush);
	p.drawRect(0, 0, m_width, m_height);
    }

    // --- draw the slider ---
    int x = offset2pixels(m_view_offset);

    p.setBrush(SLIDER_BACKGROUND);
    p.drawRect(x, 0, m_slider_width, m_height);
    if (m_bitmap) {
	QBrush brush;
	brush.setPixmap(*m_bitmap);
	brush.setColor(SLIDER_FOREGROUND);
	p.setBrush(brush);
	p.drawRect(x, 0, m_slider_width, m_height);
    }

    // frame around the slider
    p.setPen(colorGroup().mid());
    p.drawLine(0, 0, m_width, 0);
    p.drawLine(0, 0, 0, m_height);

    p.drawLine(x, 0, x + m_slider_width, 0);
    p.drawLine(x, 0, x, m_height);
    p.drawLine(x + 1, 0, x + 1, m_height);

    // shadow of the slider
    p.setPen(colorGroup().dark());
    p.drawLine(1, m_height - 1, m_width, m_height - 1);
    p.drawLine(m_width - 1, 1, m_width - 1, m_height - 1);

    p.drawLine(x + 1, m_height - 2, x + m_slider_width, m_height - 2);
    p.drawLine(x + m_slider_width, 1, x + m_slider_width, m_height);
    p.drawLine(x + m_slider_width - 1, 1, x + m_slider_width - 1, m_height);

    p.end();

    bitBlt(this, 0, 0, m_pixmap);
}

//*****************************************************************************
const QSize OverViewWidget::minimumSize()
{
    return QSize(30, 30);
}

//*****************************************************************************
const QSize OverViewWidget::sizeHint()
{
    return minimumSize();
}

//*****************************************************************************
void OverViewWidget::scaleUp()
{
    const unsigned int len = m_signal.length();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil((double)len/(double)CACHE_SIZE)));
    if (!new_scale) new_scale = 1;

    const unsigned int shrink = static_cast<unsigned int>(
	rint(ceil(new_scale / m_scale)));

    for (unsigned int t=0; t < m_state.count(); ++t) {
	unsigned int dst = 0;
	unsigned int count = len / m_scale;
	if (count > CACHE_SIZE) count = 0;
	
	char *smin = m_min.at(t)->data();
	char *smax = m_max.at(t)->data();
	CacheState *sstate = m_state.at(t)->data();
	
	char *dmin = smin;
	char *dmax = smax;
	CacheState *dstate = sstate;
	
	while (dst < count) {
	    char min = +127;
	    char max = -127;
	    CacheState state = Unused;
	    for (unsigned int i = 0; i < shrink; ++i) {
		if (*smin < min) min = *smin;
		if (*smax > max) max = *smax;
		if (*sstate < state) state = *sstate;
		++smin;
		++smax;
		++sstate;
	    }
	    *dmin = min;
	    *dmax = max;
	    *dstate = state;
	    ++dmin;
	    ++dmax;
	    ++dstate;
	    ++dst;
	}
	while (dst++ < CACHE_SIZE) {
	    *dstate = Unused;
	    dstate++;
	}
    }

    m_scale = new_scale;
}

//*****************************************************************************
void OverViewWidget::scaleDown()
{
    const unsigned int len = m_signal.length();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil(len/CACHE_SIZE)));
    if (!new_scale) new_scale = 1;
    if (m_scale == new_scale) return;
//    debug("OverViewWidget::scaleDown(), new scale = %u", new_scale);

    m_scale = new_scale;
    for (unsigned int track=0; track < m_state.count(); ++track) {
	invalidateCache(track, 0, len / m_scale);
    }
}

//*****************************************************************************
void OverViewWidget::slotTrackInserted(unsigned int index, Track &)
{
    MutexGuard lock(m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((m_signal.length() / m_scale) > CACHE_SIZE)
	scaleUp();
    if ((m_signal.length() / m_scale) < (CACHE_SIZE/4))
	scaleDown();

    QArray<CacheState> *state = new QArray<CacheState>(CACHE_SIZE);
    QByteArray *min = new QByteArray(CACHE_SIZE);
    QByteArray *max = new QByteArray(CACHE_SIZE);

    if (!state || !min || !max) {
	ASSERT(state);
	ASSERT(min);
	ASSERT(max);
	if (state) delete state;
	if (min) delete min;
	if (max) delete max;
	return;
    }

    min->fill(+127);
    max->fill(-127);
    state->fill(Unused);

    m_min.insert(index, min);
    m_max.insert(index, max);
    m_state.insert(index, state);

    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::invalidateCache(unsigned int track, unsigned int first,
                                     unsigned int last)
{
    QArray<CacheState> *state = m_state.at(track);
    ASSERT(state);
    if (!state) return;

    ASSERT(last < CACHE_SIZE);
    if (last >= CACHE_SIZE) last = CACHE_SIZE-1;

    unsigned int pos;
    for (pos = first; pos <= last; ++pos) {
	(*state)[pos] = Invalid;
    }
}

//*****************************************************************************
void OverViewWidget::slotTrackDeleted(unsigned int index)
{
    MutexGuard lock(m_lock);

    m_min.remove(index);
    m_max.remove(index);
    m_state.remove(index);

    if (m_state.isEmpty()) m_scale = 1;
    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::slotSamplesInserted(unsigned int track,
    unsigned int offset, unsigned int /*length*/)
{
//    debug("OverViewWidget::slotSamplesInserted(%u,%u,%u)",track,offset,length);
    MutexGuard lock(m_lock);

    if ((m_signal.length() / m_scale) > CACHE_SIZE)
        scaleUp();

    // invalidate all samples from offset to end of file
    unsigned int first = offset / m_scale;
    unsigned int last  = m_signal.length() / m_scale;
    invalidateCache(track, first, last);
    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::slotSamplesDeleted(unsigned int track,
    unsigned int offset, unsigned int /* length */)
{
//    debug("OverViewWidget::slotSamplesDeleted(%u,%u,%u)",track,offset,length);
    MutexGuard lock(m_lock);

    if ((m_signal.length() / m_scale) < (CACHE_SIZE/4))
        scaleDown();

    // invalidate all samples from offset to end of file
    unsigned int first = offset / m_scale;
    unsigned int last  = m_signal.length() / m_scale;
    invalidateCache(track, first, last);
    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::slotSamplesModified(unsigned int track,
    unsigned int offset, unsigned int length)
{
//    debug("OverViewWidget::slotSamplesModified(%u,%u,%u)",track,offset,length);
    MutexGuard lock(m_lock);

    unsigned int first = offset / m_scale;
    unsigned int last  = ((offset+length-1) / m_scale) + 1;
    invalidateCache(track, first, last);
    refreshBitmap();
}

//*****************************************************************************
void OverViewWidget::refreshBitmap()
{
//    debug("OverViewWidget::refreshBitmap()");

    const unsigned int length = m_signal.length();
    MultiTrackReader src;
    m_signal.openMultiTrackReader(src, m_signal.allTracks(),
                                  0, length-1);

    // loop over all min/max buffers and make their content valid
    for (unsigned int t=0; t < m_state.count(); ++t) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 0;
	
	char *min = m_min.at(t)->data();
	char *max = m_max.at(t)->data();
	CacheState *state = m_state.at(t)->data();
	SampleReader *reader = src[t];
	
	for (unsigned int ofs=0; ofs < count; ++ofs) {
	    if (state[ofs] == Valid)  continue;
	    if (state[ofs] == Unused) continue;
	
	    sample_t min_sample = SAMPLE_MAX;
	    sample_t max_sample = SAMPLE_MIN;
	    unsigned int first = ofs*m_scale;
	    unsigned int count = m_scale;
	
	    reader->seek(first);
	    while (count--) {
		sample_t sample;
		(*reader) >> sample;
		if (sample > max_sample) max_sample = sample;
		if (sample < min_sample) min_sample = sample;
	    }
	
	    min[ofs] = min_sample >> (SAMPLE_BITS - 8);
	    max[ofs] = max_sample >> (SAMPLE_BITS - 8);
	    state[ofs] = Valid;
//	    debug("ofs=%d, min=%d, max=%d", ofs, min[ofs], max[ofs]);
	}
    }

    // if bitmap has to be resized or re-created...
    if (!m_bitmap || (m_bitmap->height() != m_height) ||
        (m_bitmap->width() != m_width))
    {
	if (m_bitmap) delete m_bitmap;
	if (m_width && m_height) m_bitmap = new QBitmap(m_width, m_height);
    }
    if (!m_width || !m_height) return; // empty ?

    ASSERT(m_bitmap);
    if (!m_bitmap) return;

    m_bitmap->fill(color0);
    QPainter p;
    p.begin(m_bitmap);

    // loop over all min/max buffers
    for (int x=0; (x < m_width) && (m_state.count()); ++x) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 1;
	
	// get the corresponding cache index
	unsigned int index = ((count-1) * x) / (m_width-1);
	unsigned int last_index  = ((count-1) * (x+1)) / (m_width-1);
	if (last_index > index) last_index--;
	
	// loop over all cache indices
	int minimum = +127;
	int maximum = -127;
	for (; index <= last_index; ++index) {
	    // loop over all tracks
	    for (unsigned int t=0; t < m_state.count(); ++t) {
		char *min = m_min.at(t)->data();
		char *max = m_max.at(t)->data();
		CacheState *state = m_state.at(t)->data();
		if (state[index] != Valid) continue;
		
		if (min[index] < minimum) minimum = min[index];
		if (max[index] > maximum) maximum = max[index];
	    }
	}
//	debug("x=%d, min=%d, max=%d", x, minimum, maximum);
	
	// update the bitmap
	p.setPen(color0);
	p.drawLine(x, 0, x, m_height-1);
	p.setPen(color1);
	
	const int middle = (m_height>>1);
	p.drawLine(x, middle + (minimum * m_height)/254,
	           x, middle + (maximum * m_height)/254);
    }

    p.end();

    // update the display
    repaint(false);
}

//*****************************************************************************
//*****************************************************************************
