/***************************************************************************
    MultiTrackWriter.cpp - writer for multi-track signals
                             -------------------
    begin                : Sat Jun 30 2001
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

#include "config.h"

#include <new>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoInsertAction.h"
#include "libkwave/undo/UndoModifyAction.h"
#include "libkwave/undo/UndoTransactionGuard.h"

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter()
    :Kwave::MultiWriter()
{
}

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter(
    Kwave::SignalManager &signal_manager,
    const QVector<unsigned int> &track_list,
    Kwave::InsertMode mode,
    sample_index_t left,
    sample_index_t right)
    :Kwave::MultiWriter()
{
    if (!init(signal_manager, track_list, mode, left, right)) {
        // out of memory or aborted
        qWarning("MultiTrackWriter::init FAILED");
     }
}

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter(Kwave::SignalManager &signal_manager,
                                          Kwave::InsertMode mode)
    :Kwave::MultiWriter()
{
    QVector<unsigned int> track_list = signal_manager.selectedTracks();
    sample_index_t left = 0;
    sample_index_t right = 0;

    if (signal_manager.length()) {
        // default if signal is present: current selection
        left  = signal_manager.selection().first();
        right = signal_manager.selection().last();
        if (left == right) {
            // if no selection: whole signal
            left  = 0;
            right = signal_manager.length() - 1;
        }
    }

    if (!init(signal_manager, track_list, mode, left, right)) {
        // out of memory or aborted
        qWarning("MultiTrackWriter::init FAILED");
     }
}

//***************************************************************************
Kwave::MultiTrackWriter::~MultiTrackWriter()
{
    flush();
    clear();
}

//***************************************************************************
bool Kwave::MultiTrackWriter::init(Kwave::SignalManager &signal_manager,
                                   const QVector<unsigned int> &track_list,
                                   Kwave::InsertMode mode,
                                   sample_index_t left,
                                   sample_index_t right)
{
    Kwave::UndoTransactionGuard guard(signal_manager, QString());

    unsigned int index = 0;
    foreach (unsigned int track, track_list) {
        // NOTE: this function is *nearly* identical to the one in the
        //       Signal class, except for undo support
        Kwave::Writer *writer = signal_manager.openWriter(
            mode, track, left, right);
        if (writer) {
            insert(index++, writer);

            // get the real/effective left and right sample
            left  = writer->first();
            right = writer->last();
        } else {
            // out of memory or aborted
            clear();
            return false;
        }
    }

    // skip all that undo handling below if undo is not enabled
    // or the writer creation has failed
    if (!signal_manager.undoEnabled()) return true;

    // enter a new undo transaction and let it close when the writer closes
    signal_manager.startUndoTransaction();
    QObject::connect(this, SIGNAL(destroyed()),
                     &signal_manager,   SLOT(closeUndoTransaction()),
                     Qt::QueuedConnection);

    // create an undo action for the modification of the samples
    UndoAction *undo = nullptr;
    switch (mode) {
        case Kwave::Append:
        case Kwave::Insert: {
            undo = new(std::nothrow) Kwave::UndoInsertAction(
                signal_manager.parentWidget(),
                track_list,
                left,
                right - left + 1
            );

            if (undo) {
                Kwave::Writer *last_writer = at(tracks() - 1);
                QObject::connect(
                    last_writer,
                    SIGNAL(sigSamplesWritten(sample_index_t)),
                    static_cast<Kwave::UndoInsertAction *>(undo),
                    SLOT(setLength(sample_index_t)));
            }
            break;
        }
        case Kwave::Overwrite: {
            foreach (unsigned int track, track_list) {
                undo = new(std::nothrow) Kwave::UndoModifyAction(
                    track, left, right - left + 1);
                if (!signal_manager.registerUndoAction(undo)) {
                    // aborted, do not continue without undo
                    clear();
                    return false;
                }
            }
            break;
        }
    }

    if ( (mode != Kwave::Overwrite) &&
         !signal_manager.registerUndoAction(undo) )
    {
        // aborted, do not continue without undo
        clear();
        return false;
    }

    // Everything was ok, the action now is owned by the current undo
    // transaction. The transaction is owned by the SignalManager and
    // will be closed when the writer gets closed.
    return true;
}

//***************************************************************************
//***************************************************************************

#include "moc_MultiTrackWriter.cpp"
