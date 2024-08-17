/***************************************************************************
     SelectionTracker.h  -  tracker for selection changes
                             -------------------
    begin                : Tue Feb 25 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef SELECTION_TRACKER_H
#define SELECTION_TRACKER_H

#include "config.h"
#include "libkwavegui_export.h"

#include <QtGlobal>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QRecursiveMutex>
#include <QUuid>
#include <QVector>

#include "libkwave/Sample.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoHandler.h"

namespace Kwave
{

    class Track;
    class UndoTransaction;

    class LIBKWAVEGUI_EXPORT SelectionTracker: public QObject,
                                          private Kwave::UndoHandler
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param signal the signal manager to track
         * @param offset index of the first selected sample
         * @param length number of selected samples
         * @param tracks list of selected tracks or null pointer for "all"
         */
        SelectionTracker(Kwave::SignalManager *signal,
                         sample_index_t offset,
                         sample_index_t length,
                         const QVector<unsigned int> *tracks);

        /** Destructor */
        ~SelectionTracker() override;

        /**
         * Returns all currently selected tracks
         */
        QList<QUuid> allTracks();

        /**
         * changes the selected range covered by the cache
         * @internal used during undo/redo
         * @param tracks list of selected tracks
         * @param offset index of the first selected sample
         * @param length number of selected samples
         */
        void selectRange(QList<QUuid> tracks,
                         sample_index_t offset, sample_index_t length);

        /**
         * Returns the start of the selected range
         * @return the first sample index
         */
        sample_index_t offset() const { return m_offset; }

        /**
         * Returns the length of the selected range
         * @return number of selected samples
         */
        sample_index_t length() const { return m_length; }

        /**
         * Returns the start of the selected range
         * @see offset()
         * @return the first sample index
         */
        sample_index_t first() const { return m_offset; }

        /**
         * Returns the end of the selected range
         * @return the last sample index
         */
        sample_index_t last() const {
            return m_offset + ((m_length) ? (m_length - 1) : 0);
        }

    signals:

        /**
         * Signals that a track has been inserted.
         * @param uuid unique ID of the track
         */
        void sigTrackInserted(const QUuid &uuid);

        /**
         * Signals that a track has been deleted.
         * @param uuid unique ID of the track
         */
        void sigTrackDeleted(const QUuid &uuid);

        /**
         * signals that the offset of the selection has changed
         * @param offset new index of the selected area [samples]
         */
        void sigOffsetChanged(sample_index_t offset);

        /**
         * signals that the length of the selection has changed
         * @param length new length of the selection [samples]
         */
        void sigLengthChanged(sample_index_t length);

        /**
         * signals that a range of samples has become invalid
         * @param track UUID of the track or null for "all tracks"
         * @param first index of the first invalidated sample
         * @param last index of the last invalidated sample
         */
        void sigInvalidated(const QUuid *track,
                            sample_index_t first,
                            sample_index_t last);

    private slots:

        /**
         * Connected to the signal's sigTrackInserted.
         * @param index the index [0...tracks()-1] of the inserted track
         * @param track pointer to the track instance
         * @see Signal::sigTrackInserted
         * @internal
         */
        void slotTrackInserted(unsigned int index, Kwave::Track *track);

        /**
         * Connected to the signal's sigTrackInserted.
         * @param index the index of the inserted track
         * @param track pointer to the track instance
         * @see Signal::sigTrackDeleted
         * @internal
         */
        void slotTrackDeleted(unsigned int index, Kwave::Track *track);

        /**
         * Connected to the signal's sigSamplesInserted.
         * @param track index of the source track [0...tracks-1]
         * @param offset position from which the data was inserted
         * @param length number of samples inserted
         * @see Signal::sigSamplesInserted
         * @internal
         */
        void slotSamplesInserted(unsigned int track, sample_index_t offset,
                                 sample_index_t length);

        /**
         * Connected to the signal's sigSamplesDeleted.
         * @param track index of the source track [0...tracks-1]
         * @param offset position from which the data was removed
         * @param length number of samples deleted
         * @see Signal::sigSamplesDeleted
         * @internal
         */
        void slotSamplesDeleted(unsigned int track, sample_index_t offset,
                                sample_index_t length);

        /**
         * Connected to the signal's sigSamplesModified
         * @param track index of the source track [0...tracks-1]
         * @param offset position from which the data was modified
         * @param length number of samples modified
         * @see Signal::sigSamplesModified
         * @internal
         */
        void slotSamplesModified(unsigned int track, sample_index_t offset,
                                 sample_index_t length);

    protected:

        /**
         * Called by an undo manager to notify us that it is time to
         * save data for undo.
         *
         * @param undo an undo transaction to append some undo data
         * @retval true if successful
         * @retval false if saving undo data failed, e.g. out of memory
         *               or aborted
         */
        bool saveUndoData(Kwave::UndoTransaction &undo) override;

    private:

        /**
         * Undo action for tracking selection changes
         */
        class LIBKWAVEGUI_EXPORT Undo: public Kwave::UndoAction
        {
        public:

            /**
             * Constructor
             * @param selection pointer to the corresponding selection tracker
             */
            explicit Undo(Kwave::SelectionTracker *selection);

            /** Destructor */
            ~Undo() override;

            /**
             * Returns a verbose short description of the action.
             */
            QString description() override;

            /**
             * Returns the required amount of memory that is needed for storing
             * undo data for the operation. This will be called to determine the
             * free memory to be reserved.
             * @note this is the first step (after the constructor)
             */
            qint64 undoSize() override;

            /**
             * Returns the difference of needed memory that is needed for
             * redo.
             */
            qint64 redoSize() override;

            /**
             * Stores the data needed for undo.
             * @param manager the SignalManager for modifying the signal
             * @note this is the second step, after size() has been called
             * @return true if successful, false if failed (e.g. out of memory)
             */
            bool store(Kwave::SignalManager &manager) override;

            /**
             * Takes back an action by creating a new undo action (for further
             * redo) and restoring the previous state.
             * @param manager the SignalManager for modifying the signal
             * @param with_redo if true a UndoAction for redo will be created
             * @note The return value is allowed to be the same object. This
             *       is useful for objects that can re-use their data for
             *       undo/redo. You have to check for this when deleting an
             *       UndoAction object after undo.
             */
            virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
                                            bool with_redo) override;

            /**
             * This undo action does not contribute to the modification
             * of the signal.
             * @return true always
             */
            bool containsModification() const override {
                return false;
            }

            /** dump, for debugging purposes */
            void dump(const QString &indent) override {
                qDebug("%s%s", DBG(indent), DBG(description()));
            }

        private:

            /** pointer to the overview cache */
            QPointer<Kwave::SelectionTracker> m_tracker;

            /** list of selected tracks */
            QList<QUuid> m_tracks;

            /** start of the selection, first sample */
            sample_index_t m_offset;

            /** number of selected samples */
            sample_index_t m_length;

        };

    private:

        /** signal with the data to be shown */
        QPointer<Kwave::SignalManager> m_signal;

        /** first sample index in the source */
        sample_index_t m_offset;

        /** length of the source in samples, or zero for "whole signal" */
        sample_index_t m_length;

        /** list of currently selected source tracks */
        QList<QUuid> m_tracks;

        /** if true, track the selection only, otherwise the whole signal */
        bool m_selection_only;

        /** mutex for threadsafe access to the selection */
        QRecursiveMutex m_lock;
    };
}

#endif /* SELECTION_TRACKER_H */
