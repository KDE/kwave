/***************************************************************************
       OverViewWidget.h  -  horizontal slider with overview over a signal
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

#ifndef _OVER_VIEW_WIDGET_H_
#define _OVER_VIEW_WIDGET_H_

#include "config.h"
#include <qbitarray.h>
#include <qcstring.h>
#include <qlist.h>
#include <qtimer.h>
#include <qwidget.h>

#include "mt/Mutex.h"

class QBitmap;
class QPixmap;
class SignalManager;
class Track;

/**
 * @class OverViewWidget
 * An OverviewWidget can be used like a QSlider but has some background image
 * that can show an overview over the whole data that is currently not
 * visible. (QScrollbar has proven to be unstable with high numbers)
 *
 * The behaviour is also somewhat "smarter":
 *
 * The slider can be moved like in the QScrollbar by pressing down the left
 * mouse button and moving the mouse left or right.
 *
 * When pressing the left mouse button left or right of the slider, the slider
 * moves towards the mouse position in steps of approx 1/10 of the current view
 * width - with a speed of 10 steps per second - until the slider goes under
 * the mouse. Then the slider snaps in and will be centered under the mouse
 * cursor and can be moved.
 *
 * By double-clicking on the bar, the slider immediately will be centered
 * under the clicked position.
 */
class OverViewWidget : public QWidget
{
    Q_OBJECT
public:
    OverViewWidget(SignalManager &signal, QWidget *parent = 0);

    /** Destructor */
    virtual ~OverViewWidget();

    /**
     * Sets the slider to a new offset. The offset is given in
     * user's coordinates (e.g. samples).
     */
    void setValue(unsigned int newval);

    /** minimum size of the widget, @see QWidget::minimumSize() */
    virtual const QSize minimumSize();

    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual const QSize sizeHint();

protected:

    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *e);

public slots:

    /**
     * Sets new range parameters of the slider, using a scale that is calculated
     * out of the slider's maximum position. All parameters are given in the
     * user's coordinates/units (e.g. samples).
     * @param new_val position of the slider (left or top of the bar)
     * @param new_width width of the slider (bar)
     * @param new_length size of the slider's area
     */
    void setRange(unsigned int new_pos, unsigned int new_width,
	unsigned int new_length);

protected slots:

    /** moves the slider one further bit left or right */
    void increase();

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &);

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::sigTrackDeleted
     * @internal
     */
    void slotTrackDeleted(unsigned int index);

    /**
     * Connected to the signal's sigSamplesInserted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Signal::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(unsigned int track, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, unsigned int offset,
                             unsigned int length);

signals:

    /**
     * Will be emitted if the slider position has changed. The value
     * is in user's units (e.g. samples).
     */
    void valueChanged(unsigned int new_value);

protected:

    /** repaints the bar with overview and the slider */
    void paintEvent(QPaintEvent *);

    /**
     * Converts an offset in the user's coordinate system into
     * a pixel offset withing the overview's drawing area.
     * @param offset an offset in the user's coordinate system [0..length-1]
     * @return the internal pixel coordinate [0...width-1]
     */
    int offset2pixels(unsigned int offset);

    /**
     * Converts a pixel offset within the overview's drawing area
     * into the user's coordinate system.
     * @param pixels the pixel coordinate [0...width-1]
     * @return an offset [0..length-1]
     */
    unsigned int pixels2offset(int pixels);

    /** State of a cache entry */
    typedef enum {Invalid, Fuzzy, Valid, Unused} CacheState;

private:

    /**
     * Compresses the cache to hold more samples per entry.
     */
    void scaleUp();

    /**
     * Expands the cache to hold less samples per entry. As this
     * process looses accuracy, the cache must be "polished" in
     * a second step.
     */
    void scaleDown();

    /**
     * Marks a range of cache entries of a track as invalid
     * @param track index of the track to invalidate
     * @param first index of the first entry
     * @param last index of the last entry (will be truncated to CACHE_SIZE-1)
     */
    void invalidateCache(unsigned int track, unsigned int first,
                         unsigned int last);

    /**
     * Refreshes all modified parts of the bitmap
     */
    void refreshBitmap();

private:

    /** width of the widget in pixels */
    int m_width;

    /** height of the widget in pixels */
    int m_height;

    /** position of the cursor within the slider when clicked */
    int m_grabbed;

    /** last position of the mouse pointer when held down */
    int m_mouse_pos;

    /** width of the slider [pixels] */
    int m_slider_width;

    /** width of the visible area [user's coordinates] */
    unsigned int m_view_width;

    /** length of the whole area [user's coordinates] */
    unsigned int m_view_length;

    /** offset of the visible area [user's coordinates] */
    unsigned int m_view_offset;

    /** addup for direction... */
    int m_dir;

    /** flag for redrawing pixmap */
    bool m_redraw;

    /** to spare user repeated pressing of the widget... */
    QTimer m_timer;

    /** QBitmap with the overview */
    QBitmap *m_bitmap;

    /** QPixmap to be blitted to screen (avoiding flicker) */
    QPixmap *m_pixmap;

    /** signal with the data to be shown */
    SignalManager &m_signal;

    /** list of minimum value arrays, one array per track */
    QList<QByteArray> m_min;

    /** list of maximum value arrays, one array per track */
    QList<QByteArray> m_max;

    /** bitmask for "validity" of the min/max values */
    QList< QArray <CacheState> > m_state;

    /** number of min/max values */
    unsigned int m_count;

    /** number of samples per cache entry */
    unsigned int m_scale;

    /** mutex for threadsafe access to the cache */
    Mutex m_lock;

};

#endif // _OVER_VIEW_WIDGET_H_
