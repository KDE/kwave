/***************************************************************************
  libkwave/SampleLock.h  -  Lock object for a range of samples
                             -------------------
    begin                : Fri Apr 13 2001
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _SAMPLE_LOCK_H_
#define _SAMPLE_LOCK_H_

class Track;

/**
 * The SampleLock class is used to lock a range of samples of one track
 * in a multithreaded system. There are several modes available,
 * described in LockMode, so that it is possible to read a range of samples
 * from multiple threads or write to the range exclusively.
 *
 * Please hold in mind that this <b>locks</b> your calling thread until the
 * lock has succeeded, so it is higly recommended to avoid multiple locks
 * for one track within the same thread!
 *
 * The ranges of samples specified here do not necessarily have anything
 * to do with the real samples, that means that no range checking is done
 * here.
 *
 */
class SampleLock
{

public:

    /**
     * The LockMode specifies an access mode for a range of samples
     * that have to be locked. The enumeration values have a special
     * meaning: they are bitmasks that can be split into two parts.
     * The lower 8 bits (value & 0x0F) serves as a unique internal
     * identifier of the mode. The upper 8 bits (value >> 8) serve
     * as a bitmap of mode identifiers that it excludes or conflicts
     * with.
     * By this, a check if a mode "a" conflicts with a mode "b" can
     * be easily done by evaluating an expression like
     * (a & 0x0F) & (b >> 8). If the return value is zero, there are
     * no conflicts and the two lock modes do not conflict.
     */
    enum LockMode {
	/** read-only, but allow others to write */
	ReadShared     = (unsigned int)(1 + (8) << 8),
	
	/** writing, but allow others to read.
	    @warning Use this only if you are really
	    sure that the writes and reads do *NOT*
	    overlap! */
	WriteShared    = (unsigned int)(2 + (8+4+2) << 8),
	
	/** read-only, writing for others forbidden */
	ReadExclusive  = (unsigned int)(8 + (8+4+2) << 8),
	
	/** writing, allow no other reads/writes */
	WriteExclusive = (unsigned int)(4 + (8+4+2+1) << 8)
    };

   /**
     * Constructor. Initializes the lock object and returns only
     * after after the specified range has been locked.
     */
    SampleLock(Track &track, unsigned int offset, unsigned int length,
	LockMode mode);

    /** Destructor. Releases the lock. */
    virtual ~SampleLock();

    /** Returns our lock mode */
    inline LockMode mode() {
	return m_mode;
    };

    /** Returns the index of the first locked sample. */
    inline unsigned int offset() {
	return m_offset;
    }

    /** Returns the number of locked samples. */
    inline unsigned int length() {
	return m_length;
    }

    /**
     * Returns true if an other SampleLock conflicts with our range and
     * access mode. A special case is when one range is zero-length. In
     * this case the range will be treated as if it has length one!
     * @param other the lock to be checked
     */
    bool conflictsWith(SampleLock &other);

private:

    /** our lock mode */
    LockMode m_mode;

    /** index of the first locked sample */
    unsigned int m_offset;

    /** number of locked samples */
    unsigned int m_length;

};

#endif /* _SAMPLE_LOCK_H_ */

//***************************************************************************
//***************************************************************************
