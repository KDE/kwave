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

#include <math.h>
#include <qlist.h>

typedef double(periodic_function_t)(double);

class Functions {

public:
    /** Constructor */
    Functions();

    /** Destructor */
    ~Functions();

    /** Returns the number of functions */
    unsigned int count();

    /**
     * Returns the name of a function
     * @param index [0...count-1]
     */
    QString name(unsigned int index);

    /**
     * Returns a reference to a function
     * @param index [0...count-1]
     */
    periodic_function_t &function(unsigned int index);

protected:

    /**
     * Appends a new function, internally used in constructor
     * @param func pointer to the function
     * @param name verbose name of the function
     */
    void append(periodic_function_t *func, QString name);

private:

    /** list of pointers to arithmetic function */
    QList<periodic_function_t *> m_func;

    /** list of function names */
    QStringList m_name;

};

#endif /* _FUNCTIONS_H_ */
