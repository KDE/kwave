/***************************************************************************
            ImageView.h  -  simple widget class for displaying a QImage
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _IMAGEVIEW_H_
#define _IMAGEVIEW_H_

#include "config.h"

#include <QWidget>
#include <QPainter>
#include <QImage>

/**
 * Simple widget class for displaying a QImage.
 */
class ImageView : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent pointer to the parent widget, can be 0
     * @param fit_width if set to true, the image will be scaled to fit
     *        horizontally into the widget; if false the image will be
     *        cut off at the edge and should be scrolled
     * @param fit_height if set to true, the image will be scaled to fit
     *        vertically into the widget; if false the image will be
     *        cut off at the edge and should be scrolled
     */
    ImageView(QWidget *parent = 0, bool fit_width=true, bool fit_height=true);

    /**
     * Destructor
     */
    virtual ~ImageView();

    /**
     * Sets a new QImage for displaying.
     * @param image a pointer to the image
     */
    void setImage(QImage image);

    /**
     * Returns the position and size of the current image, packed
     * into a QRect object.
     * @return
     */
    QRect imageRect();

public slots:

    /** sets a new horizontal offset, useful for scrolling */
    void setHorizOffset(int offset);

    /** sets a new vertical offset, useful for scrolling */
    void setVertOffset(int offset);

signals:

    void viewInfo(int, int, int);
    void sigCursorPos(const QPoint pos);

protected:

    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    /**
     * offset of the image, in original unscaled coordinates
     * of the internal QImage
     */
    QPoint m_offset;

    /**
     * last displayed image rectangle. Note that the left and
     * top coordinates are unscaled, but the width and height
     * might be scaled to screen coordinates!
     */
    QRect m_last_rect;

    /** pointer to the QImage to be displayed */
    QImage m_image;

    /** if true, scale to fit horizontally */
    bool m_fit_width;

    /** if true, scale to fit vertically */
    bool m_fit_height;

    /**
     * scale factor in horizontal direction, will be
     * (width of the image / width of the widget) if m_fit_width
     * is true, or 1.0 else
     */
    double m_scale_x;

    /**
     * scale factor in vertical direction, will be
     * (height of the image / height of the widget) if m_fit_height
     * is true, or 1.0 else
     */
    double m_scale_y;

};
//***********************************************************************
#endif // _IMAGEVIEW_H_
