/***************************************************************************
                Track.h  -  collects one or more stripes in one track
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

#ifndef _TRACK_H_
#define _TRACK_H_

#include <limits.h>  // for UINT_MAX
#include <qobject.h>
#include <qlist.h>

#include "mt/SharedLock.h"

#include "libkwave/InsertMode.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Stripe.h"

class SampleWriter;
class SampleReader;

//***************************************************************************
class Track: public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor. Creates a new and empty track with
     * zero-length and no stripes
     */
    Track();

    /**
     * Constructor. Creates an empty track with one stripe
     * with specified length.
     */
    Track(unsigned int length);

    /**
     * Destructor.
     */
    virtual ~Track();

    /**
     * Returns the length of the track. This is equivalent
     * to the position of the last sample of the last Stripe.
     */
    unsigned int length();

    /**
     * Opens a stream for writing samples, starting at a
     * sample position.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleWriter *openSampleWriter(InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Opens a stream for reading samples. If the the last position
     * is omitted, the value UINT_MAX will be used.
     * @param left first offset to be read (default = 0)
     * @param right last position to read (default = UINT_MAX)
     */
    SampleReader *openSampleReader(unsigned int left = 0,
	unsigned int right = UINT_MAX);

    /**
     * Deletes a range of samples
     * @param offset index of the first sample
     * @param length number of samples
     */
    void deleteRange(unsigned int offset, unsigned int length);

    /** Returns the "selected" flag. */
    inline bool selected() { return m_selected; };

    /** Sets the "selected" flag. */
    inline void select(bool select) { m_selected = select; };

signals:

    /**
     * Emitted if the track has grown. This implies a modification of
     * the inserted data, so no extra sigSamplesModified is emitted.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see sigSamplesModified
     */
    void sigSamplesInserted(Track &src, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted if data has been removed from the track.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     */
    void sigSamplesDeleted(Track &src, unsigned int offset,
                           unsigned int length);

    /**
     * Emitted if some data within the track has been modified.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was modified
     * @param length number of samples modified
     */
    void sigSamplesModified(Track &src, unsigned int offset,
                            unsigned int length);

private slots:

    /**
     * Connected to each stripe's sigSamplesInserted.
     * @param src source stripe of the signal (*this)
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Stripe::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(Stripe &src, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to each stripe's sigSamplesDeleted.
     * @param src source stripe of the signal
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Stripe::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(Stripe &src, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to each stripe's sigSamplesModified
     * @param src source stripe of the signal
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @internal
     */
    void slotSamplesModified(Stripe &src, unsigned int offset,
                             unsigned int length);

private:
    /**
     * Returns the current length of the stripe in samples. This
     * function uses no locks and is therefore reserved for internal
     * usage from within locked functions.
     * @note this must be private, it dows no locking !
     */
    unsigned int unlockedLength();

    /**
     * Creates a new stripe with a start position and a length.
     * @param start offset of the first sample
     * @param length number of samples, zero is allowed
     * @note this must be private, it does no locking !
     */
    Stripe *newStripe(unsigned int start, unsigned int length);

private:
    /** read/write lock for access to the whole track */
    SharedLock m_lock;

    /** list of stripes (a track actually is a container for stripes) */
    QList<Stripe> m_stripes;

    /** True if the track is selected */
    bool m_selected;

};

#endif /* _TRACK_H_ */
