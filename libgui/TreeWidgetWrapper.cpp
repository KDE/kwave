/***************************************************************************
    TreeWidget.cpp  -  wrapper for QTreeWidget to get focus out information
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

#include "config.h"
#include "libgui/TreeWidgetWrapper.h"

//***************************************************************************
Kwave::TreeWidgetWrapper::TreeWidgetWrapper(QWidget *widget)
    :QTreeWidget(widget)
{
}

//***************************************************************************
Kwave::TreeWidgetWrapper::~TreeWidgetWrapper()
{
}

//***************************************************************************
void Kwave::TreeWidgetWrapper::focusOutEvent(QFocusEvent *event)
{
    QTreeWidget::focusOutEvent(event);
    emit focusLost();
}

//***************************************************************************
#include "TreeWidgetWrapper.moc"
//***************************************************************************
//***************************************************************************
