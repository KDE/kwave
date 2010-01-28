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
     m_track_index(track)
{
    // ### only for illustration...
    QPalette palette;
    palette.setBrush(this->backgroundRole(), QBrush(Qt::blue));
    setPalette(palette);
    setAutoFillBackground(true);
    setMinimumSize(400, 100);
    if (controls) {
	palette.setBrush(this->backgroundRole(), QBrush(Qt::yellow));
	controls->setPalette(palette);
	controls->setAutoFillBackground(true);
	controls->setMinimumSize(50, 50);
    }

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
#include "SignalView.moc"
//***************************************************************************
//***************************************************************************
