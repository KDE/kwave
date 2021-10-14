/***************************************************************************
                 Track.h -  collects one or more stripes in one track
			     -------------------
    begin                : Feb 09 2001
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

#ifndef TRACK_H
#define TRACK_H

#include "config.h"

#include <QtGlobal>
#include <QList>
#include <QObject>
#include <QReadWriteLock>
#include <QRecursiveMutex>
#include <QUuid>

#include "libkwave/InsertMode.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Stripe.h"

//***************************************************************************
namespace Kwave
{

    class SampleReader;
    class TrackWriter;
    class Writer;

    class Q_DECL_EXPORT Track: public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Default constructor. Creates a new and empty track with
	 * zero-length, no stripes and new uuid
	 */
	Track();

	/**
	 * Constructor. Creates an empty track with a specified length.
	 * @param length the length in samples
	 * @param uuid unique ID of the track, can be null
	 */
	Track(sample_index_t length, QUuid *uuid);

	/**
	 * Destructor.
	 */
	virtual ~Track();

	/**
	 * Returns the length of the track. This is equivalent
	 * to the position of the last sample of the last Stripe.
	 */
	sample_index_t length();

	/**
	 * Opens a stream for writing samples, starting at a
	 * sample position.
	 * @param mode specifies where and how to insert
	 * @param left start of the input (only useful in insert and
	 *             overwrite mode)
	 * @param right end of the input (only useful with overwrite mode)
	 * @see InsertMode
	 * @note destruction of the writer triggers defragmentation
	 */
	Kwave::Writer *openWriter(Kwave::InsertMode mode,
	                          sample_index_t left = 0,
	                          sample_index_t right = 0);

	/**
	 * Opens a stream for reading samples. If the last position
	 * is omitted, the value SAMPLE_INDEX_MAX will be used.
	 * @param mode read mode, see Kwave::ReaderMode
	 * @param left first offset to be read (default = 0)
	 * @param right last position to read (default = SAMPLE_INDEX_MAX)
	 */
	Kwave::SampleReader *openReader(Kwave::ReaderMode mode,
	    sample_index_t left = 0,
            sample_index_t right = SAMPLE_INDEX_MAX);

	/**
	 * Get a list of stripes that matches a given range of samples
	 * @param left  offset of the first sample
	 * @param right offset of the last sample
	 * @return a list of stripes that cover the given range
	 *         between left and right
	 */
	Kwave::Stripe::List stripes(sample_index_t left,
	                            sample_index_t right);

	/**
	 * Merge a list of stripes into the track.
	 * @param stripes list of stripes
	 * @return true if succeeded, false if failed
	 * @note triggers a defragmentation
	 */
	bool mergeStripes(const Kwave::Stripe::List &stripes);

	/**
	 * Deletes a range of samples
	 * @param offset index of the first sample
	 * @param length number of samples
	 * @param make_gap if true, make a gap into the list of stripes
	 *                 instead of moving the stuff from right to left
	 * @note triggers a defragmentation if make_gap is false
	 */
	void deleteRange(sample_index_t offset, sample_index_t length,
	                 bool make_gap = false);

	/**
	 * Inserts space at a given offset by moving all stripes that are
	 * are starting at or after the given offset right.
	 *
	 * @param offset position after which everything is moved right
	 * @param shift distance of the shift [samples]
	 * @return true if succeeded, false if failed (OOM?)
	 */
	bool insertSpace(sample_index_t offset, sample_index_t shift);

	/** Returns the "selected" flag. */
	inline bool selected() const { return m_selected; }

	/** Sets the "selected" flag. */
	void select(bool select);

	/** returns the unique ID of this track instance */
	const QUuid &uuid() const { return m_uuid; }

    public slots:

	/** toggles the selection of the slot on/off */
	void toggleSelection();

	/** do some defragmentation of stripes */
	void defragment();

    signals:

	/**
	 * Emitted if the track has grown. This implies a modification of
	 * the inserted data, so no extra sigSamplesModified is emitted.
	 * @param src source track of the signal (*this)
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see sigSamplesModified
	 */
	void sigSamplesInserted(Kwave::Track *src, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Emitted if data has been removed from the track.
	 * @param src source track of the signal (*this)
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 */
	void sigSamplesDeleted(Kwave::Track *src, sample_index_t offset,
                               sample_index_t length);

	/**
	 * Emitted if some data within the track has been modified.
	 * @param src source track of the signal (*this)
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 */
	void sigSamplesModified(Kwave::Track *src, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Emitted whenever the selection of the track has changed.
	 * @param selected true if selected, false if unselected
	 */
	void sigSelectionChanged(bool selected);

    private:
	/**
	 * Returns the current length of the stripe in samples. This
	 * function uses no locks and is therefore reserved for internal
	 * usage from within locked functions.
	 * @note this must be private, it does no locking !
	 */
	sample_index_t unlockedLength();

	/**
	 * Deletes a range of samples, used internally by deleteRange()
	 * @param offset index of the first sample
	 * @param length number of samples
	 * @param make_gap if true, make a gap into the list of stripes
	 *                 instead of moving the stuff from right to left
	 */
	void unlockedDelete(sample_index_t offset, sample_index_t length,
	                    bool make_gap = false);

	/**
	 * Append samples after a given stripe.
	 *
	 * @param stripe the stripe after which to instert. Null pointer is
	 *               allowed, in this case a new stripe is created
	 * @param offset position where the new data should start
	 * @param buffer array with samples
	 * @param buf_offset offset within the buffer
	 * @param length number of samples to write
	 * @return true if successful, false if failed (e.g. out of memory)
	 */
	bool appendAfter(Stripe *stripe, sample_index_t offset,
	                 const Kwave::SampleArray &buffer,
	                 unsigned int buf_offset, unsigned int length);

	/**
	 * Move all stripes after an offset to the right. Only looks at the
	 * start position of the stripes, comparing with ">=", if the start
	 * of a stripe is at the given offset, it will not be moved!
	 *
	 * @param offset position after which everything is moved right
	 * @param shift distance of the shift [samples]
	 */
	void moveRight(sample_index_t offset, sample_index_t shift);

	/**
	 * Append a new stripe with a given length.
	 *
	 * @param length number of samples, zero is allowed
	 */
	void appendStripe(sample_index_t length);

	/**
	 * Split a stripe into two stripes. The new stripe will be created
	 * from the right portion of the given stripe and the original
	 * stripe will be shrinked to it's new size. The newly created stripe
	 * will be inserted into m_stripes after the old one.
	 *
	 * @param stripe the stripe to be split
	 * @param offset the offset within the stripe, which becomes the first
	 *               sample in the new stripe
	 * @return the new created stripe
	 */
	Stripe splitStripe(Stripe &stripe, unsigned int offset);


	/**
	 * Merge a single stripe into the track.
	 *
	 * @param stripe the stripe to merge
	 * @return true if succeeded, false if failed
	 */
	bool mergeStripe(Kwave::Stripe &stripe);

	/**
	 * dump the list of stripes, for debugging
	 * @internal for debugging only
	 */
	void dump();

    protected:

	friend class Kwave::TrackWriter;

	/**
	 * Write a block of samples. If necessary it starts, appends to,
	 * or splits a stripe.
	 *
	 * @param mode a InsertMode (append/overwrite/insert)
	 * @param offset position where to start the write operation
	 * @param buffer array with samples
	 * @param buf_offset offset within the buffer
	 * @param length number of samples to write
	 * @return true if successful, false if failed (e.g. out of memory)
	 */
	bool writeSamples(Kwave::InsertMode mode,
	                  sample_index_t offset,
	                  const Kwave::SampleArray &buffer,
	                  unsigned int buf_offset,
	                  unsigned int length);

	/** increments the usage counter (read lock to m_lock_usage) */
	void use();

	/** decrements the usage counter (read lock to m_lock_usage) */
	void release();

    private:

	/**
	 * Creates a new stripe with a start position and a length.
	 * @param start offset of the first sample
	 * @param length number of samples, zero is allowed
	 * @note this must be private, it does no locking !
	 */
	Stripe *newStripe(sample_index_t start, unsigned int length);

    private:
	/** lock for access to the whole track */
	QRecursiveMutex m_lock;

	/** lock to protect against deletion while the track is in use */
	QReadWriteLock m_lock_usage;

	/** list of stripes (a track actually is a container for stripes) */
	QList<Stripe> m_stripes;

	/** True if the track is selected */
	bool m_selected;

	/** unique ID */
	QUuid m_uuid;
    };
}

#endif /* TRACK_H */

//***************************************************************************
//***************************************************************************
