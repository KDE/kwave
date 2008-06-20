/***************************************************************************
    TreeWidget.h  -  wrapper for QTreeWidget to get focus out information
			     -------------------
    begin                : Mon May 12 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#ifndef _TREE_WIDGET_WRAPPER_H_
#define _TREE_WIDGET_WRAPPER_H_

#include "config.h"

#include <kdemacros.h>

#include <QObject>
#include <QTreeWidget>

class QFocusEvent;

class KDE_EXPORT TreeWidgetWrapper: public QTreeWidget
{
    Q_OBJECT
public:
    /** Constructor */
    TreeWidgetWrapper(QWidget *parent);

    /** Destructor */
    virtual ~TreeWidgetWrapper();

    /** catches the "lost focus" event */
    virtual void focusOutEvent(QFocusEvent *event);

signals:

    /** emitted when the focus has been lost */
    void focusLost();

};

#endif /* _TREE_WIDGET_WRAPPER_H_ */
