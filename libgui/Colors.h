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

#ifndef _COLORS_H_
#define _COLORS_H_

#include "config.h"

#include <QtGui/QColor>

#include <kdemacros.h>

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
	static KDE_EXPORT ColorSet Normal;

	/** color set for disabled signal */
	static KDE_EXPORT ColorSet Disabled;

    };
}

#endif /* _COLORS_H_ */

//***************************************************************************
//***************************************************************************
