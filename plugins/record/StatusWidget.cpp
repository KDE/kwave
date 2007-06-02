/*************************************************************************
       StatusWidget.cpp  -  little widget with status icons
                             -------------------
    begin                : Sat Apr 15 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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
#include "StatusWidget.h"

//***************************************************************************
StatusWidget::StatusWidget(QWidget *parent)
    :QWidget(parent), m_pixmaps(), m_index(0), m_timer()
{
    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(nextPixmap()));
}

//***************************************************************************
StatusWidget::~StatusWidget()
{
    m_timer.stop();
    m_pixmaps.clear();
}

//***************************************************************************
void StatusWidget::paintEvent(QPaintEvent *)
{
    if (!m_pixmaps.count()) return;

    QPixmap pixmap = m_pixmaps.at(m_index);
    const int ww = width();
    const int wh = height();
    const int pw = pixmap.width();
    const int ph = pixmap.height();

    bitBlt(this, (ww - pw) >> 1, (wh - ph) >> 1, &pixmap, 0, 0,
           pw, ph, CopyROP);
}

//***************************************************************************
void StatusWidget::setPixmaps(const QValueVector<QPixmap> &pixmaps,
                              unsigned int speed)
{
    m_timer.stop();
    m_pixmaps.clear();
    m_pixmaps = pixmaps;
    m_index = 0;
    repaint(true);

    if (m_pixmaps.count() > 1) m_timer.start(speed, false);
}

//***************************************************************************
void StatusWidget::nextPixmap()
{
    m_index++;
    if (m_index >= m_pixmaps.count())
	m_index = 0;
    repaint(true);
}

//***************************************************************************
#include "StatusWidget.moc"
//***************************************************************************
//***************************************************************************
