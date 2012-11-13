/***************************************************************************
 TransmissionFunction.h  - virtual base class for transmission functions
			     -------------------
    begin                : Mar 14 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _TRANSMISSION_FUNCTION_H_
#define _TRANSMISSION_FUNCTION_H_

#include "config.h"

namespace Kwave
{
    class TransmissionFunction
    {
    public:

	/** Destructor */
	virtual ~TransmissionFunction() {};

	/**
	 * Returns the value of the transmission function at a given
	 * frequency.
	 * @param f frequency, normed to be between 0 and PI
	 * @return amplitude at the given frequency, normed to 1.0
	 * @todo convert to a function that returns a complex value
	 *       with phase info
	 */
	virtual double at(double f) = 0;
    };
}

#endif /* _TRANSMISSION_FUNCTION_H_ */

//***************************************************************************
//***************************************************************************
