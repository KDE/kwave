/***************************************************************************
 *      SelectionItem.cpp  -  selection item within a SignalView
 *                             -------------------
 *    begin                : Sun Mar 12 2017
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

#include "config.h"

#include <QAction>
#include <QList>

#include "libkwave/Drag.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectionItem.h"
#include "libgui/SignalView.h"

//***************************************************************************
Kwave::SelectionItem::SelectionItem(
    Kwave::SignalView &view,
    Kwave::SignalManager &signal_manager
)
    :Kwave::ViewItem(view, signal_manager),
     m_first(signal_manager.selection().first()),
     m_last(signal_manager.selection().last())
{
}

//***************************************************************************
Kwave::SelectionItem::~SelectionItem()
{
}

//***************************************************************************
Kwave::ViewItem::Flags Kwave::SelectionItem::flags() const
{
    return Kwave::ViewItem::CanDragAndDrop;
}

//***************************************************************************
void Kwave::SelectionItem::startDragging()
{
    const sample_index_t length = m_signal_manager.selection().length();
    if (!length) return;

    Kwave::Drag *d = new(std::nothrow) Kwave::Drag(&m_view);
    Q_ASSERT(d);
    if (!d) return;

    const sample_index_t first = m_signal_manager.selection().first();
    const sample_index_t last  = m_signal_manager.selection().last();
    const double         rate  = m_signal_manager.rate();
    const unsigned int   bits  = m_signal_manager.bits();

    Kwave::MultiTrackReader src(Kwave::SinglePassForward, m_signal_manager,
                                m_signal_manager.selectedTracks(), first, last);

    // create the file info
    Kwave::MetaDataList meta = m_signal_manager.metaData();
    Kwave::FileInfo info(meta);
    info.setLength(last - first + 1);
    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(src.tracks());
    meta.replace(Kwave::MetaDataList(info));

    if (!d->encode(&m_view, src, meta)) {
        delete d;
        return;
    }

    // start drag&drop, mode is determined automatically
    Kwave::UndoTransactionGuard undo(m_signal_manager, i18n("Drag and Drop"));
    Qt::DropAction drop = d->exec(Qt::CopyAction | Qt::MoveAction);

    if (drop == Qt::MoveAction) {
        // deleting also affects the selection !
        const sample_index_t f = m_signal_manager.selection().first();
        const sample_index_t l = m_signal_manager.selection().last();
        const sample_index_t len = l - f + 1;

        // special case: when dropping into the same widget, before
        // the previous selection, the previous range has already
        // been moved to the right !
        sample_index_t src_pos = first;

        SignalView *target = qobject_cast<Kwave::SignalView *>(d->target());
        if ( (f < src_pos) && target &&
              (target->signalManager() == m_view.signalManager()) ) {
            src_pos += len;
        }

        m_signal_manager.deleteRange(src_pos, len,
                                     m_signal_manager.selectedTracks());

        // restore the new selection
        m_signal_manager.selectRange((first < f) ? (f - len) : f, len);
    }
}

//***************************************************************************
//***************************************************************************
