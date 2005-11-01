/***************************************************************************
               Triple.h  -  Template class for holding three elements
			     -------------------
    begin                : Feb 04 2001
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

#ifndef _TRIPLE_H_
#define _TRIPLE_H_

#include "config.h"

template <class T1, class T2, class T3> class Triple
{
public:
    /** Stupid default constructor (sometimes needed but should not) */
    Triple()
	:m_first(),
	 m_second(),
	 m_third()
    {};

    /** Constructor with initialization data */
    Triple(const T1 &first, const T2 &second, const T3 &third)
	:m_first(first),
	 m_second(second),
	 m_third(third)
    {};

    /** Copy constructor */
    Triple(const Triple &copy)
	:m_first(copy.first()),
	 m_second(copy.second()),
	 m_third(copy.third())
    {};

    /** Destructor */
    virtual ~Triple() {};

    /** assignment operator */
    Triple<T1,T2,T3> &operator = (const Triple<T1,T2,T3> &t2) {
	qDebug("Triple<T1,T2,T3> operator = (const Triple<T1,T2,T3> &t2)");
	m_first  = t2.first();
	m_second = t2.second();
	m_third  = t2.third();
	return *this;
    };

    /** compare operator */
    inline bool operator==(const Triple<T1,T2,T3> &t2) {
	return (
	    ( m_first  == t2.first()  ) &&
	    ( m_second == t2.second() ) &&
	    ( m_third  == t2.third()  )
	);
    };

    /** returns a reference to the firstelement */
    inline const T1 &first() const { return m_first;  };

    /** returns a reference to the second element */
    inline const T2 &second() const { return m_second; };

    /** returns a reference to the third element */
    inline const T3 &third()  const { return m_third;  };

private:
    /** first element */
    T1 m_first;

    /** second element */
    T2 m_second;

    /** guess what... */
    T3 m_third;
};

#endif /* _TRIPLE_H_ */
