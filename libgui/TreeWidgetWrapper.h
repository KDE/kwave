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

#ifndef TREE_WIDGET_WRAPPER_H
#define TREE_WIDGET_WRAPPER_H

#include "config.h"

#include <QtGlobal>
#include <QObject>
#include <QTreeWidget>

class QFocusEvent;

namespace Kwave
{
    class Q_DECL_EXPORT TreeWidgetWrapper: public QTreeWidget
    {
	Q_OBJECT
    public:
	/** Constructor */
	explicit TreeWidgetWrapper(QWidget *parent);

	/** Destructor */
        virtual ~TreeWidgetWrapper() Q_DECL_OVERRIDE;

	/** catches the "lost focus" event */
        virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

    signals:

	/** emitted when the focus has been lost */
	void focusLost();

    };
}

#endif /* TREE_WIDGET_WRAPPER_H */

//***************************************************************************
//***************************************************************************
