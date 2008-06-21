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

#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>

#include "OverViewWidget.h"

/* interval for limiting the number of repaints per second [ms] */
#define REPAINT_INTERVAL 500

#define TIMER_INTERVAL 100        /* [ms] */
#define MIN_SLIDER_WIDTH m_height /* pixel */

#define BAR_BACKGROUND    palette().mid().color()
#define BAR_FOREGROUND    palette().light().color()
#define SLIDER_BACKGROUND palette().background().color()
#define SLIDER_FOREGROUND palette().mid().color()

//***************************************************************************
OverViewWidget::OverViewWidget(SignalManager &signal, QWidget *parent)
    :QWidget(parent), m_width(0), m_height(0), m_grabbed(0), m_mouse_pos(0),
     m_slider_width(0), m_view_width(0), m_view_length(0),m_view_offset(0),
     m_dir(0), m_redraw(false), m_timer(), m_bitmap(),
     m_cache(signal), m_repaint_timer()
{
    // connect the timer for scrolling
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(increase()));

    // update the bitmap if the cache has changed
    connect(&m_cache, SIGNAL(changed()), this, SLOT(overviewChanged()));

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(refreshBitmap()));
}

//***************************************************************************
OverViewWidget::~OverViewWidget()
{
    m_timer.stop();
}

//***************************************************************************
void OverViewWidget::mousePressEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
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

//***************************************************************************
void OverViewWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    unsigned int old_offset = m_view_offset;

    Q_ASSERT(e);
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
	repaint();
	emit valueChanged(m_view_offset);
    }
}

//***************************************************************************
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
	repaint();
	emit valueChanged(m_view_offset);
    }
}

//***************************************************************************
void OverViewWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_grabbed = -1;
    m_timer.stop();
}

//***************************************************************************
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

    return qMin(res, m_width-1);
}

//***************************************************************************
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

    return qMin(res, m_view_length-1);
}

//***************************************************************************
void OverViewWidget::mouseMoveEvent( QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_width);
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
	    repaint();
	    emit valueChanged(m_view_offset);
	}
    }
}

//***************************************************************************
void OverViewWidget::resizeEvent(QResizeEvent *)
{
    setRange(m_view_offset, m_view_width, m_view_length);
    refreshBitmap();
}

//***************************************************************************
void OverViewWidget::setRange(unsigned int new_pos, unsigned int new_width,
	unsigned int new_length)
{
    if ( (new_pos == m_view_offset) && (new_length == m_view_length) &&
	(new_width == m_view_width) && (width() == m_width)) return;

//    qDebug("OverViewWidget::setRange(%u,%u,%u)",new_pos,new_width,new_length);

    if ( (m_view_length == new_length) && (m_view_width == new_width) &&
	(width() == m_width))
    {
	m_view_offset = new_pos;
	repaint();
    } else {
	m_width = width();
	m_view_width = qMin(new_width, new_length);
	m_view_offset = qMin(new_pos, new_length-new_width);
	m_view_length = new_length;
	m_slider_width = offset2pixels(m_view_width);
//	qDebug("OverViewWidget::setRange(): slider_width=%d",m_slider_width);
	m_slider_width = qMax(m_slider_width, MIN_SLIDER_WIDTH);
	m_slider_width = qMin(m_slider_width, m_width-1);
	m_redraw = true;
	repaint();
    }

}

//***************************************************************************
void OverViewWidget::setValue(unsigned int newval)
{
    if (m_view_offset != newval) {
	m_view_offset = newval;
	repaint();
    }
}

//***************************************************************************
void OverViewWidget::paintEvent(QPaintEvent *)
{
//    qDebug("OverViewWidget::paintEvent(QPaintEvent *)");
    QPainter p(this);

    // --- background of the widget ---
    p.fillRect(rect(), BAR_BACKGROUND);
    if (m_bitmap.width() && m_bitmap.height()) {
	QBrush brush(m_bitmap);
	brush.setColor(BAR_FOREGROUND);

	p.setBrush(brush);
	p.drawRect(0, 0, m_width, m_height);
    }

    // --- draw the slider ---
    int x = offset2pixels(m_view_offset);

    p.setBrush(SLIDER_BACKGROUND);
    p.drawRect(x, 0, m_slider_width, m_height);
    if (m_bitmap.width() && m_bitmap.height()) {
	QBrush brush(m_bitmap);
	brush.setColor(SLIDER_FOREGROUND);
	p.setBrush(brush);
	p.drawRect(x, 0, m_slider_width, m_height);
    }

    // frame around the slider
    p.setPen(palette().mid().color());
    p.drawLine(0, 0, m_width, 0);
    p.drawLine(0, 0, 0, m_height);

    p.drawLine(x, 0, x + m_slider_width, 0);
    p.drawLine(x, 0, x, m_height);
    p.drawLine(x + 1, 0, x + 1, m_height);

    // shadow of the slider
    p.setPen(palette().dark().color());
    p.drawLine(1, m_height - 1, m_width, m_height - 1);
    p.drawLine(m_width - 1, 1, m_width - 1, m_height - 1);

    p.drawLine(x + 1, m_height - 2, x + m_slider_width, m_height - 2);
    p.drawLine(x + m_slider_width, 1, x + m_slider_width, m_height);
    p.drawLine(x + m_slider_width - 1, 1, x + m_slider_width - 1, m_height);
}

//***************************************************************************
QSize OverViewWidget::minimumSize() const
{
    return QSize(30, 30);
}

//***************************************************************************
QSize OverViewWidget::sizeHint() const
{
    return minimumSize();
}

//***************************************************************************
void OverViewWidget::overviewChanged()
{
    // repainting is inhibited -> wait until the
    // repaint timer is elapsed
    if (m_repaint_timer.isActive()) {
	return;
    } else {
	// repaint once and once later...
	refreshBitmap();

	// start the repaint timer
	m_repaint_timer.setSingleShot(true);
	m_repaint_timer.start(REPAINT_INTERVAL);
    }
}

//***************************************************************************
void OverViewWidget::refreshBitmap()
{
    // let the bitmap be updated from the cache
    m_bitmap = m_cache.getOverView(m_width, m_height);

    // update the display
    repaint();
}

//***************************************************************************
#include "OverViewWidget.moc"
//***************************************************************************
//***************************************************************************
