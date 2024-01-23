/***************************************************************************
      UndoDeleteTrack.h  -  Undo action for deletion of tracks
                             -------------------
    begin                : Mon Jun 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef UNDO_DELETE_TRACK_H
#define UNDO_DELETE_TRACK_H

#include "config.h"

#include <QString>
#include <QUuid>

#include "libkwave/Sample.h"
#include "libkwave/Stripe.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    class Signal;

    /**
     * Undo action for deleting a track.
     */
    class UndoDeleteTrack: public Kwave::UndoAction
    {
    public:

        /**
         * Constructor
         * @param signal reference to the signal
         * @param track index of the deleted track.
         */
        UndoDeleteTrack(Kwave::Signal &signal, unsigned int track);

        /** Destructor */
        virtual ~UndoDeleteTrack() Q_DECL_OVERRIDE;

        /**
         * Returns a verbose short description of the action.
         */
        virtual QString description() Q_DECL_OVERRIDE;

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

        /** Reference to the signal */
        Kwave::Signal &m_signal;

        /** Index of the deleted track */
        unsigned int m_track;

        /** Length of the track in samples */
        sample_index_t m_length;

        /** storage for all deleted stripes */
        QList<Kwave::Stripe::List> m_stripes;

        /** unique ID of the deleted track */
        QUuid m_uuid;

    };
}

#endif /* UNDO_DELETE_TRACK_H */

//***************************************************************************
//***************************************************************************
