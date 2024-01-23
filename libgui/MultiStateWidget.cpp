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

#include <QDir>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardPaths>
#include <QString>

#include "libkwave/String.h"

#include "libgui/MultiStateWidget.h"

//***************************************************************************
Kwave::MultiStateWidget::MultiStateWidget(QWidget *parent, int id)
    :QWidget(parent), m_current_index(0), m_identifier(id), m_pixmaps()
{
    resize(20, 20);
}

//***************************************************************************
Kwave::MultiStateWidget::~MultiStateWidget()
{
    m_pixmaps.clear();
}

//***************************************************************************
void Kwave::MultiStateWidget::setID(int id)
{
    m_identifier = id;
}

//***************************************************************************
void Kwave::MultiStateWidget::addPixmap(const QString &filename)
{
    QString pics_dir = _("pics");
    QString path = QStandardPaths::locate(
        QStandardPaths::AppDataLocation,
        pics_dir + QDir::separator() + filename,
        QStandardPaths::LocateFile);
    QPixmap newpix(path);
    m_pixmaps.append(newpix);
}

//***************************************************************************
void Kwave::MultiStateWidget::setState(int state)
{
    m_current_index = (state % m_pixmaps.count());
    repaint();
}

//***************************************************************************
void Kwave::MultiStateWidget::switchState(bool on)
{
    setState((on) ? 1 : 0);
}

//***************************************************************************
void Kwave::MultiStateWidget::nextState()
{
    setState(m_current_index + 1);
    emit clicked(m_identifier);
}

//***************************************************************************
void Kwave::MultiStateWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e && (e->button() == Qt::LeftButton)) {
        nextState();
    }
}

//***************************************************************************
void Kwave::MultiStateWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_pixmaps[m_current_index]);
}

//***************************************************************************
//***************************************************************************
