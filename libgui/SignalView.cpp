/***************************************************************************
    SignalView.cpp  -  base class for widgets for views to a signal
			     -------------------
    begin                : Mon Jan 18 2010
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

#include "SignalView.h"

//***************************************************************************
Kwave::SignalView::SignalView(QWidget *parent, QWidget *controls,
                              SignalManager *signal_manager,
                              Location preferred_location,
                              int track)
    :QWidget(parent),
     m_controls(controls),
     m_signal_manager(signal_manager),
     m_preferred_location(preferred_location),
     m_track_index(track),
     m_offset(0),
     m_zoom(1.0)
{
}

//***************************************************************************
Kwave::SignalView::~SignalView()
{
}

//***************************************************************************
void Kwave::SignalView::setTrack(int track)
{
    m_track_index = (track >= 0) ? track : -1;
}

//***************************************************************************
void Kwave::SignalView::setZoomAndOffset(double zoom, sample_index_t offset)
{
    if ((zoom == m_zoom) && (offset == m_offset)) return;
    m_zoom   = zoom;
    m_offset = offset;

    sample_index_t visible = ((width() - 1) * zoom) + 1;
    sample_index_t last = offset + visible - 1;
    qDebug("SignalView::setZoomAndOffset(%g, %lu), last visible=%lu",
	   zoom,
	   static_cast<unsigned long int>(offset),
	   static_cast<unsigned long int>(last));

}

//***************************************************************************
#include "SignalView.moc"
//***************************************************************************
//***************************************************************************
