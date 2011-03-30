/***************************************************************************
           LabelItem.cpp -  label item within a SignalView
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

#include <math.h>

#include <QAction>
#include <QList>
#include <QMenu>

#include "klocalizedstring.h"

#include "libkwave/Label.h"
#include "libkwave/Utils.h"

#include "libgui/LabelItem.h"

//***************************************************************************
Kwave::LabelItem::LabelItem(unsigned int index, double ms, const Label &label)
    :Kwave::ViewItem(),
     m_index(index), m_pos(label.pos()), m_ms(ms), 
     m_description(label.name())
{
}

//***************************************************************************
Kwave::LabelItem::~LabelItem()
{
}

//***************************************************************************
Qt::ItemFlags Kwave::LabelItem::flags()
{
    return Qt::ItemIsEditable;
}

//***************************************************************************
sample_index_t Kwave::LabelItem::first()
{
    return m_pos;
}

//***************************************************************************
sample_index_t Kwave::LabelItem::last()
{
    return m_pos;
}

//***************************************************************************
QString Kwave::LabelItem::toolTip(sample_index_t ofs)
{
    Q_UNUSED(ofs);

    QString description = (m_description.length()) ?
	i18nc("tooltip of a label, %1=index, %2=description/name",
		"Label #%1 (%2)", m_index, m_description) :
	i18nc("tooltip of a label, without description, %1=index",
		"Label #%1", m_index);

    QString hms  = Kwave::ms2hms(m_ms);
    QString text = QString("%1\n%2\n%3").arg(description).arg(m_pos).arg(hms);

    return text;
}

//***************************************************************************
void Kwave::LabelItem::appendContextMenu(QMenu *parent)
{
    Q_ASSERT(parent);
    if (!parent) return;
    
    // locate the "label" menu
    QAction *label_menu = 0;
    QList<QAction *> actions = parent->actions();
    foreach (QAction *action, actions) {
	if (action->text() == i18n("&Label")) {
	    qDebug("----- BINGO :-) -----");
	    label_menu = action;
	    break;
	}
    }
}

//***************************************************************************
#include "LabelItem.moc"
//***************************************************************************
//***************************************************************************
