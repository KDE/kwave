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

#include <QObject>
#include <QString>
#include <Qt>

#include <TODO:kdemacros.h>

#include "libkwave/Sample.h"

class QMenu;

namespace Kwave
{
    class Q_DECL_EXPORT ViewItem: public QObject
    {
	Q_OBJECT

    public:
	/** Constructor */
	ViewItem();

	/** Destructor */
	virtual ~ViewItem();

	/**
	 * Returns flags describing the possible interactions with this object
	 * @see Qt::ItemFlag
	 */
	virtual Qt::ItemFlags flags();

	/** Returns the index of the first visible sample */
	virtual sample_index_t first() = 0;

	/** Returns the index of the last visible sample */
	virtual sample_index_t last() = 0;

	/**
	 * Can be overwritten to return a tooltip. The default implementation
	 * returns an empty string.
	 *
	 * @param ofs offset within the object the tooltip should refer to
	 * @return an already localized tooltip
	 */
	virtual QString toolTip(sample_index_t ofs);

	/**
	 * Called to append entries to a context menu. The default
	 * implementation does nothing.
	 * @param parent context menu to add items
	 */
	virtual void appendContextMenu(QMenu *parent);

    signals:

	/** forward a sigCommand to the next layer */
	void sigCommand(const QString &command);

    };
}

#endif /* VIEW_ITEM_H */

//***************************************************************************
//***************************************************************************
