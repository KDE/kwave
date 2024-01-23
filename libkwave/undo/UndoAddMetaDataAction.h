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

#ifndef UNDO_ADD_META_DATA_ACTION_H
#define UNDO_ADD_META_DATA_ACTION_H

#include "config.h"

#include <QList>
#include <QString>

#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    class SignalManager;

    /**
     * Undo action for inserting meta data.
     */
    class Q_DECL_EXPORT UndoAddMetaDataAction: public Kwave::UndoAction
    {
    public:

        /**
         * Constructor
         * @param meta_data reference to the meta data that has been inserted
         */
        explicit UndoAddMetaDataAction(const Kwave::MetaDataList &meta_data);

        /** Destructor */
        virtual ~UndoAddMetaDataAction() Q_DECL_OVERRIDE;

        /**
         * Returns a verbose short description of the action.
         */
        QString description() Q_DECL_OVERRIDE;

        /** @see UndoAction::undoSize() */
        virtual qint64 undoSize() Q_DECL_OVERRIDE;

        /** @see UndoAction::redoSize() */
        virtual qint64 redoSize() Q_DECL_OVERRIDE;

        /** @see UndoAction::store() */
        virtual bool store(SignalManager &manager) Q_DECL_OVERRIDE;

        /** @see UndoAction::undo() */
        virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                        bool with_redo) Q_DECL_OVERRIDE;

    protected:

        /** description of the action */
        QString m_description;

        /** index of the first sample position */
        sample_index_t m_offset;

        /** number of affected samples */
        sample_index_t m_length;

    };
}

#endif /* UNDO_ADD_META_DATA_ACTION_H */

//***************************************************************************
//***************************************************************************
