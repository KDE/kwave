/***************************************************************************
       RecordTypesMap.h  -  map for record methods
                             -------------------
    begin                : Sun Jul 31 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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
#ifndef RECORD_TYPES_MAP_H
#define RECORD_TYPES_MAP_H

#include "RecordParams.h"
#include "config.h"
#include "libkwave/TypesMap.h"

namespace Kwave
{
    class RecordTypesMap
        :public Kwave::TypesMap<unsigned int, Kwave::record_method_t>
    {
    public:
        /** Constructor */
        explicit RecordTypesMap()
            :Kwave::TypesMap<unsigned int, Kwave::record_method_t>()
        {
            fill();
        }

        /** Destructor */
        virtual ~RecordTypesMap() override {}

        /** fill function for the map */
        virtual void fill() override;
    };
}

#endif /* RECORD_TYPES_MAP_H */

//***************************************************************************
//***************************************************************************
