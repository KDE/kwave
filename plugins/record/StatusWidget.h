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

#ifndef STATUS_WIDGET_H
#define STATUS_WIDGET_H

#include "config.h"

#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QWidget>

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
        explicit StatusWidget(QWidget *parent = Q_NULLPTR);

        /** Destructor */
        virtual ~StatusWidget() Q_DECL_OVERRIDE;

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
        virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

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

#endif /* STATUS_WIDGET_H */

//***************************************************************************
//***************************************************************************
