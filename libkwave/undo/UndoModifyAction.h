/***************************************************************************
     UndoModifyAction.h  -  UndoAction for modifications on samples
                             -------------------
    begin                : May 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UNDO_MODIFY_ACTION_H
#define UNDO_MODIFY_ACTION_H

#include "config.h"

#include <QString>

#include "libkwave/Sample.h"
#include "libkwave/Stripe.h"

#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    class UndoModifyAction: public UndoAction
    {
    public:

        /**
         * Constructor.
         * @param track index of the track
         * @param offset index of the first modified sample
         * @param length number of samples
         */
        UndoModifyAction(unsigned int track, sample_index_t offset,
                         sample_index_t length);

        /** Destructor */
        virtual ~UndoModifyAction() override;

        /** @see UndoAction::description() */
        virtual QString description() override;

        /** @see UndoAction::undoSize() */
        virtual qint64 undoSize() override;

        /** @see UndoAction::redoSize() */
        virtual qint64 redoSize() override { return undoSize(); }

        /**
        * @see UndoAction::store()
        */
        virtual bool store(Kwave::SignalManager &manager) override;

        /**
         * Exchange samples from the current signal and the internal undo
         * buffer. So this instance will be re-used for redo and so does not
         * require any extra memory for redo.
         * @see UndoAction::undo()
         */
        virtual UndoAction *undo(Kwave::SignalManager &manager, bool with_redo)
            override;

    protected:

        /** index of the modified track */
        unsigned int m_track;

        /** first sample */
        sample_index_t m_offset;

        /** number of samples */
        sample_index_t m_length;

        /** storage for all deleted stripes */
        QList<Kwave::Stripe::List> m_stripes;

    };
}

#endif /* UNDO_MODIFY_ACTION_H */

//***************************************************************************
//***************************************************************************
