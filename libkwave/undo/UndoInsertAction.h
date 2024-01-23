/***************************************************************************
     UndoInsertAction.h  -  UndoAction for insertion of a range of samples
                             -------------------
    begin                : Jun 14 2001
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

#ifndef UNDO_INSERT_ACTION_H
#define UNDO_INSERT_ACTION_H

#include "config.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

#include "libkwave/Sample.h"
#include "libkwave/undo/UndoAction.h"

class QWidget;

namespace Kwave
{

    class SignalManager;

    class UndoInsertAction: public QObject, public Kwave::UndoAction
    {
        Q_OBJECT
    public:

        /**
         * Constructor.
         * @param parent_widget the widget used as parent for displaying
         *                      error messages
         * @param track_list list of affected tracks
         * @param offset index of the first inserted sample
         * @param length number of inserted samples
         */
        UndoInsertAction(QWidget *parent_widget,
                         const QVector<unsigned int> &track_list,
                         sample_index_t offset,
                         sample_index_t length);

        /** @see UndoAction::description() */
        virtual QString description() Q_DECL_OVERRIDE;

        /** @see UndoAction::undoSize() */
        virtual qint64 undoSize() Q_DECL_OVERRIDE;

        /** @see UndoAction::redoSize() */
        virtual qint64 redoSize() Q_DECL_OVERRIDE;

        /**
         * @see UndoAction::store()
         */
        virtual bool store(Kwave::SignalManager &manager) Q_DECL_OVERRIDE;

        /**
         * Removes samples from the track.
         * @see UndoAction::undo()
         */
        virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                        bool with_redo) Q_DECL_OVERRIDE;

        /** dump, for debugging purposes */
        virtual void dump(const QString &indent) Q_DECL_OVERRIDE;

    public slots:

        /**
         * Can be connected to a Writer's <c>sigSamplesWritten</c> signal
         * if the writer has been opened in insert or append mode. In these
         * cases the undo action's length only is determined when the writer
         * gets closed.
         * @see Kwave::Writer::sigSamplesWritten
         */
        void setLength(sample_index_t length);

    protected:

        /** parent widget for showing error messages */
        QWidget *m_parent_widget;

        /** list of affected tracks */
        QVector<unsigned int> m_track_list;

        /** first sample */
        sample_index_t m_offset;

        /** number of samples */
        sample_index_t m_length;

    };
}

#endif /* UNDO_INSERT_ACTION_H */

//***************************************************************************
//***************************************************************************
