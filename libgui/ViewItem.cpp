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

#include "config.h"

#include <KLocalizedString>

#include "libgui/LabelItem.h"

//***************************************************************************
Kwave::ViewItem::ViewItem()
    :QObject()
{
}

//***************************************************************************
Kwave::ViewItem::~ViewItem()
{
}

//***************************************************************************
Qt::ItemFlags Kwave::ViewItem::flags()
{
    return Qt::NoItemFlags;
}

//***************************************************************************
QString Kwave::ViewItem::toolTip(sample_index_t ofs)
{
    Q_UNUSED(ofs);
    return QString();
}

//***************************************************************************
void Kwave::ViewItem::appendContextMenu(QMenu *parent)
{
    Q_UNUSED(parent);
}

//***************************************************************************
//***************************************************************************
