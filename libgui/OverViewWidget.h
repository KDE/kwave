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

#include <QBitmap>
#include <QSize>
#include <QTimer>
#include <QWidget>

#include <kdemacros.h>

#include "OverViewCache.h"

class QMouseEvent;
class QResizeEvent;
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
class KDE_EXPORT OverViewWidget : public QWidget
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
    virtual QSize minimumSize() const;

    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

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
     * @param new_pos position of the slider (left or top of the bar)
     * @param new_width width of the slider (bar)
     * @param new_length size of the slider's area
     */
    void setRange(unsigned int new_pos, unsigned int new_width,
	unsigned int new_length);

protected slots:

    /** moves the slider one further bit left or right */
    void increase();

    /** Refreshes all modified parts of the bitmap */
    void refreshBitmap();

    /**
     * connected to the m_repaint_timer, called when it has
     * elapsed and the signal has to be repainted
     */
    void overviewChanged();

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
    typedef enum {Invalid, Fuzzy, Intermediate, Unused} CacheState;

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
    QBitmap m_bitmap;

    /** cache with overview data */
    OverViewCache m_cache;

    /** timer for limiting the number of repaints per second */
    QTimer m_repaint_timer;

};

#endif // _OVER_VIEW_WIDGET_H_
