/***************************************************************************
  MultiStateWidget.h  -  provides methods of multistateWidget a Class that
                         switches the image it, displays on clicking, used
                         for the channel enable/disable lamps...
                             -------------------
    begin                : Sun Jun 04 2000
    copyright            : (C) 2000 by Martin Wilz
    email                : martin@wilz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MULTI_STATE_WIDGET_H
#define MULTI_STATE_WIDGET_H

#include "config.h"
#include "libkwavegui_export.h"

#include <QtGlobal>
#include <QPixmap>
#include <QVector>
#include <QWidget>

class QPaintEvent;
class QMouseEvent;
class QString;

namespace Kwave
{

    class LIBKWAVEGUI_EXPORT MultiStateWidget: public QWidget
    {
        Q_OBJECT

    public:

        /**
         * Constructor
         * @param parent the parent widget
         * @param id identifier
         */
        MultiStateWidget(QWidget *parent, int id);

        /** Destructor */
        virtual ~MultiStateWidget() override;

        /**
         * Sets the number that will passed as argument to the
         * "clicked" signal.
         * @param id new identifier
         */
        void setID(int id);

        /**
         * Adds a the content of pixmap file as pixmap for the
         * next state. The file is found through the KStandardDirs
         * mechanism. Adding a file for a second or further time
         * is not possible, in this case the pixmap will not be
         * loaded and the return value will be the id of the
         * existing version.
         * @see KStandardDirs
         * @param filename name of the file to be added, without
         *        path.
         */
        void addPixmap(const QString &filename);

    public slots:

        /**
         * Activates a new state, with wrap-around on overflows, limited
         * to [ 0 ... m_pixmaps.count()-1 ].
         * @param newstate index of the new state [0...N]
         */
        void setState(int newstate);

        /**
         * For widgets that have only two states (on and off),
         * this selects state 1 or 0
         * @param on if true, switch on (state 1), otherwise
         *           switch off (state 0)
         */
        void switchState(bool on);

        /** advance to the next state, with wrap-around to zero */
        void nextState();

    signals:

        /**
         * Signals that the widget has changed it's state.
         * @param id identifier of this widget's instance
         */
        void clicked(int id);

    private:

        /** reacts to the mouse release (click) */
        virtual void mouseReleaseEvent(QMouseEvent *) override;

        /** repaints the pixmap */
        virtual void paintEvent(QPaintEvent *) override;

    private:

        /** index of the current state */
        int m_current_index;

        /** identifier used for the clicked() signal */
        int m_identifier;

        /** list of QPixmaps */
        QVector<QPixmap> m_pixmaps;
    };
}

#endif  // _MULTI_STATE_WIDGET_H_

//***************************************************************************
//***************************************************************************
