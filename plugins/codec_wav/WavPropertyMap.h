/*************************************************************************
       WavPropertyMap.h  -  map for translating properties to chunk names
                             -------------------
    begin                : Sat Jul 06 2002
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

#ifndef _WAV_PROPERTY_MAP_H_
#define _WAV_PROPERTY_MAP_H_

#include "config.h"
#include <qmap.h>
#include <qstring.h>
#include "libkwave/FileInfo.h"

class WavPropertyMap: public QMap<QCString, FileProperty>
{
public:
    /** Default constructor, with initializing */
    WavPropertyMap();
};

#endif /* _WAV_PROPERTY_MAP_H_ */
