/***************************************************************************
 *        SelectionItem.h  -  selection item within a SignalView
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

#ifndef SELECTION_ITEM_H
#define SELECTION_ITEM_H

#include "config.h"

#include <QCursor>

#include "libkwave/Sample.h"
#include "libgui/ViewItem.h"

class QWidget;

namespace Kwave
{
    // forward declarations
    class SignalManager;
    class SignalView;

    class SelectionItem: public Kwave::ViewItem
    {
    public:
        /**
         * Constructor
         * @param view the parent signal view
         * @param signal_manager the signal manager
         */
        SelectionItem(SignalView &view,
                      Kwave::SignalManager &signal_manager);

        /** Destructor */
        virtual ~SelectionItem() Q_DECL_OVERRIDE;

        /**
         * Returns flags describing the possible interactions with this object
         * @see Kwave::ViewItem::Flags
         */
        virtual Kwave::ViewItem::Flags flags() const Q_DECL_OVERRIDE;

        /** Starts a drag & drop operation. */
        virtual void startDragging() Q_DECL_OVERRIDE;

    private:

        /** start of the selection */
        sample_index_t m_first;

        /** end of the selection */
        sample_index_t m_last;

    };
}

#endif /* SELECTION_ITEM_H */
