/***************************************************************************
      CompressionType.h  -  Map for all known compression types
                             -------------------
    begin                : Mon Jul 29 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _COMPRESSION_TYPE_H_
#define _COMPRESSION_TYPE_H_

#include "config.h"
#include "audiofile.h"
#include "TypesMap.h"

class CompressionType: public TypesMap<int, int>
{
public:
    /** Constructor */
    CompressionType();

    /** Destructor */
    virtual ~CompressionType();

    /** fills the list */
    virtual void fill();

};

#endif /* _COMPRESSION_TYPE_H_ */
