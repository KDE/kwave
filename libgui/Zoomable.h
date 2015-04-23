/***************************************************************************
    libgui/Zoomable.h  -  Interface for a GUI element that supports zooming
			     -------------------
    begin                : 2014-09-21
    copyright            : (C) 2014 by Thomas.Eschenbacher
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

#ifndef ZOOMABLE_H
#define ZOOMABLE_H

#include "config.h"

#include "libkwave/Sample.h"

namespace Kwave
{
    class Zoomable
    {
    public:

	/** default constructor */
	Zoomable() {}

	/** destructor */
	virtual ~Zoomable() {}

	/**
	 * Returns the width of the visible area
	 * @return width of visible area in pixels
	 */
	virtual int visibleWidth() const = 0;

	/** Returns the width of the current view in samples */
	virtual sample_index_t visibleSamples() const = 0;

	/** Returns the current zoom factor [samples/pixel] */
	virtual double zoom() const = 0;

	/**
	 * Set a new zoom factor [samples/pixel]
	 * @param factor new zoom value
	 */
	virtual void setZoom(double factor) = 0;

	/**
	 * Scrolls the display so that the given position gets visible,
	 * centered within the display if possible.
	 */
	virtual void scrollTo(sample_index_t pos) = 0;

    };
}

#endif /* ZOOMABLE_H */
//***************************************************************************
//***************************************************************************
