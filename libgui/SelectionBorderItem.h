/***************************************************************************
 *  SelectionBorderItem.h  -  selection border within a SignalView
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

#ifndef SELECTION_BORDER_ITEM_H
#define SELECTION_BORDER_ITEM_H

#include "config.h"

#include <QCursor>
#include <QString>

#include "libkwave/Sample.h"

#include "libgui/MouseMark.h"
#include "libgui/ViewItem.h"


namespace Kwave
{
    // forward declarations
    class SignalManager;
    class SignalView;

    class SelectionBorderItem: public Kwave::ViewItem
    {
    public:
        /**
         * Constructor
         * @param view the owner (SignalView)
         * @param signal_manager the corresponding SignalManager
         * @param pos start position, either left or right border
         */
        SelectionBorderItem(SignalView &view,
                            Kwave::SignalManager &signal_manager,
                            sample_index_t pos);

        /** Destructor */
        ~SelectionBorderItem() override;

        /**
         * Returns flags describing the possible interactions with this object
         * @see Kwave::ViewItem::Flags
         */
        Kwave::ViewItem::Flags flags() const override;

        /**
         * Can be overwritten to return a tooltip. The default implementation
         * returns an empty string.
         *
         * @param ofs sample index the tooltip should refer to (unused)
         * @return an already localized tooltip
         */
        QString toolTip(sample_index_t &ofs) override;

        /**
         * Returns a mouse cursor used when moving the item
         */
        QCursor mouseCursor() const override;

        /**
         * handles updates when being moved with the mouse
         * @param mouse_pos position of the mouse, in pixel coordinates
         *                  relative to the parent widget
         */
        void moveTo(const QPoint &mouse_pos) override;

    private:

        /** selection handler */
        Kwave::MouseMark m_selection;

    };
}

#endif /* SELECTION_BORDER_ITEM_H */
