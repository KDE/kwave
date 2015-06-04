/***************************************************************************
          ClipBoard.cpp  -  the Kwave clipboard
			     -------------------
    begin                : Tue Jun 26, 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <new>

#include <QtGui/QClipboard>
#include <QtCore/QList>
#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <QtWidgets/QApplication>

#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/MimeData.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/SignalManager.h"

/** static instance of Kwave's clipboard */
static Kwave::ClipBoard g_clipboard;

//***************************************************************************
Kwave::ClipBoard &Kwave::ClipBoard::instance()
{
    return g_clipboard;
}

//***************************************************************************
Kwave::ClipBoard::ClipBoard()
    :m_tracks(0)
{
}

//***************************************************************************
Kwave::ClipBoard::~ClipBoard()
{
    // clear() must have been before, e.g. in the application's destructor !
}

//***************************************************************************
void Kwave::ClipBoard::slotChanged(QClipboard::Mode mode)
{
    if (mode != QClipboard::Clipboard) return;
    bool data_available = !isEmpty();

//     qDebug("=> ClipBoard::clipboardChanged(%s)",
// 	data_available ? "FULL" : "EMPTY");

    emit clipboardChanged(data_available);
}

//***************************************************************************
void Kwave::ClipBoard::copy(QWidget *widget,
                            Kwave::SignalManager &signal_manager,
                            const QList<unsigned int> &track_list,
                            sample_index_t offset, sample_index_t length)
{
    // break if nothing to do
    if (!length || !track_list.count()) return;

    // get a buffer, implemented as a KwaveMimeData container
    Kwave::MimeData *buffer = new(std::nothrow) Kwave::MimeData();
    Q_ASSERT(buffer);
    if (!buffer) return;

    // encode into the mime data container
    Kwave::MultiTrackReader src(Kwave::SinglePassForward, signal_manager,
	track_list, offset, offset + length - 1);

    if (!buffer->encode(widget, src, signal_manager.metaData())) {
	// encoding failed, reset to empty
	buffer->clear();
	delete buffer;
	return;
    }

    // give the buffer to the KDE clipboard
    qApp->processEvents();
    QApplication::clipboard()->setMimeData(buffer, QClipboard::Clipboard);
}

//***************************************************************************
bool Kwave::ClipBoard::paste(QWidget *widget,
                             Kwave::SignalManager &signal_manager,
                             sample_index_t offset, sample_index_t length)
{
    Q_ASSERT(!isEmpty());
    if (isEmpty()) return false; // clipboard is empty ?

    // delete the current selection (with undo)
    if (length <= 1) length = 0; // do not paste single samples !
    if (length && !signal_manager.deleteRange(offset, length))
	return false;

    sample_index_t decoded_samples = Kwave::MimeData::decode(
	widget,
	QApplication::clipboard()->mimeData(QClipboard::Clipboard),
	signal_manager,
	offset);
    if (!decoded_samples) return false;

    // set the selection to the inserted range
    signal_manager.selectRange(offset, decoded_samples);
    return true;
}

//***************************************************************************
void Kwave::ClipBoard::clear()
{
    QApplication::clipboard()->clear();
}

//***************************************************************************
bool Kwave::ClipBoard::isEmpty()
{
    const QMimeData *mime_data =
	QApplication::clipboard()->mimeData(QClipboard::Clipboard);

    // no mime data -> empty
    if (!mime_data) return true;

    // there is a format that we can decode -> not empty
    foreach (const QString &format, mime_data->formats())
	if (Kwave::CodecManager::canDecode(format)) return false;

    // nothing to decode -> empty
    return true;
}

//***************************************************************************
//***************************************************************************
