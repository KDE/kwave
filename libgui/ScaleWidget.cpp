/***************************************************************************
        ScaleWidget.cpp  -  widget for drawing a scale under an image
			     -------------------
    begin                : Sep 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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
#include <stdio.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qdir.h>
#include <qpainter.h>

#include <kiconloader.h>
#include <kstddirs.h>

#include "ScaleWidget.h"

#define FONTSIZE 6

//***************************************************************************
ScaleWidget::ScaleWidget(QWidget *parent, const char *name)
    :QWidget(parent, name), m_low(0), m_high(100), m_logmode(false),
     m_unittext("%")
{
    KIconLoader icon_loader;
    m_scalefont = icon_loader.loadIcon("minifont.xpm", KIcon::Small);
}

//***************************************************************************
ScaleWidget::ScaleWidget(QWidget *parent, int low, int high,
                         const QString &unit)
    :QWidget(parent), m_low(low), m_high(high), m_logmode(false),
     m_unittext(unit)
{
    KIconLoader icon_loader;
    m_scalefont = icon_loader.loadIcon("minifont.xpm", KIcon::Small);
}

//***************************************************************************
ScaleWidget::~ScaleWidget()
{
    m_scalefont.resize(0,0);
}

//***************************************************************************
void ScaleWidget::setUnit(const QString &text)
{
    m_unittext = text;
    repaint();
}

//***************************************************************************
void ScaleWidget::setLogMode(bool log)
{
    if (m_logmode == log) return;
    m_logmode = log;
    repaint();
}

//***************************************************************************
void ScaleWidget::setMinMax(int min, int max)
{
    if ((m_low == min) && (m_high == max)) return;
    m_low  = min;
    m_high = max;
    repaint();
}

//***************************************************************************
void ScaleWidget::paintText(QPainter &p, int x, int y, int ofs,
                            bool reverse, const QString &text)
{
    int len = text.length();
    int pos; // position within the font bitmap

    if (reverse) x -= ofs;
    for (int i = 0; i < len; i++) {
	pos = (reverse) ? (len-1-i) : i;
	int c = text.at(pos).latin1();

	pos = 40; // default = space	
	switch (c) {
	    case '\'':  pos = 36; break;
	    case '"':   pos = 37; break;
	    case '°':   pos = 38; break;
	    case '.':   pos = 39; break;
	    case ' ':   pos = 40; break;
	    case '%':   pos = 41; break;
	    case '-':   pos = 42; break;
	    default:
		if ((c > 64) && (c < 91))  pos = c-65;    // letter
		if ((c > 96) && (c < 123)) pos = c-97;    // letter
		if ((c > 47) && (c < 58))  pos = c-48+26; // number
	}
	
	p.drawPixmap(x, y, m_scalefont, pos*FONTSIZE, 0, FONTSIZE, FONTSIZE);
	
	x += (reverse) ? (-ofs) : (+ofs);
    }
}

//***************************************************************************
void ScaleWidget::drawLog(QPainter &p, int w, int h, bool inverse)
{
    int dir = (inverse) ? -1 : +1;

    p.setPen (colorGroup().dark());
    p.drawLine (0, dir*(h-1), dir*w, dir*(h-1));
    p.drawLine (dir*(w-1), 0, dir*(w-1), dir*(h-1));

    p.setPen (colorGroup().text());
}

//***************************************************************************
void ScaleWidget::drawLinear(QPainter &p, int w, int h, bool inverse)
{
    int dir = (inverse) ? -1 : +1;

    p.setPen(colorGroup().dark());
    p.drawLine(0, dir*(h-1), dir*w, dir*(h-1));
    p.drawLine(dir*(w-1), 0, dir*(w-1), dir*(h - 1));

    p.setPen (colorGroup().text());

    int a, x;
    double ofs;
    double t = w - 1;
    int h2 = h;

    // print the lines
    while ((t / 10 > 1) && (h2 > 0)) {
	for (ofs = 0; ofs < w - 1; ofs += t) {
	    for (a = 0; a < 6; a++) {
		x = (int)(ofs + (t * a / 5));
		p.drawLine (dir*x, dir*1, dir*x, dir*(h2-2));
	    }
	}
	h2 >>= 1;
	t /= 5;
    }

    // print the text
    for (a = 0; a < 5; a++) {
	char buf[64];
	snprintf(buf, sizeof(buf), "%d %s", m_low + (((m_high - m_low)*a)/5),
	         m_unittext.data());
	x = ((w-1) * a)/5;
	paintText(p, dir*(x+2), dir*(h-FONTSIZE-2), FONTSIZE, inverse, buf);
    }

}

//***************************************************************************
void ScaleWidget::paintEvent(QPaintEvent *)
{
    bool inverse = false;
    int h = height();
    int w = width();
    QPainter p;

    p.begin(this);
    p.save();
    p.setPen(colorGroup().light());

    p.drawLine(0, 0, w, 0);
    if (h > w) {
	p.setWindow(-w, 0, w, h);
	p.rotate(-90);
	h = width();
	w = height();
	
	inverse = true;
    };

    (m_logmode) ? drawLog(p, w, h, inverse) : drawLinear(p, w, h, inverse);

    p.restore();
    p.end();
}

//****************************************************************************
const QSize ScaleWidget::sizeHint()
{
    return QSize(4*FONTSIZE, 4*FONTSIZE);
}

//****************************************************************************
const QSize ScaleWidget::minimumSize()
{
    return QSize(5*2*FONTSIZE, 5*2*FONTSIZE);
}

//****************************************************************************
//****************************************************************************
