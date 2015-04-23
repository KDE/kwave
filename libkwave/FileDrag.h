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

#ifndef FILE_DRAG_H
#define FILE_DRAG_H

#include "config.h"

#include <QtCore/QMimeData>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "libkwave/CodecManager.h"
#include "libkwave/String.h"

namespace Kwave
{
    namespace FileDrag
    {
	static inline bool canDecode(const QMimeData *source) {
	    if (!source) return false;

	    if (source->hasUrls()) {
		// dropping URLs
		foreach (QUrl url, source->urls()) {
		    QString filename = url.toLocalFile();
		    QString mimetype =
			Kwave::CodecManager::whatContains(filename);
		    if (Kwave::CodecManager::canDecode(mimetype)) {
			return true;
		    }
		}
	    }

	    foreach (QString format, source->formats()) {
		// dropping known mime type
		if (Kwave::CodecManager::canDecode(format)) {
		    qDebug("Kwave::FileDrag::canDecode(%s)",
		            DBG(QString(format)));
		    return true;
		}
	    }
	    return false;
	}
    }
}

#endif /* FILE_DRAG_H */

//***************************************************************************
//***************************************************************************
