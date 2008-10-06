/***************************************************************************
     OverViewWidget.cpp  -  horizontal slider with overview over a signal
                             -------------------
    begin                : Tue Oct 21 2000
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

#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVBoxLayout>

#include "OverViewWidget.h"

/* interval for limiting the number of repaints per second [ms] */
#define REPAINT_INTERVAL 500

#define BAR_BACKGROUND    palette().mid().color()
#define BAR_FOREGROUND    palette().light().color()

//***************************************************************************
OverViewWidget::OverViewWidget(SignalManager &signal, QWidget *parent)
    :ImageView(parent), m_view_offset(0), m_view_width(0), m_signal_length(0),
     m_last_offset(0), m_cache(signal), m_repaint_timer()
{
    // update the bitmap if the cache has changed
    connect(&m_cache, SIGNAL(changed()), this, SLOT(overviewChanged()));

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(refreshBitmap()));

    setMouseTracking(true);
}

//***************************************************************************
OverViewWidget::~OverViewWidget()
{
}

//***************************************************************************
void OverViewWidget::mouseMoveEvent(QMouseEvent *e)
{
    mousePressEvent(e);
}

//***************************************************************************
void OverViewWidget::mousePressEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;
    if (e->buttons() != Qt::LeftButton) {
	e->ignore();
	return;
    }

    // move the clicked position to the center of the viewport
    unsigned int offset = pixels2offset(e->x());
    if (offset != m_last_offset) {
	m_last_offset = offset;
	emit valueChanged(offset);
    }
    e->accept();
}

//***************************************************************************
void OverViewWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;
    if (e->button() != Qt::LeftButton) {
	e->ignore();
	return;
    }

    // move the clicked position to the center of the viewport
    unsigned int offset = pixels2offset(e->x());
    if (offset != m_last_offset) {
	m_last_offset = offset;
	emit valueChanged(offset);
    }

    if (e->modifiers() == Qt::NoModifier) {
	// double click without shift => zoom in
	emit sigCommand("zoomin()");
    } else if (e->modifiers() == Qt::ShiftModifier) {
	// double click with shift => zoom out
	emit sigCommand("zoomout()");
    }

    e->accept();
}

//***************************************************************************
int OverViewWidget::pixels2offset(int pixels)
{
    int width = this->width();
    if (!width) return 0;

    int offset = static_cast<int>(m_signal_length *
	(static_cast<qreal>(pixels) / static_cast<qreal>(width)));
    int center = m_view_width >> 1;
    offset = (offset > center) ? (offset - center) : 0;
    return offset;
}

//***************************************************************************
void OverViewWidget::setRange(unsigned int offset, unsigned int viewport,
                              unsigned int total)
{
    m_view_offset   = offset;
    m_view_width    = viewport;
    m_signal_length = total;
}

//***************************************************************************
void OverViewWidget::resizeEvent(QResizeEvent *)
{
    refreshBitmap();
}

//***************************************************************************
QSize OverViewWidget::minimumSize() const
{
    return QSize(30, 30);
}

//***************************************************************************
QSize OverViewWidget::sizeHint() const
{
    return minimumSize();
}

//***************************************************************************
void OverViewWidget::overviewChanged()
{
    // repainting is inhibited -> wait until the
    // repaint timer is elapsed
    if (m_repaint_timer.isActive()) {
	return;
    } else {
	// repaint once and once later...
	refreshBitmap();

	// start the repaint timer
	m_repaint_timer.setSingleShot(true);
	m_repaint_timer.start(REPAINT_INTERVAL);
    }
}

//***************************************************************************
void OverViewWidget::refreshBitmap()
{
    int width  = this->width();
    int height = this->height();
    if (!width || !height)
	return;

    // let the bitmap be updated from the cache
    QBitmap bitmap = m_cache.getOverView(width, height);

    // convert the bitmap into a QImage
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    QPainter p;
    p.begin(&image);
    p.fillRect(rect(), BAR_BACKGROUND);
    QBrush brush(bitmap);
    brush.setColor(BAR_FOREGROUND);
    p.setBrush(brush);
    p.drawRect(0, 0, width, height);
    p.end();

    // update the widget with the overview
    setImage(image);
}

//***************************************************************************
#include "OverViewWidget.moc"
//***************************************************************************
//***************************************************************************
