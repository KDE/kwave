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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/Qt>

#include "kdemacros.h"

#include "libgui/ViewItem.h"

class QMenu;

namespace Kwave
{

    class Label;

    class KDE_EXPORT LabelItem: public Kwave::ViewItem
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param index the one-based index of the label
	 * @param ms position of the label in ms
	 * @param label reference to the label
	 */
	LabelItem(unsigned int index, double ms, const Kwave::Label &label);

	/** Destructor */
	virtual ~LabelItem();

	/**
	 * Returns flags describing the possible interactions with this object
	 * @see Qt::ItemFlag
	 */
	virtual Qt::ItemFlags flags();

	/** Returns the index of the first visible sample */
	virtual sample_index_t first();

	/** Returns the index of the last visible sample */
	virtual sample_index_t last();

	/**
	 * Can be overwritten to return a tooltip.
	 *
	 * @param ofs offset within the object the tooltip should refer to
	 * @return an already localized tooltip
	 */
	virtual QString toolTip(sample_index_t ofs);

	/**
	 * Called to append entries to a context menu.
	 * @param parent context menu to add items
	 */
	virtual void appendContextMenu(QMenu *parent);

    private slots:

	/** context menu: "label / delete" */
	void contextMenuLabelDelete();

	/** context menu: "label / properties..." */
	void contextMenuLabelProperties();

    private:

	/** index of the label */
	unsigned int m_index;

	/** position of the label, sample index */
	sample_index_t m_pos;

	/** position of the label, milliseconds */
	double m_ms;

	/** description of the label */
	QString m_description;
    };

}

#endif /* LABEL_ITEM_H */

//***************************************************************************
//***************************************************************************
