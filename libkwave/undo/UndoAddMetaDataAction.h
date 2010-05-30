/***************************************************************************
   UndoAddMetaDataAction.h  -  Undo action for insertion of meta data
			     -------------------
    begin                : Wed Aug 16 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef _UNDO_ADD_META_DATA_ACTION_H_
#define _UNDO_ADD_META_DATA_ACTION_H_

#include "config.h"

#include <QList>
#include <QString>

#include "libkwave/Sample.h"
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/undo/UndoAction.h"

class SignalManager;

/**
 * Undo action for inserting meta data.
 */
class UndoAddMetaDataAction: public UndoAction
{
public:

    /**
     * Constructor
     * @param meta_data reference to the meta data that has been inserted
     */
    UndoAddMetaDataAction(const Kwave::MetaDataList &meta_data);

    /** Destructor */
    virtual ~UndoAddMetaDataAction();

    /**
     * Returns a verbose short description of the action.
     */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /** @see UndoAction::store() */
    virtual bool store(SignalManager &manager);

    /** @see UndoAction::undo() */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

protected:

    /** description of the action */
    QString m_description;

    /** index of the first sample position */
    sample_index_t m_first;

    /** index of the last sample position */
    sample_index_t m_last;

    /** list of affected track inidices */
    QList<unsigned int> m_tracks;

};

#endif /* _UNDO_ADD_META_DATA_ACTION_H_ */
