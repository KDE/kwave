/***************************************************************************
    TrackView.cpp  -  signal views that shows the track in time space
			     -------------------
    begin                : Sat Jan 30 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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
#include <QPalette>
#include <QResizeEvent>

#include "TrackView.h"

/** interval for limiting the number of repaints per second [ms] */
#define REPAINT_INTERVAL 125

//***************************************************************************
Kwave::TrackView::TrackView(QWidget *parent, QWidget *controls,
                            SignalManager *signal_manager,
                            QPointer<Track> track)
    :Kwave::SignalView(parent, controls, signal_manager,
                       Kwave::SignalView::BelowTrackTop),
     m_pixmap(*track),
     m_repaint_timer()
{
    setMinimumSize(400, 100);

    // trigger a repaint request when the signal has been modified
    connect(&m_pixmap, SIGNAL(sigModified()),
            this,      SLOT(needRepaint()));

    // use a timer for limiting the repaint rate
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this,             SLOT(refreshBitmap()));

    if (controls) {
	// add the channel controls, for "enabled" / "disabled"
	// ### TODO ###
    }

}

//***************************************************************************
Kwave::TrackView::~TrackView()
{
}

//***************************************************************************
void Kwave::TrackView::setZoomAndOffset(double zoom, sample_index_t offset)
{
    Kwave::SignalView::setZoomAndOffset(zoom, offset);
    m_pixmap.setZoom(zoom);
    m_pixmap.setOffset(offset);
    if (m_pixmap.isModified())
	needRepaint();
}

//***************************************************************************
void Kwave::TrackView::resizeEvent(QResizeEvent *event)
{
    Kwave::SignalView::resizeEvent(event);
    if (!event) return;

    // request a repaint on all size changes, but not on horizontal shrink
    if ((event->size().width() > event->oldSize().width()) ||
	(event->size().height() != event->oldSize().height()))
    {
	needRepaint();
    }
}

//***************************************************************************
void Kwave::TrackView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::white);
    p.drawLine(0, 0, width() - 1, height() - 1);

    m_pixmap.resize(width(), height());
    p.drawPixmap(rect(), m_pixmap.pixmap());
}

//***************************************************************************
void Kwave::TrackView::needRepaint()
{
//     qDebug("Kwave::TrackView[%d]::needRepaint()", track());
    // repainting is inhibited -> wait until the
    // repaint timer is elapsed
    if (m_repaint_timer.isActive()) {
	return;
    } else {
	// repaint once and once later...
	m_pixmap.repaint();
	repaint();

	// start the repaint timer
	m_repaint_timer.setSingleShot(true);
	m_repaint_timer.start(REPAINT_INTERVAL);
    }
}

//***************************************************************************
void Kwave::TrackView::refreshBitmap()
{
//     qDebug("Kwave::TrackView[%d]::refreshBitmap()", track());
    m_pixmap.repaint();
    repaint();
}

//***************************************************************************
#include "TrackView.moc"
//***************************************************************************
//***************************************************************************
