/***************************************************************************
            Functions.h  -  list of simple periodic functions
			     -------------------
    begin                : Jan 21 2001
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

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "config.h"
#include <math.h>

#include <qlist.h>
#include <qstringlist.h>
#include <qmap.h>

#include "libkwave/TypesMap.h"

typedef double(periodic_function_t)(double);

/**
 * @class Functions
 * Holds a list of simple periodic arithmetic functions. All functions are
 * normed to work within the interval [0...2 Pi]
 *
 * @bug the list of names contains only i18n names
 * @todo needs a map with internal names and i18n names, like in Interpolation
 */
class Functions
{
public:
    /** Constructor */
    Functions();

    /** Destructor */
    ~Functions();

    /** Returns the number of functions */
    unsigned int count();

    /**
     * Returns the name of a function. If the index is out of range,
     * the returned name will be that of the "zero()" function.
     * @param index [0...count-1]
     */
    QString name(unsigned int index);

    /**
     * Returns a reference to a function. If the index is out of range,
     * the returned function will be "zero()".
     * @param index [0...count-1]
     */
    periodic_function_t &function(unsigned int index);

private:

    class FunctionTypesMap:
	public TypesMap<unsigned int, periodic_function_t* >
    {
	public:
	/** fills the types map */
	virtual void fill();
    };

    /** map of periodic functions */
    FunctionTypesMap m_functions_map;

};

#endif /* _FUNCTIONS_H_ */
