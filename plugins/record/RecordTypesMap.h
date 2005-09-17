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
#ifndef _RECORD_TYPES_MAP_H_
#define _RECORD_TYPES_MAP_H_

#include "config.h"
#include "libkwave/TypesMap.h"
#include "RecordParams.h"

class RecordTypesMap: public TypesMap<unsigned int, record_method_t>
{
public:
    /** fill function for the map */
    virtual void fill();
};

#endif /* _RECORD_TYPES_MAP_H_ */
