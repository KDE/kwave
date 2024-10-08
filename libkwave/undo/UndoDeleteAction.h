/***************************************************************************
     UndoDeleteAction.h  -  UndoAction for deletion of a range of samples
                             -------------------
    begin                : Jun 08 2001
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

#ifndef UNDO_DELETE_ACTION_H
#define UNDO_DELETE_ACTION_H

#include "config.h"

#include <QList>
#include <QString>
#include <QVector>

#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"
#include "libkwave/undo/UndoAction.h"

class QWidget;

namespace Kwave
{

    class SignalManager;

    class UndoDeleteAction: public Kwave::UndoAction
    {
    public:

        /**
         * Constructor.
         * @param parent_widget the widget used as parent for displaying
         *                      error messages
         * @param track_list list of affected tracks
         * @param offset index of the first deleted sample
         * @param length number of samples to delete
         */
        UndoDeleteAction(QWidget *parent_widget,
                         const QVector<unsigned int> &track_list,
                         sample_index_t offset, sample_index_t length);

        /** Destructor */
        ~UndoDeleteAction() override;

        /** @see UndoAction::description() */
        QString description() override;

        /** @see UndoAction::undoSize() */
        qint64 undoSize() override;

        /** @see UndoAction::redoSize() */
        qint64 redoSize() override;

        /**
         * Stores the data needed for undo.
         * @param manager the SignalManager for modifying the signal
         * @note this is the second step, after size() has been called
         * @return true if successful, false if failed (e.g. out of memory)
         */
        bool store(Kwave::SignalManager &manager) override;

        /**
         * Copies the samples to be deleted to the internal buffer.
         * @see UndoAction::undo()
         */
        Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                        bool with_redo) override;

        /** dump, for debugging purposes */
        void dump(const QString &indent) override;

    private:

        /** parent widget for showing error messages */
        QWidget *m_parent_widget;

        /** storage for all deleted stripes */
        QList<Kwave::Stripe::List> m_stripes;

        /** storage for the affected meta data items */
        Kwave::MetaDataList m_meta_data;

        /** list of affected tracks */
        QVector<unsigned int> m_track_list;

        /** first deleted sample */
        sample_index_t m_offset;

        /** number of deleted samples */
        sample_index_t m_length;

        /** memory needed for undo */
        unsigned int m_undo_size;

    };
}

#endif /* UNDO_DELETE_ACTION_H */

//***************************************************************************
//***************************************************************************
