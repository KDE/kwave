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
#define _OVER_VIEW_WIDGET_H_ 1

#include <qtimer.h>
#include <qwidget.h>

class QBitmap;
class QPixmap;

/**
 * \class OverViewWidget
 * OverviewWidget can be used like a QSlider but has some background image
 * that can show an overview over the whole data that is currently not
 * visible. (QScrollbar has proven to be unstable with high numbers)
 */
class OverViewWidget : public QWidget
{
    Q_OBJECT
public:
    OverViewWidget(QWidget *parent = 0, const char *name = 0);
    virtual ~OverViewWidget();

    /** sets the slider to a new offset */
    void setValue(int);

    /** refreshes the overview */
    void setOverView(QBitmap *overview);

    /** minimum size of the widtget, @see QWidget::minimumSize() */
    virtual const QSize minimumSize();

    /** optimal size for the widget, @see QWidget::sizeHint() */
    virtual const QSize sizeHint();

protected:

    void resizeEvent(QResizeEvent *e);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

public slots:

    /*
     * Sets new range parameters of the slider, using a scale that is calculated
     * out of the slider's maximum position.
     * @param new_val position of the slider (left or top of the bar)
     * @param new_width width of the slider (bar)
     * @param new_length size of the slider's area
     */
    void setRange(int new_pos, int new_width, int new_length);

    /** moves the slider one further bit left or right */
    void increase();

signals:

    /** will be emitted if the slider position has changed */
    void valueChanged(int new_value);

protected:

    /** repaints the bar with overview and the slider */
    void paintEvent(QPaintEvent *);

    /**
     * Converts an offset in the user's coordinate system into
     * a pixel offset withing the overview's drawing area.
     * @param offset an offset in the user's coordinate system [0..length-1]
     * @return the internal pixel coordinate [0...width-1]
     */
    int offset2pixels(int offset);

    /**
     * Converts a pixel offset within the overview's drawing area
     * into the user's coordinate system.
     * @param pixels the pixel coordinate [0...width-1]
     * @return an offset [0..length-1]
     */
    int pixels2offset(int pixels);

private:

    /** width of the widget in pixels */
    int m_width;

    /** height of the widget in pixels */
    int m_height;

    /** position of the cursor within the slider when clicked */
    int m_grabbed;

    /** width of the slider in pixels */
    int m_slider_width;

    /** width of the visible area [user's coordinates] */
    int m_view_width;

    /** length of the whole area [user's coordinates] */
    int m_view_length;

    /** offset of the visible area [user's coordinates] */
    int m_view_offset;

    /** addup for direction... */
    int m_dir;

    /** flag for redrawing pixmap */
    bool m_redraw;

    /** to spare user repeated pressing of the widget... */
    QTimer m_timer;

    /** QBitmap with the overview */
    QBitmap *m_overview;

    /** QPixmap to be blitted to screen (avoiding flicker) */
    QPixmap *m_pixmap;
}
;

#endif // _OVER_VIEW_WIDGET_H_
