/***************************************************************************
        TrackPixmap.cpp  -  buffered pixmap for displaying a kwave track
                             -------------------
    begin                : Tue Mar 20 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <qpainter.h>

#include "libgui/TrackPixmap.h"

//***************************************************************************
TrackPixmap::TrackPixmap()
    :QPixmap(), m_offset(0), m_zoom(0.0), m_minmax_mode(false),
    m_sample_buffer(), m_min_buffer(), m_max_buffer()
{
    debug("TrackPixmap::TrackPixmap()"); // ###
}

//***************************************************************************
TrackPixmap::~TrackPixmap()
{
    debug("TrackPixmap::~TrackPixmap()"); // ###
}

//***************************************************************************
void TrackPixmap::setOffset(unsigned int offset)
{
    debug("TrackPixmap::setOffset(%u)", offset);
    m_offset = offset;
}

//***************************************************************************
void TrackPixmap::setZoom(double zoom)
{
    debug("TrackPixmap::setZoom(%0.3f)", zoom);
    m_zoom = zoom;
}

//***************************************************************************
//	    if (!m_signal_manager.isEmpty()) {
//		if (m_zoom < 0.1) {
//		    drawInterpolatedSignal(i, zero, track_height);
//		} else if (m_zoom <= 1.0)
//		    drawPolyLineSignal(i, zero, track_height);
//		else
//		    drawOverviewSignal(i, zero, track_height,
//		                       0, m_zoom*width);
//	    }
//
//	    // draw the baseline
//	    p.setPen(green);
//	    p.drawLine(0, zero, width, zero);
//	    p.setPen(white);
//	    zero += track_height;

//***************************************************************************
void TrackPixmap::resize(int width, int height)
{
    debug("TrackPixmap::resize(%d, %d)", width, height); // ###

    QPixmap::resize(width, height);

    QPainter p(this);
    p.setRasterOp(CopyROP);
    p.fillRect(0, 0, width, height, black);

    p.setPen(green);
    p.drawLine(0, 0, width-1, height-1);
    p.drawLine(0, height-1, width-1, 0);

    p.flush();
    p.end();

}

//***************************************************************************
//***************************************************************************
