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

#include <qobject.h>
#include <qlist.h>

#include "mt/Mutex.h"

#include "libkwave/InsertMode.h"
#include "libkwave/Stripe.h"

class SampleInputStream;

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
     * Appends a new empty stripe to the end of the track.
     */
    Stripe *appendStripe(unsigned int length);

    /**
     * Returns the length of the track. This is equivalent
     * to the position of the last sample of the last Stripe.
     */
    unsigned int length();

    /**
     * Opens an input stream for a track, starting at a specified sample
     * position.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleInputStream *openInputStream(InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

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

protected:
friend class SampleIputStream;

    /** returns the mutex for access to the whole track. */
    inline Mutex &mutex() {
	return m_lock;
    };

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
     */
    unsigned int unlockedLength();

    /** mutex for access to the whole track */
    Mutex m_lock;

    /** list of stripes (a track actually is a container for stripes) */
    QList<Stripe> m_stripes;

    /** mutex for access to the stripes list */
    Mutex m_lock_stripes;

};

#endif /* _TRACK_H_ */
