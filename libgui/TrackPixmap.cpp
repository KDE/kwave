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
}

//***************************************************************************
void TrackPixmap::setZoom(double zoom)
{
}

//***************************************************************************
//***************************************************************************
