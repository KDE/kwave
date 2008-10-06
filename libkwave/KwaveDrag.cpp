/***************************************************************************
          KwaveDrag.cpp  -  Drag&Drop container for Kwave's audio data
			     -------------------
    begin                : Jan 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <QMimeData>
#include <QMimeSource>

#include "libkwave/CodecManager.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/KwaveMimeData.h"
#include "libkwave/MultiTrackReader.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave" // ; codec=001"

//***************************************************************************
KwaveDrag::KwaveDrag(QWidget *dragSource)
    :QDrag(dragSource)
{
}

//***************************************************************************
KwaveDrag::~KwaveDrag()
{
}

//***************************************************************************
const char *KwaveDrag::format(int i) const
{
    // see RFC 2361 for other codecs
    switch (i) {
	case 0: return WAVE_FORMAT_PCM;
    }
    return 0;
}

//***************************************************************************
bool KwaveDrag::canDecode(const QMimeData *data)
{
    if (!data) return false;
    foreach (QString format, data->formats())
	if (CodecManager::canDecode(format)) return true;
    return false;
}

//***************************************************************************
bool KwaveDrag::encode(QWidget *widget, MultiTrackReader &src, FileInfo &info)
{
    Q_ASSERT(src.tracks());
    if (!src.tracks()) return false;
    Q_ASSERT(src[0]);
    if (!src[0]) return false;

    // create a mime data container
    Kwave::MimeData *mime_data = new Kwave::MimeData;
    Q_ASSERT(mime_data);
    if (!mime_data) return false;

    // encode into the mime data
    if (!mime_data->encode(widget, src, info)) {
	delete mime_data;
	return false;
    }

    // use it for the drag container
    setMimeData(mime_data);
    return true;
}

//***************************************************************************
unsigned int KwaveDrag::decode(QWidget *widget, const QMimeSource *e,
                               SignalManager &sig, unsigned int pos)
{
    return Kwave::MimeData::decode(widget, e, sig, pos);
}

//***************************************************************************
#include "KwaveDrag.moc"
//***************************************************************************
//***************************************************************************
