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

#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qslider.h>  // for it's sizeHint()

#include "OverViewWidget.h"

#define TIMER_INTERVAL 100        /* ms */
#define MIN_SLIDER_WIDTH m_height /* pixel */

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
OverViewWidget::OverViewWidget(QWidget *parent, const char *name)
    :QWidget(parent, name)
{
    m_dir = 0;
    m_grabbed = 0;
    m_height = 0;
    m_overview = 0;
    m_pixmap = 0;
    m_redraw = false;
    m_slider_width = 0;
    m_view_length = 0;
    m_view_offset = 0;
    m_view_width = 0;
    m_width = 0;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(increase()));
}

//*****************************************************************************
OverViewWidget::~OverViewWidget()
{
    m_timer.stop();
    if (m_pixmap) delete m_pixmap;
    if (m_overview) delete m_overview;
}

//*****************************************************************************
void OverViewWidget::mousePressEvent(QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    int x = offset2pixels(m_view_offset);

    if (e->x() > x + m_slider_width) {
	// clicked at the right
	m_dir = m_view_width / 2;
	m_timer.stop();
	m_timer.start(TIMER_INTERVAL);
    } else if (e->x() < x) {
	// clicked at the left
	m_dir = -m_view_width / 2;
	m_timer.stop();
	m_timer.start(TIMER_INTERVAL);
    } else {
	// clicked into the slider
	m_grabbed = e->x() - x;
    }
}

//****************************************************************************
void OverViewWidget::increase()
{
    m_view_offset += m_dir;
    if (m_view_offset < 0) m_view_offset = 0;
    if (m_view_offset > m_view_length - m_view_width)
	m_view_offset = m_view_length - m_view_width;
    repaint (false);
    emit valueChanged(m_view_offset);
}

//****************************************************************************
void OverViewWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_grabbed = 0;
    m_timer.stop();
}

//****************************************************************************
int OverViewWidget::offset2pixels(int offset)
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
int OverViewWidget::pixels2offset(int pixels)
{
    int res;
    if (m_width <= 1) return 0;

    int slider_width = (m_view_length) ? (int)((float)m_view_width *
	(float)m_width / (float)m_view_length) : 0;
	
    if ((slider_width) >= MIN_SLIDER_WIDTH) {
	res = (int)((float)pixels*(float)(m_view_length-1)/(float)(m_width-1));
    } else {
	int max_pixel  = (m_width - MIN_SLIDER_WIDTH);
	int max_offset = (m_view_length - m_view_width);
	if (max_pixel <= 1) return 0;
	res = (int)((float)pixels * (float)(max_offset-1) / (float)(max_pixel-1));
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

    if (m_grabbed) {
	int pos = e->x() - m_grabbed;
	if (pos < 0) pos = 0;
	if (pos > m_width) pos = m_width;

	m_view_offset = pixels2offset(pos);

	repaint(false);
	emit valueChanged(m_view_offset);
    }
}

//*****************************************************************************
void OverViewWidget::setOverView(QBitmap *overview)
{
    if (m_overview) delete m_overview;
    m_overview = 0;

    if (overview) m_overview = new QBitmap(*overview);

    m_redraw = true;
    repaint(false);
}

//*****************************************************************************
void OverViewWidget::resizeEvent(QResizeEvent *e)
{
    setRange(m_view_offset, m_view_width, m_view_length);
}

//*****************************************************************************
void OverViewWidget::setRange(int new_pos, int new_width, int new_length)
{
    if ( (new_pos == m_view_offset) && (new_length == m_view_length) &&
	(new_width == m_view_width) ) return;
	
    if ( (m_view_length == new_length) && (m_view_width == new_width) ) {
	m_view_offset = new_pos;
	repaint(false);
    } else {
//	debug("OverViewWidget::setRange(%d,%d,%d)",new_pos,new_width,new_length); // ###
	m_width = width();
	m_view_width = min(new_width, new_length);
	m_view_offset = min(new_pos, new_length-new_width);
	m_view_length = new_length;
	m_slider_width = offset2pixels(m_view_width);
//	debug("OverViewWidget::setRange(): slider_width=%d",m_slider_width); // ###
	m_slider_width = max(m_slider_width, MIN_SLIDER_WIDTH);
	m_slider_width = min(m_slider_width, m_width-1);
	m_redraw=true;
	repaint(false);
    }

}

//*****************************************************************************
void OverViewWidget::setValue(int newval)
{
    if (m_view_offset != newval) {
	m_view_offset = newval;
	repaint (false);
    }
}

//*****************************************************************************
void OverViewWidget::paintEvent (QPaintEvent *)
{
    QPainter p;

    // if pixmap has to be resized ...
    if ((height() != m_height) || (width() != m_width) || m_redraw) {
	m_redraw = false;
	m_height = rect().height();
	m_width = rect().width();

	if (m_pixmap) delete m_pixmap;
	m_pixmap = new QPixmap (size());
        ASSERT(m_pixmap);
        if (!m_pixmap) return;

	m_pixmap->fill(BAR_BACKGROUND);
	if (m_overview) {
	    p.begin(m_pixmap);

	    QBrush brush;
	    brush.setPixmap(*m_overview);
	    brush.setColor(BAR_FOREGROUND);
	
	    p.setBrush(brush);
	    p.drawRect(0, 0, m_width, m_height);
	
	    p.end();
	}
    }
    if (m_pixmap) bitBlt(this, 0, 0, m_pixmap);

    // --- draw the slider ---
    int x = offset2pixels(m_view_offset);

    p.begin (this);
    p.setBrush(SLIDER_BACKGROUND);
    p.drawRect(x, 0, m_slider_width, m_height);
    if (m_overview) {
	QBrush brush;
	brush.setPixmap(*m_overview);
	brush.setColor(SLIDER_FOREGROUND);
	p.setBrush(brush);
	p.drawRect(x, 0, m_slider_width, m_height);
    }

    p.setPen(colorGroup().light());
    p.drawLine(0, 0, m_width, 0);
    p.drawLine(0, 0, 0, m_height);

    p.drawLine(x, 0, x + m_slider_width, 0);
    p.drawLine(x, 0, x, m_height);
    p.drawLine(x + 1, 0, x + 1, m_height);

    p.setPen (colorGroup().dark());
    p.drawLine(1, m_height - 1, m_width, m_height - 1);
    p.drawLine(m_width - 1, 1, m_width - 1, m_height - 1);

    p.drawLine(x + 1, m_height - 2, x + m_slider_width, m_height - 2);
    p.drawLine(x + m_slider_width, 1, x + m_slider_width, m_height);
    p.drawLine(x + m_slider_width - 1, 1, x + m_slider_width - 1, m_height);

    p.end ();
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
//*****************************************************************************
