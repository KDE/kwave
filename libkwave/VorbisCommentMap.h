/*************************************************************************
VorbisCommentMap.h  -  map for translating properties to vorbis comments
                             -------------------
    begin                : Sun May 23 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#ifndef _VORBIS_COMMENT_MAP_H_
#define _VORBIS_COMMENT_MAP_H_

#include "config.h"

#include <QtCore/QMap>
#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/FileInfo.h"

namespace Kwave
{
    class KDE_EXPORT VorbisCommentMap :public QMap<QString, Kwave::FileProperty>
    {
    public:
	/** Default constructor, with initializing */
	VorbisCommentMap();

	/** Destructor */
	virtual ~VorbisCommentMap() {}

	/**
	 * Returns the vorbis comment name of a property or an empty string
	 * if nothing found (reverse lookup).
	 */
	QString findProperty(const Kwave::FileProperty property);

	/** Returns true if the map contains a given property */
	bool containsProperty(const Kwave::FileProperty property);

    };
}
#endif /* _VORBIS_COMMENT_MAP_H_ */

//***************************************************************************
//***************************************************************************
