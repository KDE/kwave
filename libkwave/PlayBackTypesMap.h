/***************************************************************************
     PlayBackTypesMap.h  -  map for playback methods
                             -------------------
    begin                : Sat Feb 05 2005
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
#ifndef PLAY_BACK_TYPES_MAP_H
#define PLAY_BACK_TYPES_MAP_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>

#include "libkwave/PlayBackParam.h"
#include "libkwave/TypesMap.h"

namespace Kwave
{
    class LIBKWAVE_EXPORT PlayBackTypesMap
        :public Kwave::TypesMap<unsigned int, Kwave::playback_method_t>
    {
    public:
        /** Constructor */
        explicit PlayBackTypesMap()
            :Kwave::TypesMap<unsigned int, Kwave::playback_method_t>()
        {
            fill();
        }

        /** Destructor */
        ~PlayBackTypesMap() override {}

        /** fill function for the map */
        void fill() override;
    };
}

#endif /* PLAY_BACK_TYPES_MAP_H */

//***************************************************************************
//***************************************************************************
