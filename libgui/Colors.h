/***************************************************************************
               Colors.h  -  Kwave color sets
                             -------------------
    begin                : Sat Oct 05 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#ifndef COLORS_H
#define COLORS_H

#include "config.h"

#include <QtGlobal>
#include <QColor>

namespace Kwave
{
    class Colors
    {
    public:
        typedef struct {
            /** Background color */
            QColor background;

            /** Color for samples */
            QColor sample;

            /** Color for interpolated samples */
            QColor interpolated;

            /** Color for the zero line, used areas */
            QColor zero;

            /** Color of the zero line, unused areas */
            QColor zero_unused;
        } ColorSet;

        /** color set for normal signal */
        static Q_DECL_EXPORT ColorSet Normal;

        /** color set for disabled signal */
        static Q_DECL_EXPORT ColorSet Disabled;

    };
}

#endif /* COLORS_H */

//***************************************************************************
//***************************************************************************
