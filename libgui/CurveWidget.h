/***************************************************************************
          CurveWidget.h  -  widget for editing an interpolated curve
			     -------------------
    begin                : Sep 16 2001
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

#ifndef CURVE_WIDGET_H
#define CURVE_WIDGET_H

#include "config.h"

#include <QtGui/QMenu>
#include <QtGui/QWidget>

#include <kdemacros.h>

#include "libkwave/Curve.h"

class QAction;
class QMouseEvent;
class QPaintEvent;
class QPixmap;
class QString;

namespace Kwave
{
    class Q_DECL_EXPORT CurveWidget: public QWidget
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit CurveWidget(QWidget *parent);

	/** Destructor */
	virtual ~CurveWidget();

	/** Returns a command string for the curve. */
	QString getCommand();

	/** Sets the curve parameters/points from a command string. */
	void setCurve(const QString &command);

	/**
	 * Adds a new point to the curve.
	 * @param x the point's x coordinate, should be [0.0...1.0]
	 * @param y the point's y coordinate, should be [0.0...1.0]
	 */
	void addPoint(double x, double y);

	/**
	 * Tries to find a point that is nearest to the given widget coordinates
	 * and within a tolerance.
	 * @param sx screen x coordinate, left is 0
	 * @param sy screen y coordinate, top is 0
	 * @return the point of the curve or Curve::NoPoint if nothing found
	 */
	Kwave::Curve::Point findPoint(int sx, int sy);

    public slots:

	/**
	 * Selects a new interpolation type by it's numeric index. Used from the
	 * mouse context menu.
	 */
	void selectInterpolationType(QAction *action);

	/**
	 * Scales the size of the curve so that all interpolated points are
	 * between 0.0 and 1.0 in x and y direction.
	 */
	void scaleFit();

	/** Mirrors the curve on the y axis */
	void VFlip();

	/** Mirrors the curve on the y axis */
	void HFlip();

	/** Deletes the last point of the curve. */
	void deleteLast();

	/** Deletes the every second (even) point of the curve. */
	void deleteSecond();

	/**
	 * Scales the x coordinates of all points to 50% so that the curve's
	 * points move into the first half of the curve. A new "last" point
	 * with the y value of the previous last point will be inserted at
	 * x coordinate 1.0.
	 */
	void firstHalf();

	/** Like firstHalf(), but moves points to the right half. */
	void secondHalf();

	void savePreset();

	/**
	 * Loads an existing preset.
	 * @param action the menu actio of the corresponding menu entry
	 */
	void loadPreset(QAction *action);

    protected slots:

	void mousePressEvent(QMouseEvent * );
	void mouseReleaseEvent(QMouseEvent * );
	void mouseMoveEvent(QMouseEvent * );
	void paintEvent(QPaintEvent *);

    protected:

	/**
	 * (Re-)Loads the list of preset files and fills the popup
	 * menu with all preset files.
	 */
	void loadPresetList();

    private:

	/** Cached width of the widget */
	int m_width;

	/** Cached height of the widget */
	int m_height;

	/** The curve to be edited */
	Kwave::Curve m_curve;

	/** Popup (context) menu for the right mouse button */
	QMenu *m_menu;

	/**
	 * Part of the popup (context) menu for the right
	 * mouse button with the list of preset files
	 */
	QMenu *m_preset_menu;

	/** Currently selected point or null if none selected */
	Kwave::Curve::Point m_current;

	/** Last selected point, remembered for deleting. */
	Kwave::Curve::Point m_last;

	/** State of the left mouse button (when moving points) */
	bool m_down;

	/** pixmap for the unselected knob */
	QPixmap m_knob;

	/** pixmap for the selected knob */
	QPixmap m_selected_knob;

    };
}

#endif // _CURVE_WIDGET_H_

//***************************************************************************
//***************************************************************************
