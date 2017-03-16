/***************************************************************************
            ViewItem.h  -  base class for a visible item within a SignalView
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

#ifndef VIEW_ITEM_H
#define VIEW_ITEM_H

#include "config.h"

#include <QtGlobal>
#include <QCursor>
#include <QFlag>
#include <QObject>
#include <QPoint>
#include <QString>

#include "libkwave/Sample.h"

class QCursor;
class QMenu;

namespace Kwave
{
    // forward declarations
    class SignalManager;
    class SignalView;

    class Q_DECL_EXPORT ViewItem: public QObject
    {
	Q_OBJECT

    public:
	enum Flag
	{
	    None           = 0, /**< no special capabilities       */
	    CanGrabAndMove = 1, /**< can be grabbed and moved      */
	    CanDragAndDrop = 2  /**< can be used for drag and drop */
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	/**
	 * Constructor
	 * @param view pointer to the owner (a SignalView)
	 * @param signal_manager the corresponding SignalManager
	 */
	ViewItem(Kwave::SignalView &view, Kwave::SignalManager &signal_manager);

	/** Destructor */
	virtual ~ViewItem();

	/**
	 * Returns flags describing the possible interactions with this object
	 * @see Flags
	 */
	virtual Kwave::ViewItem::Flags flags() const;

	/**
	 * Can be overwritten to return a tooltip. The default implementation
	 * returns an empty string.
	 *
	 * @param ofs absolute sample index the tooltip should refer to
	 *            (can be modified)
	 * @return an already localized tooltip
	 */
	virtual QString toolTip(sample_index_t &ofs);

	/**
	 * Called to append entries to a context menu. The default
	 * implementation does nothing.
	 * @param parent context menu to add items
	 */
	virtual void appendContextMenu(QMenu *parent);

	/**
	 * Returns a mouse cursor used when moving the item
	 */
	virtual QCursor mouseCursor() const;

	/**
	 * Handles updates when being moved with the mouse.
	 * @param mouse_pos position of the mouse, in pixel coordinates
	 *                  relative to the parent widget
	 */
	virtual void moveTo(const QPoint &mouse_pos);

	/** Starts a drag & drop operation. */
	virtual void startDragging();

	/**
	 * Called when leaving the move or drag mode, when the mouse button
	 * has been released or the operation is done.
	 */
	virtual void done();

    signals:

	/** forward a sigCommand to the next layer */
	void sigCommand(const QString &command);

    protected:

	/** the owner of this view item (a SignalView) */
	Kwave::SignalView &m_view;

	/** the corresponding SignalManager */
	Kwave::SignalManager &m_signal_manager;

    };
}

#endif /* VIEW_ITEM_H */

//***************************************************************************
//***************************************************************************
