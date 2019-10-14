/***************************************************************************
   Signal.h - representation of a Kwave signal with multiple tracks
			     -------------------
    begin                : Sat Feb 03 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#ifndef SIGNAL_H
#define SIGNAL_H

#define PROGRESS_SIZE (512 * 3 * 5)

#include "config.h"

#include <pthread.h>

#include <QtGlobal>
#include <QList>
#include <QReadWriteLock>
#include <QUuid>
#include <QVector>

#include "libkwave/InsertMode.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/Sample.h"
#include "libkwave/Stripe.h"
#include "libkwave/WindowFunction.h"

//**********************************************************************
namespace Kwave
{

    class MultiTrackReader;
    class SampleReader;
    class Track;
    class Writer;

    class Q_DECL_EXPORT Signal: public QObject
    {
	Q_OBJECT

    public:

	/**
	 * Default Constructor. Creates an empty signal with
	 * zero-length and no tracks
	 */
	Signal();

	/**
	 * Constructor. Creates an empty signal with a specified
	 * number of tracks and length. Each track will contain
	 * only one stripe.
	 */
	Signal(unsigned int tracks, sample_index_t length);

	/**
	 * Destructor.
	 */
	virtual ~Signal();

	/**
	 * Closes the signal by removing all tracks.
	 */
	void close();

	/**
	 * Inserts a new track to into the track list or appends it to the end.
	 * @param index the position where to insert [0...tracks()]. If the
	 *        position is at or after the last track, the new track will
	 *        be appended to the end.
	 * @param length number of samples of the new track (zero is allowed)
	 * @param uuid pointer to a unique ID (optional, can be null)
	 * @return pointer to the created track. If the length is
	 *         omitted or zero, the track will have zero length.
	 */
	Kwave::Track *insertTrack(unsigned int index,
	                          sample_index_t length,
	                          QUuid *uuid);

	/**
	 * Appends a new track to the end of the tracks list, shortcut for
	 * insertTrack(tracks()-1, length)
	 * @see insertTrack
	 * @param length number of samples of the new track (zero is allowed)
	 * @param uuid pointer to a unique ID (optional, can be null)
	 */
	Kwave::Track *appendTrack(sample_index_t length,
	                          QUuid *uuid);

	/**
	 * Deletes a track.
	 * @param index the index of the track to be deleted [0...tracks()-1]
	 */
	void deleteTrack(unsigned int index);

	/**
	 * Returns an array of indices of all present tracks.
	 */
	QVector<unsigned int> allTracks();

	/**
	 * Opens an output stream for a track, starting at a specified sample
	 * position.
	 * @param mode specifies where and how to insert
	 * @param track index of the track
	 *  @param left start of the input (only useful in insert and
	 *             overwrite mode)
	 * @param right end of the input (only useful with overwrite mode)
	 * @return a Writer or null pointer if the track does not exist
	 * @see InsertMode
	 */
	Kwave::Writer *openWriter(Kwave::InsertMode mode,
	                          unsigned int track,
	                          sample_index_t left = 0,
	                          sample_index_t right = 0);

	/**
	 * Opens a stream for reading samples. If the last position
	 * is omitted, the value SAMPLE_INDEX_MAX will be used.
	 * @param mode a reader mode, see Kwave::ReaderMode
	 * @param track index of the track
	 * @param left first offset to be read (default = 0)
	 * @param right last position to read (default = SAMPLE_INDEX_MAX)
	 * @return a SampleReader or null if the track does not exist
	 */
	Kwave::SampleReader *openReader(Kwave::ReaderMode mode,
	                          unsigned int track,
	                          sample_index_t left = 0,
	                          sample_index_t right = SAMPLE_INDEX_MAX);

	/**
	 * Get a list of stripes that matches a given range of samples
	 * @param track index of the track
	 * @param left  offset of the first sample
	 * @param right offset of the last sample (default = SAMPLE_INDEX_MAX)
	 * @return a list of stripes that cover the given range
	 *         between left and right
	 */
	Kwave::Stripe::List stripes(unsigned int track,
	                            sample_index_t left = 0,
	                            sample_index_t right = SAMPLE_INDEX_MAX);

	/**
	 * Merge a list of stripes into the signal.
	 * @param stripes list of stripes
	 * @param track index of the track
	 * @return true if succeeded, false if failed
	 */
	bool mergeStripes(const Kwave::Stripe::List &stripes,
	                  unsigned int track);

	/**
	 * Returns the number of tracks.
	 */
	unsigned int tracks();

	/**
	 * Deletes a range of samples
	 * @param track index of the track
	 * @param offset index of the first sample
	 * @param length number of samples
	 */
	void deleteRange(unsigned int track,
	                 sample_index_t offset,
	                 sample_index_t length);

	/**
	 * Inserts some space at a given position
	 * @param track index of the track
	 * @param offset index of the first sample
	 * @param length number of samples
	 */
	void insertSpace(unsigned int track,
	                 sample_index_t offset,
	                 sample_index_t length);

	/**
	 * Returns the length of the signal. This is determined by
	 * searching for the highest sample position of all tracks.
	 */
	sample_index_t length();

	/**
	 * Queries if a track is selected. If the index of the track is
	 * out of range, the return value will be false.
	 */
	bool trackSelected(unsigned int track);

	/**
	 * Sets the "selected" flag of a track.
	 * @param track index of the track [0...tracks-1]
	 * @param select true if the track should be selected,
	 *               false for de-selecting
	 */
	void selectTrack(unsigned int track, bool select);

	/**
	 * Returns the uuid of a track
	 * @param track index of the track [0...tracks-1]
	 * @return the QUuid of the track or a "null" uuid if the track
	 *         does not exist
	 */
	QUuid uuidOfTrack(unsigned int track);

    signals:

	/**
	 * Signals that a track has been inserted.
	 * @param index position of the new track [0...tracks()-1]
	 * @param track reference to the new track
	 */
	void sigTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Signals that a track has been deleted.
	 * @param index position of the deleted track [0...tracks()-1]
	 * @param track reference to the deleted track
	 */
	void sigTrackDeleted(unsigned int index, Kwave::Track *track);

	/**
	 * Signals that the selection of one of the tracks has changed
	 * @param enabled state of the track, true=selected
	 */
	void sigTrackSelectionChanged(bool enabled);

	/**
	 * Emitted if samples have been inserted into a track. This implies
	 * a modification of the inserted data, so no extra sigSamplesModified
	 * is emitted.
	 * @param track index of the track
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see sigSamplesModified
	 */
	void sigSamplesInserted(unsigned int track, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Emitted if samples have been removed from a track.
	 * @param track index of the track
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 */
	void sigSamplesDeleted(unsigned int track, sample_index_t offset,
	                       sample_index_t length);

	/**
	 * Emitted if samples within a track have been modified.
	 * @param track index of the track
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 */
	void sigSamplesModified(unsigned int track, sample_index_t offset,
	                        sample_index_t length);

    private slots:

	/**
	 * Connected to each track's sigSamplesInserted.
	 * @param src source track
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see Track::sigSamplesInserted
	 * @internal
	 */
	void slotSamplesInserted(Kwave::Track *src, sample_index_t offset,
	                         sample_index_t length);

	/**
	 * Connected to each track's sigSamplesDeleted.
	 * @param src source track
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 * @see Track::sigSamplesDeleted
	 * @internal
	 */
	void slotSamplesDeleted(Kwave::Track *src, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Connected to each track's sigSamplesModified
	 * @param src source track
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 * @see Track::sigSamplesModified
	 * @internal
	 */
	void slotSamplesModified(Kwave::Track *src, sample_index_t offset,
	                         sample_index_t length);

    private:

	/**
	 * Looks up the index of a track in the track list
	 * @param track reference to the trac to be looked up
	 * @returns index of the track [0...tracks()-1] or tracks() if not found
	 */
	unsigned int trackIndex(const Kwave::Track *track);

    private:

	/** list of tracks */
	QList<Kwave::Track *> m_tracks;

	/** mutex for access to the track list */
	QReadWriteLock m_lock_tracks;

    };
}

#endif  /* SIGNAL_H */

//***************************************************************************
//***************************************************************************
