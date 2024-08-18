/***************************************************************************
 *  SelectionBorderItem.cpp  -  selection border within a SignalView
 *                             -------------------
 *    begin                : Sat Mar 11 2017
 *    copyright            : (C) 2017 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/SelectionBorderItem.h"
#include "libgui/SignalView.h"

//***************************************************************************
Kwave::SelectionBorderItem::SelectionBorderItem(
    Kwave::SignalView &view,
    Kwave::SignalManager &signal_manager,
    sample_index_t pos
)
    :Kwave::ViewItem(view, signal_manager),
     m_selection()
{
    m_selection.set(
        signal_manager.selection().first(),
        signal_manager.selection().last()
    );
    m_selection.grep(pos);
}

//***************************************************************************
Kwave::SelectionBorderItem::~SelectionBorderItem()
{
}

//***************************************************************************
Kwave::ViewItem::Flags Kwave::SelectionBorderItem::flags() const
{
    return Kwave::ViewItem::CanGrabAndMove;
}

//***************************************************************************
QString Kwave::SelectionBorderItem::toolTip(sample_index_t &ofs)
{
    sample_index_t first = m_selection.left();
    sample_index_t last  = m_selection.right();
    if (first == last) return QString(); // empty selection

    sample_index_t d_left  = (first < ofs) ? (ofs - first) : (first - ofs);
    sample_index_t d_right = (last  < ofs) ? (ofs - last) :  (last  - ofs);

    QString which;
    if (d_left <= d_right) {
        which = i18n("Selection, left border");
        ofs   = first;
    } else {
        which = i18n("Selection, right border");
        ofs   = last;
    }

    QString hms = Kwave::ms2hms(m_view.samples2ms(ofs));
    return _("%1\n%2\n%3").arg(which).arg(ofs).arg(hms);
}

//***************************************************************************
QCursor Kwave::SelectionBorderItem::mouseCursor() const
{
    return Qt::SizeHorCursor;
}

//***************************************************************************
void Kwave::SelectionBorderItem::moveTo(const QPoint &mouse_pos)
{
    const sample_index_t new_pos = m_view.offset() +
                                   m_view.pixels2samples(mouse_pos.x());
    m_selection.update(new_pos);
    m_signal_manager.selectRange(m_selection.left(), m_selection.length());
}

//***************************************************************************
//***************************************************************************
