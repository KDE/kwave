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
#include <qbitarray.h>
#include <qobject.h>
#include <qpixmap.h>

#include "libkwave/Sample.h"
#include "mt/Mutex.h"

class Track;

/**
  *@author Thomas Eschenbacher
  */
class TrackPixmap : public QObject, public QPixmap
{
    Q_OBJECT

public: 
    /** Default constructor */
    TrackPixmap(Track &track);

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
     * Resizes the current buffer and sets all new entries to
     * invalid (if any).
     */
    void resizeBuffer();

    /**
     * Sets the current buffer to "invalid" state. Note: this does
     * not include any resize!
     */
    void invalidateBuffer();

    /**
     * Adapts the current mode and size of the buffers and fills all
     * areas that do not contain valid data with fresh samples. In other
     * words: it makes the buffer "valid" and consistent.
     */
    void validateBuffer();

    /**
     * Repaints the current pixmap.
     */
    void repaint();

    /**
     * Draws the signal as an overview with multiple samples per
     * pixel.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @param first the index of the first sample
     * @param last the index of the last sample
     */
    void drawOverviewSignal(int channel, int middle, int height,
			    int first, int last);

    /**
     * Calculates the parameters for interpolation of the graphical
     * display when zoomed in. Allocates (new) buffer for the
     * filter coefficients of the low pass filter used for interpolation.
     * @see #interpolation_alpha
     */
    void calculateInterpolation();

    /**
     * Draws the signal and interpolates the pixels between the
     * samples. The interpolation is done by using a simple FIR
     * lowpass filter.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @see #calculateInterpolation()
     */
    void drawInterpolatedSignal(int channel, int middle, int height);

    /**
     * Draws the signal and connects the pixels between the samples
     * by using a simple poly-line. This gets used if the current zoom
     * factor is not suitable for either an overview nor an interpolated
     * signal display.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     */
    void drawPolyLineSignal(int channel, int middle, int height);

    /**
     * Converts a pixel offset into a sample offset.
     */
    inline unsigned int pixels2samples(int pixels) {
	return (unsigned int)(pixels*m_zoom);
    }

    /**
     * Converts a sample offset into a pixel offset.
     */
    inline int samples2pixels(unsigned int samples) {
	if (m_zoom==0.0) return 0;
	return (int)(samples / m_zoom);
    }

    /**
     * Reference to the track with our sample data.
     */
    Track &m_track;

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

    /**
     * Array with one bit for each position in the internal
     * buffers. If the bit corresponding to a certain buffer
     * index is set to "1", the position contains valid data,
     * if "0" the content of the buffer is invalid.
     */
    QBitArray m_valid;

    /** Mutex for exclusive access to the buffers. */
    Mutex m_lock_buffer;

    /**
     * order of the low pass filter used for interpolation
     */
    int m_interpolation_order;

    /**
     * buffer for filter coefficients of the low pass used for
     * interpolation
     * @see #calculateInterpolation()
     */
    float *m_interpolation_alpha;

};

#endif /* _TRACK_PIXMAP_H_ */
