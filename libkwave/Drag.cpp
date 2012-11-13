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

#include "libkwave/CodecManager.h"
#include "libkwave/Drag.h"
#include "libkwave/MimeData.h"
#include "libkwave/MultiTrackReader.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave" // ; codec=001"

//***************************************************************************
Kwave::Drag::Drag(QWidget *dragSource)
    :QDrag(dragSource)
{
}

//***************************************************************************
Kwave::Drag::~Drag()
{
}

//***************************************************************************
bool Kwave::Drag::canDecode(const QMimeData *data)
{
    if (!data) return false;
    foreach (QString format, data->formats())
	if (Kwave::CodecManager::canDecode(format)) return true;
    return false;
}

//***************************************************************************
bool Kwave::Drag::encode(QWidget *widget, Kwave::MultiTrackReader &src,
                         const Kwave::MetaDataList &meta_data)
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
    if (!mime_data->encode(widget, src, meta_data)) {
	delete mime_data;
	return false;
    }

    // use it for the drag container
    setMimeData(mime_data);
    return true;
}

//***************************************************************************
unsigned int Kwave::Drag::decode(QWidget *widget, const QMimeData *e,
                                 Kwave::SignalManager &sig,
                                 sample_index_t pos)
{
    return Kwave::MimeData::decode(widget, e, sig, pos);
}

//***************************************************************************
#include "Drag.moc"
//***************************************************************************
//***************************************************************************
