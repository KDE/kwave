/***************************************************************************
           ImageView.cpp  -  simple widget class for displaying a QImage
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
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

#include <QtGui/QCursor>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>

#include "libgui/ImageView.h"
#include "libkwave/Utils.h"

//****************************************************************************
Kwave::ImageView::ImageView(QWidget *parent, bool fit_width, bool fit_height)
    :QWidget(parent), m_offset(0,0), m_last_rect(0,0,0,0),
     m_image(), m_fit_width(fit_width), m_fit_height(fit_height),
     m_scale_x(1.0), m_scale_y(1.0)
{
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

//****************************************************************************
Kwave::ImageView::~ImageView()
{
}

//****************************************************************************
void Kwave::ImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;

    if (!m_image.width() || !m_image.height())
	return; // image not yet loaded

    int x = e->pos().x();
    int y = e->pos().y();

    if ((x > width()) || (x < 0)) return;
    if ((y > height()) || (y < 0)) return;

    /*
     * re-transform the coordinates from the screen (pixmap) coordinates to
     * the original image coordinates and emit a cursor position signal
     */
    x = m_offset.x() + Kwave::toInt((m_scale_x != 0) ?
	(static_cast<double>(x) / m_scale_x) : 0);
    y = m_offset.y() + Kwave::toInt((m_scale_y != 0) ?
	(static_cast<double>(y) / m_scale_y) : 0);
    emit sigCursorPos(QPoint(x, y));
}

//****************************************************************************
void Kwave::ImageView::setImage(QImage image)
{
    m_image = image;
    update();
}

//****************************************************************************
QRect Kwave::ImageView::imageRect()
{
    return QRect(m_offset.x(), m_offset.y(),
	         m_image.width(), m_image.height());
}

//****************************************************************************
void Kwave::ImageView::setHorizOffset(int offset)
{
    if (m_offset.x() != offset) {
	m_offset.setX(offset);
	repaint();
    }
}

//****************************************************************************
void Kwave::ImageView::setVertOffset(int offset)
{
    if (m_offset.y() != offset) {
	m_offset.setY(offset);
	repaint();
    }
}

//****************************************************************************
void Kwave::ImageView::paintEvent(QPaintEvent *)
{
    if (m_image.isNull() || !m_image.width() || !m_image.height()) return;

    QPainter p(this);
    QMatrix matrix;
    QPixmap newmap = QPixmap::fromImage(m_image,
	Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);

    m_scale_x = m_fit_width  ? static_cast<float>(width())  /
                               static_cast<float>(m_image.width()) : 1.0;
    m_scale_y = m_fit_height ? static_cast<float>(height()) /
                               static_cast<float>(m_image.height()) : 1.0;

    if (m_offset.x() + m_scale_x * m_image.width() > width())
	m_offset.setX(Kwave::toInt(m_scale_x * m_image.width() - width()));
    if (m_offset.y() + m_scale_y * m_image.height() > height())
	m_offset.setY(Kwave::toInt(
	    m_scale_y * m_image.height() - height()));

    matrix.scale(m_scale_x, m_scale_y);
    QPixmap pixmap = newmap.transformed(matrix);

    p.drawPixmap(0, 0, pixmap, m_offset.x(), m_offset.y(),
	width(), height()
    );

    m_last_rect = imageRect();
}

//***************************************************************************
#include "ImageView.moc"
//***************************************************************************
//***************************************************************************
