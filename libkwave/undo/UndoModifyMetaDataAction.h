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

#ifndef UNDO_MODIFY_META_DATA_ACTION_H
#define UNDO_MODIFY_META_DATA_ACTION_H

#include "config.h"

#include <QtGlobal>
#include <QString>

#include "libkwave/MetaDataList.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    /**
     * Undo action for modifying meta data
     */
    class Q_DECL_EXPORT UndoModifyMetaDataAction: public UndoAction
    {
    public:

        /**
         * Constructor
         * @param meta_data list of meta data that has been changed
         */
        explicit UndoModifyMetaDataAction(const Kwave::MetaDataList &meta_data);

        /** Destructor */
        virtual ~UndoModifyMetaDataAction() Q_DECL_OVERRIDE;

        /**
        * Returns a verbose short description of the action.
        */
        virtual QString description() Q_DECL_OVERRIDE;

        /** @see UndoAction::undoSize() */
        virtual qint64 undoSize() Q_DECL_OVERRIDE;

        /** @see UndoAction::redoSize() */
        virtual qint64 redoSize() Q_DECL_OVERRIDE;

        /** @see UndoAction::store() */
        virtual bool store(Kwave::SignalManager &manager) Q_DECL_OVERRIDE;

        /** @see UndoAction::undo() */
        virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                        bool with_redo) Q_DECL_OVERRIDE;

        /** dump, for debugging purposes */
        virtual void dump(const QString &indent) Q_DECL_OVERRIDE;

    protected:

        /** the unmodified metadata */
        Kwave::MetaDataList m_saved_data;

    };
}

#endif /* UNDO_MODIFY_META_DATA_ACTION_H */

//***************************************************************************
//***************************************************************************
