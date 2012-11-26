/***************************************************************************
          ScaleWidget.h  -  widget for drawing a scale under an image
			     -------------------
    begin                : Sep 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SCALE_WIDGET_H_
#define _SCALE_WIDGET_H_

#include "config.h"

#include <QtCore/QString>
#include <QtGui/QWidget>

#include <kdemacros.h>

class QPaintEvent;
class QPainter;
class QSize;

namespace Kwave
{
    class KDE_EXPORT ScaleWidget : public QWidget
    {
    public:

	/**
	 * Primitve constructor for usage in a Qt designer's dialog
	 * @param parent the widget's parent widget
	 */
	ScaleWidget(QWidget *parent);

	/**
	 * Constructor with initialization.
	 * @param parent the widget's parent widget
	 * @param low left/lower border value
	 * @param high right/upper border value
	 * @param unit text of the units to show
	 */
	ScaleWidget(QWidget *parent, int low, int high, const QString &unit);

	/** Destructor */
	virtual ~ScaleWidget();

	/**
	 * Sets the border values.
	 * @param min left/lower border value
	 * @param max right/upper border value
	 */
	void setMinMax(int min, int max);

	/**
	 * Set the text of the units.
	 * @param text the units to show
	 */
	void setUnit(const QString &text);

	/**
	 * Sets logarithmic or linear mode.
	 * @param log if true, set logarithmic mode, if not select
	 *        linear mode
	 */
	void setLogMode(bool log);

	/** minimum size of the widtget, @see QWidget::minimumSize() */
	virtual QSize minimumSize() const;

	/** optimal size for the widget, @see QWidget::sizeHint() */
	virtual QSize sizeHint() const;

    protected:

	/**
	 * Draws the widget.
	 * @see QWidget::paintEvent
	 */
	void paintEvent(QPaintEvent *);

	/**
	 * Draws a linear scale
	 * @param p reference to the painter
	 * @param w width of the drawing area in pixels
	 * @param h height of the drawing area in pixels
	 * @param inverse of true, the coordinate system is rotated
	 *        to be upside-down and the scale has to be drawn
	 *        mirrored in x and y axis.
	 */
	void drawLinear(QPainter &p, int w, int h, bool inverse);

	/**
	 * @todo implementation of logarithmic scale
	 */
	void drawLog(QPainter &p, int w, int h, bool inverse);

	/**
	 * Painting routine for own small font with fixed size
	 * There are Problems with smaller displays using QFont,
	 * sizes are not correct.
	 * @param p reference to the painter
	 * @param x coordinate of the left edge of the first character
	 * @param y coordinate of the lower edge of the first character
	 * @param reverse if true, print reverse: x is right edge of
	 *        the text, like "align right".
	 * @param text the text to be printed. Must only contain known
	 *        characters that are present in the font bitmap, like
	 *        numbers, letters and some special chars like "%",
	 *        space, dot and comma.
	 */
	void paintText(QPainter &p, int x, int y,
	                bool reverse, const QString &text);

    private:

	/** Lower boundary value */
	int m_low;

	/** Upper boundary value */
	int m_high;

	/** If true, logarithmic mode, linear mode if false */
	bool m_logmode;

	/** String containing the name of the unit */
	QString m_unittext;

    };
}

#endif // SCALE_WIDGET_H

//***************************************************************************
//***************************************************************************
