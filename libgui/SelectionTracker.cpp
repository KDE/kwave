/***************************************************************************
   SelectionTracker.cpp  -  tracker for selection changes
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

#include "config.h"

#include "libkwave/Track.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoManager.h"
#include "libkwave/undo/UndoTransaction.h"

#include "libgui/SelectionTracker.h"

//***************************************************************************
Kwave::SelectionTracker::SelectionTracker(Kwave::SignalManager *signal,
                                          sample_index_t offset,
                                          sample_index_t length,
                                          const QList<unsigned int> *tracks)
    :m_signal(signal),
     m_offset(offset),
     m_length((length || !signal) ? length : signal->length()),
     m_tracks(),
     m_selection_only((length != 0)),
     m_lock(QMutex::Recursive)
{
    Q_ASSERT(signal);
    if (!signal) return;

    QObject::connect(
	signal, SIGNAL(sigTrackInserted(uint,Kwave::Track*)),
	this,   SLOT(slotTrackInserted(uint,Kwave::Track*)));
    connect(signal, SIGNAL(sigTrackDeleted(uint,Kwave::Track*)),
            this,   SLOT(slotTrackDeleted(uint,Kwave::Track*)));
    connect(
	signal,
	SIGNAL(sigSamplesDeleted(uint,sample_index_t,sample_index_t)),
	this,
	SLOT(slotSamplesDeleted(uint,sample_index_t,sample_index_t))
    );
    connect(
	signal,
	SIGNAL(sigSamplesInserted(uint,sample_index_t,sample_index_t)),
	this,
	SLOT(slotSamplesInserted(uint,sample_index_t,sample_index_t))
    );
    connect(
	signal,
	SIGNAL(sigSamplesModified(uint,sample_index_t,sample_index_t)),
	this,
	SLOT(slotSamplesModified(uint,sample_index_t,sample_index_t))
    );

    // register ourself at the undo manager
    Kwave::UndoManager &undo = signal->undoManager();
    undo.registerHandler(this);

    if (tracks && !tracks->isEmpty()) {
	// having a list of selected tracks
	foreach (unsigned int track, *tracks) {
	    slotTrackInserted(track, 0);
	    if (m_selection_only)
		m_tracks.append(m_signal->uuidOfTrack(track));
	}
    } else {
	// take over all tracks from the signal manager
	foreach (unsigned int track, m_signal->allTracks()) {
	    slotTrackInserted(track, 0);
	    if (m_selection_only)
		m_tracks.append(m_signal->uuidOfTrack(track));
	}
    }
}

//***************************************************************************
Kwave::SelectionTracker::~SelectionTracker()
{
}

//***************************************************************************
QList<QUuid> Kwave::SelectionTracker::allTracks()
{
    return m_tracks;
}

//***************************************************************************
bool Kwave::SelectionTracker::saveUndoData(Kwave::UndoTransaction &undo)
{
    // shortcut: we only have to do something when we are in
    //           "selection only" mode
    if (!m_selection_only)
	return true;

    Q_ASSERT(!m_signal.isNull());
    if (m_signal.isNull())
	return false; // should never happen

    Kwave::UndoAction *action =
	new(std::nothrow) Kwave::SelectionTracker::Undo(this);
    Q_ASSERT(action);
    if (!action) return false;

    if (action->store(*m_signal)) {
	undo.append(action);
    } else {
	// out of memory
	delete action;
	return false;
    }

    return true;
}

//***************************************************************************
void Kwave::SelectionTracker::slotTrackInserted(unsigned int index,
                                                Kwave::Track *track)
{
    QMutexLocker lock(&m_lock);

    if (m_selection_only)
	return; // in "selection only" mode we are not interested in this

    Q_ASSERT(track || !m_signal.isNull());
    if (!track && m_signal.isNull()) return;

    const QUuid &uuid = (track) ? track->uuid() : m_signal->uuidOfTrack(index);
    Q_ASSERT(!uuid.isNull());

    // track signal length changes when tracks were inserted
    sample_index_t new_len = m_signal->length();
    if (m_length != new_len) {
	m_length = new_len;
	emit sigLengthChanged(m_length);
    }

    // a new track has been inserted
    m_tracks.append(uuid);
    emit sigTrackInserted(uuid);
}

//***************************************************************************
void Kwave::SelectionTracker::slotTrackDeleted(unsigned int index,
                                               Kwave::Track *track)
{
    QMutexLocker lock(&m_lock);

    Q_UNUSED(index);

    Q_ASSERT(track);
    if (!track) return;

    const QUuid &uuid = track->uuid();
    Q_ASSERT(!uuid.isNull());
    if (!m_tracks.contains(uuid))
	return; // track not selected

    // track signal length changes when tracks were inserted
    if (!m_selection_only) {
	sample_index_t new_len = m_signal->length();
	if (m_length != new_len) {
	    m_length = new_len;
	    emit sigLengthChanged(m_length);
	}
    }

    // one of our selected tracks was deleted
    m_tracks.removeAll(uuid);
    emit sigTrackDeleted(uuid);
}

//***************************************************************************
void Kwave::SelectionTracker::slotSamplesInserted(unsigned int track,
                                                  sample_index_t offset,
                                                  sample_index_t length)
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(!m_signal.isNull());
    if (m_signal.isNull()) return;

    const QUuid uuid = m_signal->uuidOfTrack(track);
    if (!m_tracks.contains(uuid))
	return; // track not selected

    if (!length)
	return; // nothing to do

    // NOTE: adjust offsets/lengths only for the first selected track
    const bool is_first = (uuid == m_tracks.first());

    if (m_selection_only) {
	if (offset >= (m_offset + m_length))
	    return; // right of us

	if (offset < m_offset) {
	    // left of us, no overlap
	    if (is_first) {
		m_offset += length;
		emit sigOffsetChanged(m_offset);
	    }
	    return;
	}
    }

    // in our range -> increase length and invalidate all
    // samples from offset to end of file
    if (is_first) {
	m_length += length;
	emit sigLengthChanged(m_length);
    }

    emit sigInvalidated(&uuid, offset, SAMPLE_INDEX_MAX);
}

//***************************************************************************
void Kwave::SelectionTracker::slotSamplesDeleted(unsigned int track,
                                                 sample_index_t offset,
                                                 sample_index_t length)
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(!m_signal.isNull());
    if (m_signal.isNull()) return;

    const QUuid uuid = m_signal->uuidOfTrack(track);
    if (!m_tracks.contains(uuid))
	return; // track not selected

    if (!length)
	return; // nothing to do

    if (offset >= (m_offset + m_length))
	return; // right of us

    // NOTE: adjust offsets/lengths only for the first selected track
    const bool is_first = (uuid == m_tracks.first());

    if ((offset + length - 1) < m_offset) {
	// left of us, no overlap
	if (is_first) {
	    m_offset -= length;
	    emit sigOffsetChanged(m_offset);
	}
	return;
    }

    // determine shift and number of deleted samples
    sample_index_t first_del = offset;
    sample_index_t last_del  = offset + length - 1;
    sample_index_t shift = (first_del < m_offset) ? (m_offset - first_del) : 0;
    sample_index_t left  = qMax(first_del, m_offset);
    sample_index_t right = qMin(last_del,  m_offset + m_length - 1);
    sample_index_t deleted_samples = right - left + 1;

    // adjust the start of the selection
    if (is_first && shift) {
	m_offset -= shift;
	left      = m_offset;
	emit sigOffsetChanged(m_offset);
    }

    // adjust the length of the selection
    if (is_first && deleted_samples) {
	m_length -= deleted_samples;
	emit sigLengthChanged(m_length);
    }

    // in our range -> invalidate all samples from offset to end of file
    emit sigInvalidated(&uuid, left, SAMPLE_INDEX_MAX);
}

//***************************************************************************
void Kwave::SelectionTracker::slotSamplesModified(unsigned int track,
                                                  sample_index_t offset,
                                                  sample_index_t length)
{
    QMutexLocker lock(&m_lock);

    if (!length) return; // nothing to do

    Q_ASSERT(!m_signal.isNull());
    if (m_signal.isNull()) return;

    const QUuid uuid = m_signal->uuidOfTrack(track);
    if (!m_tracks.contains(uuid))
	return; // track not selected

    if (offset >= (m_offset + m_length))
	return; // right out of our range -> out of interest

    if (offset + length < m_offset)
	return; // completely left from us -> out of interest

    // overlapping
    sample_index_t first_mod = offset;
    sample_index_t last_mod  = offset + length - 1;
    sample_index_t left  = qMax(first_mod, m_offset);
    sample_index_t right = qMin(last_mod,  m_offset + m_length - 1);

    emit sigInvalidated(&uuid, left, right);
}

//***************************************************************************
void Kwave::SelectionTracker::selectRange(QList<QUuid> tracks,
                                          sample_index_t offset,
                                          sample_index_t length)
{
    QMutexLocker lock(&m_lock);

    // remove deleted tracks
    foreach (const QUuid &uuid, m_tracks) {
	if (!tracks.contains(uuid)) {
	    m_tracks.removeAll(uuid);
	    emit sigTrackDeleted(uuid);
	}
    }

    // add new tracks
    foreach (const QUuid &uuid, tracks) {
	if (!m_tracks.contains(uuid)) {
	    m_tracks.append(uuid);
	    emit sigTrackInserted(uuid);
	}
    }

    // track signal length changes when tracks were inserted
    if (!m_selection_only) {
	sample_index_t new_len = m_signal->length();
	if (m_length != new_len) {
	    m_length = new_len;
	    emit sigLengthChanged(m_length);
	}

	return; // nothing more to do in this mode
    }

    if ((offset == m_offset) && (length == m_length))
	return; // no change, nothing to do

    if (!length)
	return; // makes no sense

    // remember the old settings
    const sample_index_t old_ofs = m_offset;
    const sample_index_t old_len = m_length;

    // take over the new selection
    if (m_length != length) {
	m_length = length;
	emit sigLengthChanged(m_length);
    }
    if (offset != old_ofs) {
	// offset has changed -> invalidate all
	m_offset = offset;
	emit sigOffsetChanged(m_offset);
	emit sigInvalidated(0, m_offset, SAMPLE_INDEX_MAX);
    } else if (length > old_len) {
	// length has changed and increased -> invalidate new area
	emit sigInvalidated(0, m_offset + old_len - 1, SAMPLE_INDEX_MAX);
    } else if (length < old_len) {
	// length was reduced -> invalidate shrinked area at end
	emit sigInvalidated(0, m_offset + length - 1, SAMPLE_INDEX_MAX);
    }
}

//***************************************************************************
//***************************************************************************
Kwave::SelectionTracker::Undo::Undo(Kwave::SelectionTracker *selection)
    :Kwave::UndoAction(),
     m_tracker(selection),
     m_tracks(selection ? selection->allTracks() : QList<QUuid>()),
     m_offset(selection ? selection->offset() : 0),
     m_length(selection ? selection->length() : 0)
{
}

//***************************************************************************
Kwave::SelectionTracker::Undo::~Undo()
{
}

//***************************************************************************
QString Kwave::SelectionTracker::Undo::description()
{
    return QString();
}

//***************************************************************************
qint64 Kwave::SelectionTracker::Undo::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
qint64 Kwave::SelectionTracker::Undo::redoSize()
{
    return 0;
}

//***************************************************************************
bool Kwave::SelectionTracker::Undo::store(Kwave::SignalManager &manager)
{
    Q_UNUSED(manager); // data has already been stored in the constructor
    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::SelectionTracker::Undo::undo(
    Kwave::SignalManager &manager, bool with_redo)
{
    Q_UNUSED(manager);

    if (!m_tracker.isNull()) {
	QList<QUuid>   tracks = m_tracks;
	sample_index_t ofs    = m_offset;
	sample_index_t len    = m_length;

	m_tracks = m_tracker->allTracks();
	m_offset = m_tracker->offset();
	m_length = m_tracker->length();

	m_tracker->selectRange(tracks, ofs, len);
    }

    return (with_redo) ? this : 0;
}

//***************************************************************************
#include "SelectionTracker.moc"
//***************************************************************************
//***************************************************************************
