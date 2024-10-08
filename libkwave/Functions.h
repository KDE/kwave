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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "config.h"

#include "libkwave/TypesMap.h"
#include <QString>

namespace Kwave
{

    /**
     * @class Functions
     * Holds a list of simple periodic arithmetic functions. All functions are
     * normed to work within the interval [0...2 Pi]
     */
    class Functions
    {
    public:

        typedef double(periodic_function_t)(double);

        /** Constructor */
        Functions();

        /** Destructor */
        virtual ~Functions();

        /** Returns the number of functions */
        unsigned int count() const;

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
        periodic_function_t &function(unsigned int index) const;

    private:

        class FunctionTypesMap:
            public Kwave::TypesMap< unsigned int, periodic_function_t* >
        {
        public:
            /** Constructor */
            explicit FunctionTypesMap()
                :Kwave::TypesMap<unsigned int, periodic_function_t *>()
            {
                fill();
            }

            /** fills the types map */
            void fill() override;
        };

        /** map of periodic functions */
        FunctionTypesMap m_functions_map;

    };
}

#endif /* FUNCTIONS_H */

//***************************************************************************
//***************************************************************************
