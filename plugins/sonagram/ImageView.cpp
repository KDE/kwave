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
#include <qimage.h>
#include <qcursor.h>
#include "ImageView.h"

//****************************************************************************
ImageView::ImageView(QWidget *parent, bool fit_width, bool fit_height)
    :QWidget(parent), m_offset(0,0), m_last_rect(0,0,0,0)
{
    m_image = 0;
    m_fit_width = fit_width;
    m_fit_height = fit_height;
    m_scale_x = 1.0;
    m_scale_y = 1.0;

    setCursor(crossCursor);
    setMouseTracking(true);
}

//****************************************************************************
ImageView::~ImageView()
{
    if (m_image) delete m_image;
}

//****************************************************************************
void ImageView::mouseMoveEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;

    if (!m_image) return; // image not yet loaded

    int x = e->pos().x();
    int y = e->pos().y();

    if ((x > width()) || (x < 0)) return;
    if ((y > height()) || (y < 0)) return;

    /*
     * re-transform the coordinates from the screen (pixmap) coordinates to
     * the original image coordinates and emit a cursor position signal
     */
    x = m_offset.x() + (int)((m_scale_x != 0) ? ((double)x / m_scale_x) : 0);
    y = m_offset.y() + (int)((m_scale_y != 0) ? ((double)y / m_scale_y) : 0);
    emit sigCursorPos(QPoint(x, y));
}

//****************************************************************************
void ImageView::setImage(const QImage *image)
{
    if (m_image) delete m_image;
    m_image = 0;

    if (image) m_image = new QImage(*image);

    repaint();
}

//****************************************************************************
QRect ImageView::imageRect()
{
    return QRect(m_offset.x(), m_offset.y(),
	m_image ? m_image->width() : 0, m_image ? m_image->height() : 0);
}

//****************************************************************************
void ImageView::setHorizOffset(int offset)
{
    if (m_offset.x() != offset) {
	m_offset.setX(offset);
	repaint();
    }
}

//****************************************************************************
void ImageView::setVertOffset(int offset)
{
    if (m_offset.y() != offset) {
	m_offset.setY(offset);
	repaint();
    }
}

//****************************************************************************
void ImageView::paintEvent(QPaintEvent *)
{
    if (!m_image) return;

    Q_ASSERT(m_image->width());
    Q_ASSERT(m_image->height());
    if (!m_image->width()) return;
    if (!m_image->height()) return;
	
    QWMatrix matrix;
    QPixmap newmap;
    newmap.convertFromImage(*m_image, ColorOnly|ThresholdDither|AvoidDither);

    m_scale_x = m_fit_width ? (float)width()  / (float)m_image->width()  : 1.0;
    m_scale_y = m_fit_height ?(float)height() / (float)m_image->height() : 1.0;

    if (m_offset.x() + m_scale_x * m_image->width() > width())
	m_offset.setX( (int)(m_scale_x * m_image->width() - width()));
    if (m_offset.y() + m_scale_y * m_image->height() > height())
	m_offset.setY( (int)(m_scale_y * m_image->height() - height()));

    matrix.scale(m_scale_x, m_scale_y);
    m_pixmap = newmap.xForm(matrix);

    bitBlt(this, 0, 0, &m_pixmap,
	m_offset.x(), m_offset.y(),
	width(), height()
    );

    if (imageRect() != m_last_rect) {
// ###	emit viewInfo (offset, width, image->width());
	m_last_rect = imageRect();
    }

}

//****************************************************************************
//****************************************************************************
