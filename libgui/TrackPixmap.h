/***************************************************************************
          TrackPixmap.h  -  buffered pixmap for displaying a kwave track
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

#ifndef _TRACK_PIXMAP_H_
#define _TRACK_PIXMAP_H_

#include <qarray.h>
#include <qobject.h>
#include <qpixmap.h>

#include "libkwave/Sample.h"

/**
  *@author Thomas Eschenbacher
  */
class TrackPixmap : public QObject, public QPixmap
{
    Q_OBJECT

public: 
    /** Default constructor */
    TrackPixmap();

    /** Destructor */
    virtual ~TrackPixmap();

    /**
     * Resize the pixmap.
     * @param width new width in pixels
     * @param height new height in pixels
     * @see QPixmap::resize()
     */
    void resize(int width, int height);

public slots:

    /**
     * Sets a new sample offset and moves the signal display
     * left or right. Only the new areas that were moved in
     * will be redrawn.
     * @param offset index of the first visible sample
     */
    void setOffset(unsigned int offset);

    /**
     * Sets a new zoom factor in samples per pixel. This normally
     * affects the number of visible samples and a redraw of
     * the current view.
     */
    void setZoom(double zoom);

private:

    /**
     * Index of the first sample. Needed for converting pixel
     * positions into absolute sample numbers.
     */
    unsigned int m_offset;

    /**
     * Zoom factor in samples/pixel. Needed for converting
     * sample indices into pixels and vice-versa.
     */
    double m_zoom;

    /**
     * If true, we are in min/max mode. This means that m_sample_buffer
     * is not used and m_minmax_buffer contains is used instead.
     */
    bool m_minmax_mode;

    /**
     * Array with samples needed for drawing when not in min/max mode.
     * This might sometimes include one sample before or after the
     * current view.
     */
    QArray<sample_t> m_sample_buffer;

    /**
     * Array with minimum sample values, if in min/max mode.
     */
    QArray<sample_t> m_min_buffer;

    /**
     * Array with maximum sample values, if in min/max mode.
     */
    QArray<sample_t> m_max_buffer;

};

#endif /* _TRACK_PIXMAP_H_ */
