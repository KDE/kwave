/***************************************************************************
    FrequencyResponseWidget.cpp  -  displays a frequency response
			     -------------------
    begin                : Mar 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include <stdlib.h>
#include <math.h>

#include <QtGui/QBrush>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "libkwave/TransmissionFunction.h"
#include "FrequencyResponseWidget.h"

//***************************************************************************
Kwave::FrequencyResponseWidget::FrequencyResponseWidget(QWidget *widget)
    :QWidget(widget), m_f_max(0), m_db_min(0), m_db_max(0),
     m_decades(0), m_function(0)
{
    init(10000, -12, +12);
}

//***************************************************************************
Kwave::FrequencyResponseWidget::~FrequencyResponseWidget()
{
}

//***************************************************************************
void Kwave::FrequencyResponseWidget::init(double freq, int db_min, int db_max)
{
    const int base = 10;
    m_decades = static_cast<int>(ceil(log(freq)/log(base)));
    m_f_max = pow(base, m_decades);

    m_db_min = db_min;
    m_db_max = db_max;
}

//***************************************************************************
void Kwave::FrequencyResponseWidget::setFilter(
    Kwave::TransmissionFunction *func)
{
    m_function = func;
    repaint();
}

//***************************************************************************
void Kwave::FrequencyResponseWidget::paintEvent(QPaintEvent*)
{
//  const int base = 10;
//  const double m_frequency = m_f_max * 2/3;
    const int width  = this->width();
    const int height = this->height();

    Q_ASSERT(width > 0);
    Q_ASSERT(height > 0);
    if ((width <= 0) || (height <= 0)) return;

    QPainter p(this);
    p.fillRect(rect(), QBrush(palette().dark()));

    double scale = static_cast<double>(height-1) /
	static_cast<double>(m_db_max-m_db_min);
    double min = pow(10.0, static_cast<double>(m_db_min) / 10.0);
    double max = pow(10.0, static_cast<double>(m_db_max) / 10.0);
    p.setPen(Qt::green);//colorGroup().text());

    for (int x=0; x < width; x++) {
	// transform x coordinate to frequency

//	// logarithmic frequency scale, didn't look so good :-(
//	double f = pow(base, (double)m_decades * (double)x / (double)width);

	// linear frequency scale
	double f = (m_f_max * static_cast<double>(x) /
	    static_cast<double>(width));

	// calculate the filter function's output at the given frequency
	f = (f / m_f_max) * M_PI;
	double a = (m_function) ? m_function->at(f): 1.0;

	// limit to upper and lower margins
	if (a < min) a = min;
	if (a > max) a = max;

	// convert to logarithmic scale
	double db = 10.0 * log10(a);

	// draw one line
	int y = height - static_cast<int>((db - m_db_min) * scale);

	p.drawLine(x, y+1, x, height-1);
    }

    // draw the zero db line
    p.setPen(palette().text().color());
    int y = height - static_cast<int>((0.0 - m_db_min) * scale);
    p.drawLine(0, y, width-1, y);
}

//***************************************************************************
#include "FrequencyResponseWidget.moc"
//***************************************************************************
//***************************************************************************
