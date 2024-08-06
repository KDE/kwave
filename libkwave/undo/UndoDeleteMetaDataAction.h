/***************************************************************************
    UndoDeleteMetaDataAction.h  -  Undo action for deleting meta data
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

#ifndef UNDO_DELETE_META_DATA_ACTION_H
#define UNDO_DELETE_META_DATA_ACTION_H

#include "config.h"
#include <QString>

#include "libkwave/MetaDataList.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    class SignalManager;

    /**
     * Undo action for deleting a list of meta data items.
     */
    class Q_DECL_EXPORT UndoDeleteMetaDataAction: public Kwave::UndoAction
    {
    public:

        /**
         * Constructor
         * @param meta_data reference to the meta data that should be deleted
         */
        explicit UndoDeleteMetaDataAction(const Kwave::MetaDataList &meta_data);

        /** Destructor */
        virtual ~UndoDeleteMetaDataAction() override;

        /**
         * Returns a verbose short description of the action.
         */
        virtual QString description() override;

        /** @see UndoAction::undoSize() */
        virtual qint64 undoSize() override;

        /** @see UndoAction::redoSize() */
        virtual qint64 redoSize() override;

        /** @see UndoAction::store() */
        virtual bool store(Kwave::SignalManager &manager) override;

        /** @see UndoAction::undo() */
        virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                        bool with_redo) override;

        /** dump, for debugging purposes */
        virtual void dump(const QString &indent) override;

    protected:

        /** the list of deleted meta data items */
        Kwave::MetaDataList m_meta_data;

    };
}

#endif /* UNDO_DELETE_META_DATA_ACTION_H */

//***************************************************************************
//***************************************************************************
