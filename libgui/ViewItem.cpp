/***************************************************************************
           ViewItem.cpp  -  base class for a visible item within a SignalView
                             -------------------
    begin                : Sat Mar 26 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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



#include <KLocalizedString>

#include "libgui/LabelItem.h"
#include "libgui/SignalView.h"

//***************************************************************************
Kwave::ViewItem::ViewItem(Kwave::SignalView &view,
                          Kwave::SignalManager &signal_manager)
    :QObject(&view), m_view(view), m_signal_manager(signal_manager)
{
}

//***************************************************************************
Kwave::ViewItem::~ViewItem()
{
}

//***************************************************************************
Kwave::ViewItem::Flags Kwave::ViewItem::flags() const
{
    return Kwave::ViewItem::None;
}

//***************************************************************************
QString Kwave::ViewItem::toolTip(sample_index_t &ofs)
{
    Q_UNUSED(ofs)
    return QString();
}

//***************************************************************************
void Kwave::ViewItem::appendContextMenu(QMenu *parent)
{
    Q_UNUSED(parent)
}

//***************************************************************************
QCursor Kwave::ViewItem::mouseCursor() const
{
    return Qt::ArrowCursor;
}

//***************************************************************************
void Kwave::ViewItem::moveTo(const QPoint &mouse_pos)
{
    Q_UNUSED(mouse_pos)
}

//***************************************************************************
void Kwave::ViewItem::startDragging()
{
}

//***************************************************************************
void Kwave::ViewItem::done()
{
}

//***************************************************************************
//***************************************************************************

#include "moc_ViewItem.cpp"
