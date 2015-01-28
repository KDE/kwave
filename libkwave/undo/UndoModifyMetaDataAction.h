/***************************************************************************
    UndoModifyMetaDataAction.h  -  Undo action for modifying meta data
			     -------------------
    begin                : Sun Apr 03 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _UNDO_MODIFY_META_DATA_ACTION_H_
#define _UNDO_MODIFY_META_DATA_ACTION_H_

#include "config.h"

#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/MetaDataList.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    /**
     * Undo action for modifying meta data
     */
    class KDE_EXPORT UndoModifyMetaDataAction: public UndoAction
    {
    public:

	/**
	 * Constructor
	 * @param meta_data list of meta data that has been changed
	 */
	explicit UndoModifyMetaDataAction(const Kwave::MetaDataList &meta_data);

	/** Destructor */
	virtual ~UndoModifyMetaDataAction();

	/**
	* Returns a verbose short description of the action.
	*/
	virtual QString description();

	/** @see UndoAction::undoSize() */
	virtual qint64 undoSize();

	/** @see UndoAction::redoSize() */
	virtual qint64 redoSize();

	/** @see UndoAction::store() */
	virtual bool store(Kwave::SignalManager &manager);

	/** @see UndoAction::undo() */
	virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
	                                bool with_redo);

	/** dump, for debugging purposes */
	virtual void dump(const QString &indent);

    protected:

	/** the unmodified metadata */
	Kwave::MetaDataList m_saved_data;

    };
}

#endif /* _UNDO_MODIFY_META_DATA_ACTION_H_ */

//***************************************************************************
//***************************************************************************
