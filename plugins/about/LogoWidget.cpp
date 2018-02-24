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

#include "config.h"
#include <math.h>

#include <QBrush>
#include <QColor>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPolygon>
#include <QTimer>

#include "libkwave/Utils.h"

#include "LogoWidget.h"
#include "logo.xpm"

/** increment value of the "h" channel of the color of the sine waves */
#define COLOR_INCREMENT (static_cast<double>(0.001))

//***************************************************************************
Kwave::LogoWidget::LogoWidget(QWidget *parent)
    :QWidget(parent), m_width(-1), m_height(-1), m_repaint(false),
     m_image(Q_NULLPTR), m_logo(xpm_aboutlogo), m_timer(Q_NULLPTR),
     m_color_h(0.0)
{
    for (int i = 0; i < MAXSIN; m_deg[i++] = 0) {}

    m_timer = new QTimer(this);
    Q_ASSERT(m_timer);
    if (!m_timer) return;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(doAnim()));

    // gives 40ms refresh ;-)...
    m_timer->setInterval(40);
    m_timer->start();

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
}

//***************************************************************************
void Kwave::LogoWidget::doAnim()
{
    double mul = 0.04131211 + m_deg[MAXSIN-1] / 75;

    for (int i = 0; i < MAXSIN; i++) {
	m_deg[i] += mul;
	if (m_deg[i] > 2 * M_PI) m_deg[i] = 0;
	mul  = ((mul * 521)/437);
	mul -= floor(mul);  // gives again a number between 0 and 1
	mul /= 17;
	mul += m_deg[i] / 100; //so that chaos may be ...
    }

    m_repaint = true;
    repaint();
}

//***************************************************************************
Kwave::LogoWidget::~LogoWidget()
{
    if (m_timer) delete m_timer;
    if (m_image) delete m_image;
}

//***************************************************************************
void Kwave::LogoWidget::paintEvent(QPaintEvent *)
{
    // if image has to be resized ...
    if ((rect().height() != m_height) || (rect().width() != m_width)) {
	m_height = rect().height();
	m_width  = rect().width();

	if (m_image) delete m_image;
	m_image = new QImage(size(), QImage::Format_ARGB32_Premultiplied);
	m_repaint = true;
    }

    if ((m_repaint) && (m_image)) {
	QPainter p;
	QPolygon si(20 + 3);

	p.begin(m_image);

	// erase everything to black
	p.setPen(Qt::black);
	p.setBrush(Qt::black);
	p.drawRect(0, 0, m_width, m_height);

	// blit logo bitmap
	int ampx = (m_logo.width()  - m_width ) / 2;
	int ampy = (m_logo.height() - m_height) / 2;
	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.drawPixmap(
	    -ampx + Kwave::toInt(sin(m_deg[0]) * ampx),
	    -ampy + Kwave::toInt(sin(m_deg[1]) * ampy),
	    m_logo);

	// draw the sine waves with XOR
	p.setCompositionMode(QPainter::CompositionMode_Exclusion);
	p.setBrush(QColor::fromHsvF(m_color_h, 1.0, 1.0));
	m_color_h += COLOR_INCREMENT; // this gives the nice color change :-)
	if (m_color_h > 1.0) m_color_h -= 1.0;

	double amp = sin(m_deg[MAXSIN - 1] * 3);
	for (int j = 0; j < MAXSIN; j++) {
	    for (int i = 0; i < 21; i++) {
		si.setPoint(i, (j * m_width / MAXSIN) +
		    Kwave::toInt(amp * sin(M_PI * i / 10 + m_deg[j])
			* m_width / 2),
		    m_height * i / 20);
	    }
	    si.setPoint(21, m_width / 2, m_height);
	    si.setPoint(22, m_width / 2, 0);

	    p.drawPolygon(si);
	    amp = sin(m_deg[j] * 3);
	}

	p.end();
	m_repaint = false;
    }

    // blit the result to the display
    if (m_image) {
	QPainter p(this);
	p.drawImage(0, 0, *m_image);
	p.end();
    }

}

//***************************************************************************
//***************************************************************************
