/***************************************************************************
            LabelItem.h  -  label item within a SignalView
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

#ifndef LABEL_ITEM_H
#define LABEL_ITEM_H

#include "config.h"

#include <QtGlobal>
#include <QCursor>
#include <QObject>
#include <QString>

#include "libgui/ViewItem.h"

class QMenu;

namespace Kwave
{

    class Label;
    class SignalView;
    class UndoTransactionGuard;

    class Q_DECL_EXPORT LabelItem: public Kwave::ViewItem
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param view the owner (SignalView)
	 * @param signal_manager the corresponding SignalManager
	 * @param index the one-based index of the label
	 * @param label reference to the label
	 */
	LabelItem(Kwave::SignalView &view,
	          Kwave::SignalManager &signal_manager,
	          unsigned int index,
	          const Kwave::Label &label);

	/** Destructor */
	virtual ~LabelItem();

	/**
	 * Returns flags describing the possible interactions with this object
	 * @see Kwave::ViewItem::Flags
	 */
        virtual Kwave::ViewItem::Flags flags() const Q_DECL_OVERRIDE;

	/**
	 * Can be overwritten to return a tooltip.
	 *
	 * @param ofs sample index the tooltip should refer to (unused)
	 * @return an already localized tooltip
	 */
        virtual QString toolTip(sample_index_t &ofs) Q_DECL_OVERRIDE;

	/**
	 * Called to append entries to a context menu.
	 * @param parent context menu to add items
	 */
        virtual void appendContextMenu(QMenu *parent) Q_DECL_OVERRIDE;

	/**
	 * Returns a mouse cursor used when moving the item
	 */
        virtual QCursor mouseCursor() const Q_DECL_OVERRIDE;

	/**
	 * handles updates when being moved with the mouse
	 * @param mouse_pos position of the mouse, in pixel coordinates
	 *                  relative to the parent widget
	 */
        virtual void moveTo(const QPoint &mouse_pos) Q_DECL_OVERRIDE;

	/**
	 * Called when leaving the move mode, when the mouse button
	 * has been released.
	 */
        virtual void done() Q_DECL_OVERRIDE;

    private slots:

	/** context menu: "label / delete" */
	void contextMenuLabelDelete();

	/** context menu: "label / properties..." */
	void contextMenuLabelProperties();

    private:

	/** index of the label */
	unsigned int m_index;

	/** initial position of the label, in samples */
	sample_index_t m_initial_pos;

	/** current position of the label, in samples */
	sample_index_t m_current_pos;

	/** description of the label */
	QString m_description;

	/** used when we are within a undo transaction (we started) */
	Kwave::UndoTransactionGuard *m_undo_transaction;

    };

}

#endif /* LABEL_ITEM_H */

//***************************************************************************
//***************************************************************************
