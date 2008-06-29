/***************************************************************************
  MultiStateWidget.cpp  -  provides methods of multistateWidget a Class that
                           switches the image it, displays on clicking, used
                           for the channel enable/disable lamps...
			     -------------------
    begin                : Sun Jun 04 2000
    copyright            : (C) 2000 by Martin Wilz
    email                : martin@wilz.de
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

#include <QString>
#include <QPainter>
#include <QMouseEvent>

#include <kstandarddirs.h>

#include "MultiStateWidget.h"

//***************************************************************************
MultiStateWidget::MultiStateWidget(QWidget *parent, int id)
    :QWidget(parent), m_current_index(0), m_identifier(id), m_pixmaps()
{
    resize(20, 20);
}

//***************************************************************************
void MultiStateWidget::setID(int id)
{
    m_identifier = id;
}

//***************************************************************************
void MultiStateWidget::addPixmap(const QString &filename)
{
    QString file = KStandardDirs::locate(
	"data", QString("kwave/pics/") + filename);
    QPixmap newpix(file);
    m_pixmaps.append(newpix);
}

//***************************************************************************
void MultiStateWidget::setState(int state)
{
    int last_index = m_current_index;
    m_current_index = (state % m_pixmaps.count());
    repaint();
    if (m_current_index != last_index)
	emit clicked(m_identifier);
}

//***************************************************************************
void MultiStateWidget::nextState()
{
    setState(m_current_index + 1);
}

//***************************************************************************
void MultiStateWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e && (e->button() == Qt::LeftButton)) {
	nextState();
    }
}

//***************************************************************************
MultiStateWidget::~MultiStateWidget()
{
    m_pixmaps.clear();
}

//***************************************************************************
void MultiStateWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_pixmaps[m_current_index]);
}

//***************************************************************************
#include "MultiStateWidget.moc"
//***************************************************************************
//***************************************************************************
