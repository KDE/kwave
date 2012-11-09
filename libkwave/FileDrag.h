/***************************************************************************
             FileDrag.h  -  check if Kwave can handle a mime type per drag&drop
			     -------------------
    begin                : Sat Feb 26 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#ifndef _FILE_DRAG_H_
#define _FILE_DRAG_H_

#include "config.h"

#include <QMimeData>
#include <QString>
#include <QUrl>

#include "libkwave/CodecManager.h"

namespace KwaveFileDrag
{
    static inline bool canDecode(const QMimeData *source) {
	if (!source) return false;

	if (source->hasUrls()) {
	    // dropping URLs
	    foreach (QUrl url, source->urls()) {
		QString filename = url.toLocalFile();
		QString mimetype = CodecManager::whatContains(filename);
		if (CodecManager::canDecode(mimetype)) {
		    return true;
		}
	    }
	}

	foreach (QString format, source->formats()) {
	    // dropping known mime type
	    if (CodecManager::canDecode(format)) {
		qDebug("KwaveFileDrag::canDecode(%s)",
		       format.toLocal8Bit().data());
		return true;
	    }
	}
	return false;
    }
}

#endif /* _FILE_DRAG_H_ */

//***************************************************************************
//***************************************************************************
