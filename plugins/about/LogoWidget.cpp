/***************************************************************************
        LogoWidget.cpp  -  widget with the animated Kwave logo
                             -------------------
    begin                : Sun Oct 29 2000
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

#include <math.h>

#include <qaccel.h>
#include <qdir.h>
#include <qimage.h>
#include <qkeycode.h>
#include <qpntarry.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kapp.h>

#include "logo.xpm"
#include "LogoWidget.h"

//**********************************************************
LogoWidget::LogoWidget(QWidget *parent)
    :QWidget(parent)
{
    for (int i=0; i < MAXSIN; m_deg[i++] = 0);

    m_buffer = 0;
    m_height = -1;
    m_pixmap = 0;
    m_width = -1;
    m_repaint = false;
    m_img = 0;
    m_timer = 0;

    m_img = new QPixmap(xpm_aboutlogo);
    ASSERT(m_img);
    if (!m_img) return;

    m_timer = new QTimer(this);
    ASSERT(m_timer);
    if (!m_timer) return;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(doAnim()));

    // gives 40ms refresh ;-)...
    m_timer->start(40, false);

    this->setBackgroundColor(black);
}

//**********************************************************
void LogoWidget::doAnim()
{
    double mul = 0.04131211+m_deg[MAXSIN-1] / 75;

    for (int i=0; i < MAXSIN; i++) {
	m_deg[i] += mul;
	if (m_deg[i] > 2*M_PI) m_deg[i]=0;
	mul  = ((mul*521)/437);
	mul -= floor(mul);  // gives again a number between 0 and 1
	mul /= 17;
	mul += m_deg[i] / 100; //so that chaos may be ...
    }

    m_repaint = true;
    repaint(false);
}

//**********************************************************
LogoWidget::~LogoWidget()
{
    if (m_img) delete m_img;
    m_timer->stop();
    delete m_timer;
}

//**********************************************************
void LogoWidget::paintEvent(QPaintEvent *)
{

    // if pixmap has to be resized ...
    if ((rect().height() != m_height) || (rect().width() != m_width)) {
	m_height = rect().height();
	m_width  = rect().width();

	if (m_pixmap) delete m_pixmap;
	if (m_buffer) delete m_buffer;

	m_pixmap = new QPixmap(size());
	m_buffer = new QPixmap(size());
	m_repaint = true;
    }
	
    if ((m_repaint) && (m_pixmap)) {
	QPainter p;
	QPointArray si(20+3);

	p.begin(m_pixmap);
	p.setPen(white);
	p.setBrush(white);
	p.drawRect(0, 0, m_width, m_height);
	p.setRasterOp(XorROP);

	double amp=sin(m_deg[MAXSIN-1]*3);
	for (int j=0; j < MAXSIN; j++) {
	    for (int i=0; i < 21; i++) {
		si.setPoint(i, (j*m_width/MAXSIN) +
		    (int)(amp*sin(M_PI*i/10+m_deg[j])*m_width/2),
		    m_height*i/20
		);
	    }
	    si.setPoint(21,m_width/2, m_height);
	    si.setPoint(22,m_width/2, 0);

	    p.drawPolygon(si);
	    amp=sin(m_deg[j]*3);
	}

	p.end();
	m_repaint = false;
    }

    // blit pixmap to window in every case
    int ampx = (m_img->width()  - m_width)/2;
    int ampy = (m_img->height() - m_height)/2;

    if (m_pixmap && m_buffer) {
	m_buffer->fill(black);
	bitBlt(m_buffer, -ampx+(int)(sin(m_deg[0]) * ampx),
	       -ampy + (int)(sin(m_deg[1]) * ampy),
	       m_img, 0, 0, m_img->width(), m_img->height(),
	       CopyROP);
	bitBlt(m_buffer, 0, 0, m_pixmap, 0, 0, m_width, m_height, XorROP);
	
	bitBlt(this, 0, 0, m_buffer, 0, 0, m_width, m_height, CopyROP);
    }
}

/* end of LogoWidget.cpp */
