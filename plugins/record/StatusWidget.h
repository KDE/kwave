/*************************************************************************
         StatusWidget.h  -  little widget with status icons
                             -------------------
    begin                : Sat Apr 15 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef _STATUS_WIDGET_H_
#define _STATUS_WIDGET_H_

#include "config.h"

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtCore/QVector>
#include <QtCore/QTimer>

namespace Kwave
{
    /**
    * Widget needed to display a little icon in the status bar
    */
    class StatusWidget: public QWidget
    {
	Q_OBJECT
    public:

	/** Constructor */
	StatusWidget(QWidget *parent = 0);

	/** Destructor */
	virtual ~StatusWidget();

	/**
	 * Set a new list of pixmaps
	 *
	 * @param pixmaps a list of pixmaps (can also have only one element)
	 * @param speed if multiple pixmaps are given, the time in milliseconds
	 *              between the pixmaps, for animation
	 */
	void setPixmaps(const QVector<QPixmap> &pixmaps,
	                unsigned int speed = 150);

    protected:
	/** repaint, see QWidget::paintEvent */
	void paintEvent(QPaintEvent *);

    private slots:

	/** switch to the next pixmap in the pixmaps list */
	void nextPixmap();

    private:

	/** the pixmap to show */
	QVector<QPixmap> m_pixmaps;

	/** index of the current pixmap */
	unsigned int m_index;

	/** timer for switching to the next pixmap */
	QTimer m_timer;

    };
}

#endif /* _STATUS_WIDGET_H_ */

//***************************************************************************
//***************************************************************************
