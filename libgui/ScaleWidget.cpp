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

#include <math.h>
#include <stdlib.h>

#include <QFont>
#include <QPaintEvent>
#include <QPainter>
#include <QtGlobal>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/ScaleWidget.h"

#define FONTSIZE 6

//***************************************************************************
Kwave::ScaleWidget::ScaleWidget(QWidget *parent)
    :QWidget(parent), m_low(0), m_high(100), m_logmode(false),
     m_unittext(_("%"))
{
}

//***************************************************************************
Kwave::ScaleWidget::ScaleWidget(QWidget *parent, int low, int high,
                                const QString &unit)
    :QWidget(parent), m_low(low), m_high(high), m_logmode(false),
     m_unittext(unit)
{
}

//***************************************************************************
Kwave::ScaleWidget::~ScaleWidget()
{
}

//***************************************************************************
void Kwave::ScaleWidget::setUnit(const QString &text)
{
    m_unittext = text;
    repaint();
}

//***************************************************************************
void Kwave::ScaleWidget::setLogMode(bool log)
{
    if (m_logmode == log) return;
    m_logmode = log;
    repaint();
}

//***************************************************************************
void Kwave::ScaleWidget::setMinMax(int min, int max)
{
    if ((m_low == min) && (m_high == max)) return;
    m_low  = min;
    m_high = max;
    repaint();
}

//***************************************************************************
void Kwave::ScaleWidget::paintText(QPainter &p, int x, int y,
                                   bool reverse, const QString &text)
{
    QFont font;
    font.setStyleHint(QFont::SansSerif);
    font.setFixedPitch(true);
    font.setPixelSize(FONTSIZE);
    font.setWeight(0);
    font.setStyle(QFont::StyleNormal);
    p.setFont(font);
    QFontMetrics fm(font);

    if (reverse) {
	x += FONTSIZE + 2;
	y -= FONTSIZE + 2;
    }

    QRect rect = fm.boundingRect(text);
    const int th = rect.height();
    const int tw = rect.width();
    p.drawText(x, y, tw, th,
	((reverse) ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignBottom,
	text);
}

//***************************************************************************
void Kwave::ScaleWidget::drawLog(QPainter &p, int w, int h, bool inverse)
{
    // only use base 10 for now, tested with others too,
    // but not configurable through a property
    const int base = 10;

    int dir = (inverse) ? -1 : +1;

    p.setPen(palette().dark().color());
    p.drawLine (0, dir*(h-1), dir*w, dir*(h-1));
    p.drawLine (dir*(w-1), 0, dir*(w-1), dir*(h-1));

    p.setPen(palette().text().color());

    int a, x;
    const int h2 = h;

    Q_ASSERT(m_low >= 0);
    Q_ASSERT(m_high > m_low);

    int dec_lo = (m_low) ? Kwave::toInt(floor(log(m_low)/log(base))) : 0;
    int dec_hi = Kwave::toInt(ceil(log(m_high)/log(base)));
    int decades = qAbs(dec_hi - dec_lo) + 1;

    // check if we have enough space for the small lines within a decade
    int w1 = Kwave::toInt(w / decades); // pixels per decade
    bool small_lines = (w1 - Kwave::toInt(
	static_cast<double>(w1) * log(base-1)/log(base))) > 1;

    // print the lines
    for (a = 0; a < decades; ++a) {
	// big line, for each decade
	x = Kwave::toInt((w-1) * a / decades);
	p.drawLine (dir * x, dir * 1, dir * x, dir * (h2 - 2));

	w1 = Kwave::toInt((w - 1) * (a + 1) / decades) - x + 1;
	if (small_lines) {
	    // small lines, within the decade
	    for (int i = 1; i < base; i++) {
		int x1 = x + Kwave::toInt(static_cast<double>(w1) *
		    log(i) / log(base));
		p.drawLine (dir * x1, dir * 1, dir * x1, dir * ((h2 / 2) - 2));
	    }
	}
    }

    // print the text
    for (a = 0; a < decades; ++a) {
	QString buf = _("%1 %2");
	int value = Kwave::toInt(pow(base, dec_lo + a));
	buf = buf.arg(value).arg(m_unittext);
	x = ((w - 1) * a) / decades;
	paintText(p, dir * (x + 4), dir * (h - FONTSIZE - 4), inverse, buf);
    }
}

//***************************************************************************
void Kwave::ScaleWidget::drawLinear(QPainter &p, int w, int h, bool inverse)
{
    int dir = (inverse) ? -1 : +1;

    p.setPen(palette().dark().color());
    p.drawLine(0, dir * (h - 1), dir * w, dir * (h - 1));
    p.drawLine(dir * (w - 1), 0, dir * (w - 1), dir * (h - 1));

    p.setPen(palette().text().color());

    int a, x;
    double ofs;
    double t = w - 1;
    int h2 = h;

    // print the lines
    while ((t / 10 > 1) && (h2 > 0)) {
	for (ofs = 0; ofs < w - 1; ofs += t) {
	    for (a = 0; a < 4; ++a) {
		x = Kwave::toInt(ofs + (t * a / 4));
		p.drawLine (dir * x, dir * 1, dir * x, dir * (h2 - 2));
	    }
	}
	h2 >>= 1;
	t /= 4;
    }

    // print the text
    for (a = 0; a < 4; ++a) {
	QString buf = _("%1 %2");
	int value = m_low + (((m_high - m_low)* (inverse ? (4 - a) : a)) / 4);
	buf = buf.arg(value).arg(m_unittext);
	x = ((w - 1) * a) / 4;
	paintText(p, dir * (x + 4), dir * (h - FONTSIZE - 4), inverse, buf);
    }

}

//***************************************************************************
void Kwave::ScaleWidget::paintEvent(QPaintEvent *)
{
    bool inverse = false;
    int h = height();
    int w = width();
    QPainter p;

    p.begin(this);
    p.save();
    p.setPen(palette().light().color());

    p.drawLine(0, 0, w, 0);
    if (h > w) {
	p.setWindow(-w, 0, w, h);
	p.rotate(-90);
	h = width();
	w = height();

	inverse = true;
    }

    (m_logmode) ? drawLog(p, w, h, inverse) : drawLinear(p, w, h, inverse);

    p.restore();
    p.end();
}

//***************************************************************************
QSize Kwave::ScaleWidget::sizeHint() const
{
    return QSize(4 * FONTSIZE, 4 * FONTSIZE);
}

//***************************************************************************
QSize Kwave::ScaleWidget::minimumSize() const
{
    return QSize(5 * 2 * FONTSIZE, 5 * 2 * FONTSIZE);
}

//***************************************************************************
//***************************************************************************
