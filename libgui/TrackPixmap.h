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

#ifndef TRACK_PIXMAP_H
#define TRACK_PIXMAP_H

#include "config.h"

#include <math.h>

#include <QBitArray>
#include <QColor>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QVector>

#include <TODO:kdemacros.h>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Utils.h"

#include "libgui/Colors.h"

/**
 * The TrackPixmap is a graphical representation of a track's sample
 * data. It is connected directly to a Track object so that it is able
 * to get any needed sample data on it's own. It provides internal
 * caching mechanisms for reducing (slow) accesses to the track; this
 * is especially needed for speeding up the handling large wav files.
 *
 * @note The sample ranges in this class are a kind of "virtual", it is
 *       possible that the length of this pixmap in samples is larger
 *       than the track. However, this should not do any harm and might
 *       even be useful if a track grows.
 *
 * @todo If the "interpolated mode" is used, the sample buffer should
 *       contain some samples before and some samples after the current
 *       view. (m_extra_samples, calculated in set_zoom, !=0 only in
 *       interpolation mode, ignored in all other modes.
 *
 * @todo Check setOffset()
 * @todo optimizations if zoom factor is multiple of last zoom factor
 * @todo optimizations in slotSamplesDeleted and slotSamplesInserted if
 *       parts of the current buffers can be re-used
 *
 * @author Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 */
namespace Kwave
{
    class Track;

    class Q_DECL_EXPORT TrackPixmap: public QObject
    {
	Q_OBJECT

    public:

	/** Default constructor */
	explicit TrackPixmap(Kwave::Track &track);

	/** Destructor */
	virtual ~TrackPixmap();

	/**
	 * Resize the pixmap.
	 * @param width new width in pixels
	 * @param height new height in pixels
	 */
	virtual void resize(int width, int height);

	/**
	 * Get the width of the pixmap
	 * @return the width of the pixmap in pixels
	 */
	virtual int width() const { return m_pixmap.width(); }

	/**
	 * Get the height of the pixmap
	 * @return the height of the pixmap in pixels
	 */
	virtual int height() const { return m_pixmap.height(); }

	/**
	 * Get the internal QPixmap object
	 * @return reference to m_pixmap
	 */
	virtual const QPixmap &pixmap() const { return m_pixmap; }

	/**
	 * Repaints the current pixmap. After the repaint the pixmap is no
	 * longer in status "modified". If it was not modified before, this
	 * is a no-op.
	 */
	virtual void repaint();

	/**
	 * Sets a new vertical zoom factor.
	 * @param zoom new vertical zoom
	 */
	virtual void setVerticalZoom(double zoom);

	/**
	 * Returns "true" if the buffer has changed and the pixmap has to
	 * be re-painted.
	 */
	virtual bool isModified();

	/** Sets the internal "modified" flag */
	virtual void setModified() { m_modified = true; }

    signals:

	/** Emitted if the content of the pixmap was modified. */
	void sigModified();

    public slots:

	/**
	 * Sets a new sample offset and moves the signal display
	 * left or right. Only the new areas that were moved in
	 * will be redrawn.
	 * @param offset index of the first visible sample
	 */
	void setOffset(sample_index_t offset);

	/**
	 * Sets a new zoom factor in samples per pixel. This normally
	 * affects the number of visible samples and a redraw of
	 * the current view.
	 */
	void setZoom(double zoom);

    private slots:

	/**
	 * Connected to the track's sigSamplesInserted.
	 * @param src source track
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see Track::sigSamplesInserted
	 * @internal
	 */
	void slotSamplesInserted(Kwave::Track *src, sample_index_t offset,
				sample_index_t length);

	/**
	 * Connected to the track's sigSamplesDeleted.
	 * @param src source track
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 * @see Track::sigSamplesDeleted
	 * @internal
	 */
	void slotSamplesDeleted(Kwave::Track *src, sample_index_t offset,
				sample_index_t length);

	/**
	 * Connected to the track's sigSamplesModified
	 * @param src source track
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 * @see Track::sigSamplesModified
	 * @internal
	 */
	void slotSamplesModified(Kwave::Track *src, sample_index_t offset,
				sample_index_t length);

	/**
	 * Sets the state of the pixmap to "modified" whenever the
	 * selection of the track has changed
	 */
	void selectionChanged();

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
	 * @return true if the buffer content has changed, false if no
	 *         invalid samples were found
	 */
	bool validateBuffer();

	/**
	 * Draws the signal as an overview with multiple samples per
	 * pixel.
	 * @param p reference to a QPainter
	 * @param middle the y position of the zero line [pixels]
	 * @param height the height of the pixmap [pixels]
	 * @param first the offset of the first pixel
	 * @param last the offset of the last pixel
	 */
	void drawOverview(QPainter &p, int middle, int height,
	    int first, int last);

	/**
	 * Calculates the parameters for interpolation of the graphical
	 * display when zoomed in. Allocates (new) buffer for the
	 * filter coefficients of the low pass filter used for interpolation.
	 * @see m_interpolation_alpha
	 */
	void calculateInterpolation();

	/**
	 * Draws the signal and interpolates the pixels between the
	 * samples. The interpolation is done by using a simple FIR
	 * lowpass filter.
	 * @param p reference to a QPainter
	 * @param width the width of the pixmap in pixels
	 * @param middle the y position of the zero line in the drawing
	 *               area [pixels]
	 * @param height the height of the drawing are [pixels]
	 * @see #calculateInterpolation()
	 */
	void drawInterpolatedSignal(QPainter &p, int width, int middle,
	    int height);

	/**
	 * Draws the signal and connects the pixels between the samples
	 * by using a simple poly-line. This gets used if the current zoom
	 * factor is not suitable for either an overview nor an interpolated
	 * signal display.
	 * @param p reference to a QPainter
	 * @param width the width of the pixmap in pixels
	 * @param middle the y position of the zero line in the drawing
	 *               area [pixels]
	 * @param height the height of the drawing are [pixels]
	 */
	void drawPolyLineSignal(QPainter &p, int width, int middle, int height);

	/**
	 * Converts a pixel offset into a sample offset.
	 */
	inline sample_index_t pixels2samples(int pixels) {
	    return static_cast<sample_index_t>(rint(
		static_cast<double>(pixels) * m_zoom));
	}

	/**
	 * Converts a sample offset into a pixel offset.
	 */
	inline int samples2pixels(sample_index_t samples) {
	    if (m_zoom <= 0) return 0;
	    return Kwave::toInt(rint(static_cast<double>(samples) / m_zoom));
	}

	/**
	 * Converts the offset and length of an overlapping region into buffer
	 * indices, depending on the current mode. If the given region does
	 * not overlap at all, the length of the area will be set to zero.
	 * The length will be truncated to the end of the current buffer(s).
	 * @note If the resulting or given length is zero, the offset value
	 *       is not necessarily valid and should be ignored!
	 * @param offset reference to the source sample index, will be converted
	 *               into buffer index
	 * @param length reference to the length in samples, will be converted
	 *               to the number of buffer indices
	 */
	void convertOverlap(sample_index_t &offset, sample_index_t &length);

    private:

	/** The QPixmap with the bitmap data */
	QPixmap m_pixmap;

	/**
	 * Reference to the track with our sample data.
	 */
	Kwave::Track &m_track;

	/**
	 * Index of the first sample. Needed for converting pixel
	 * positions into absolute sample numbers. This is always
	 * in units of samples, independent of the current mode!
	 */
	sample_index_t m_offset;

	/**
	 * Zoom factor in samples/pixel. Needed for converting
	 * sample indices into pixels and vice-versa.
	 */
	double m_zoom;

	/**
	 * vertical zoom factor. Default is 1.0
	 */
	double m_vertical_zoom;

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
	Kwave::SampleArray m_sample_buffer;

	/**
	 * Array with minimum sample values, if in min/max mode.
	 */
	Kwave::SampleArray m_min_buffer;

	/**
	 * Array with maximum sample values, if in min/max mode.
	 */
	Kwave::SampleArray m_max_buffer;

	/** Indicates that the buffer content was modified */
	bool m_modified;

	/**
	 * Array with one bit for each position in the internal
	 * buffers. If the bit corresponding to a certain buffer
	 * index is set to "1", the position contains valid data,
	 * if "0" the content of the buffer is invalid.
	 */
	QBitArray m_valid;

	/** Mutex for exclusive access to the buffers. */
	QMutex m_lock_buffer;

	/**
	 * order of the low pass filter used for interpolation
	 */
	int m_interpolation_order;

	/**
	 * Buffer for filter coefficients of the low pass used for
	 * interpolation.
	 * @see #calculateInterpolation()
	 */
	QVector<double> m_interpolation_alpha;

	/** set of colors for drawing */
	Kwave::Colors::ColorSet m_colors;

    };
}

#endif /* TRACK_PIXMAP_H */

//***************************************************************************
//***************************************************************************
