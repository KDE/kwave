/***************************************************************************
          ArtsMultiIO.h  -  template for multi-track aRts Source/Sink
                             -------------------
    begin                : Sun Dec 9 2001
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

#ifndef _ARTS_MULTI_IO_H_
#define _ARTS_MULTI_IO_H_

#include <qglobal.h> // for warning()
#include <qvector.h>
#include <arts/artsflow.h>

template <class BASE, class ARTS_IO, class ARTS_IMPL, class KWAVE_IO>
class ArtsMultiIO:public BASE
{
public:
    /**
     * Constructor. Creates all i/o objects and holds them in
     * an internal vector. If the creation of an object failed,
     * the initialization will be aborted and the count of
     * objects will be reduced.
     * @param io something like aa MultiTrackReader or MultiTrackWriter
     */
    ArtsMultiIO(KWAVE_IO &io)
	:m_count(io.count()), m_ios(io.count())
    {
	unsigned int t;
	
	for (t=0; t < m_count; t++) {
	    m_ios.insert(t, 0);
	
	    ARTS_IMPL *r = new ARTS_IMPL(io[t]);
	    ASSERT(r);
	    if (r) m_ios.insert(t, new ARTS_IO(
		ARTS_IO::_from_base(r)));
	
	    ASSERT(m_ios[t]);
	    if (!m_ios[t]) {
		warning("ArtsMultiIO: creation of adapter failed!!!");
		m_count = t;
		break;
	    }
	}
    };

    /**
     * Destructor.
     * Deletes all i/o objects in reverse order
     */
    virtual ~ArtsMultiIO() {
	m_ios.setAutoDelete(true);
	while (m_count--) {
	    m_ios.remove(m_count);
	}
    };

    /**
     * Returns one of the aRts i/o objects.
     * @param i index of the track [0..count-1]
     * @return pointer to the object or 0 if index is out of range
     */
    virtual Arts::Object *operator[](unsigned int i)
    {
	ASSERT(i < m_count);
	return (i < m_count) ? m_ios[i] : 0;
    };

    /** Calls start() for each aRts i/o object. */
    virtual void start() {
	for (unsigned int t=0; t < m_count; ++t) m_ios[t]->start();
    };

    /** Calls stop() for each aRts i/o object. */
    virtual void stop() {
	for (unsigned int t=0; t < m_count; ++t) m_ios[t]->stop();
    };

protected:

    /** number of aRts i/o objects */
    unsigned int m_count;

    /** vector of aRts i/o objects */
    QVector<ARTS_IO> m_ios;
};

#endif /* _ARTS_MULTI_IO_H_ */
