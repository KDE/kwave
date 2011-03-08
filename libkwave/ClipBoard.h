/***************************************************************************
            ClipBoard.h  -  the Kwave clipboard
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

#ifndef _CLIP_BOARD_H_
#define _CLIP_BOARD_H_

#include "config.h"

#include <QObject>
#include <QClipboard>

#include <kdemacros.h>

#include "libkwave/Sample.h"

class QWidget;
class SignalManager;
class Track;

/**
 * Implements a global clipboard for Kwave. It supports only the three
 * simple operations <c>put</c>, <c>get</c> and <c>clear</c>.
 *
 */
class KDE_EXPORT ClipBoard: public QObject
{
    Q_OBJECT
public:

    /** Constructor. */
    ClipBoard();

    /** Destructor */
    virtual ~ClipBoard();

    /** returns the static instance of the clipboard */
    static ClipBoard &instance();

    /**
     * Discards the current content of the clipboard and fills
     * it with a selected range of samples from a set of tracks.
     * @param widget the widget used as parent for displaying
     *               error messages
     * @param signal_manager the SignalManager with the tracks to read from
     * @param track_list a list of indices of tracks for reading
     * @param offset first sample to copy
     * @param length number of samples
     */
    void copy(QWidget *widget, SignalManager &signal_manager,
              const QList<unsigned int> &track_list,
              sample_index_t offset, sample_index_t length);

    /**
     * Transfers all stored data from the clipboard into a SignalManager.
     *
     * @param widget the widget used as parent for displaying
     *               error messages
     * @param signal_manager the SignalManager with the tracks to write to
     * @param offset sample position where to paste
     * @param length length of the current selection in samples
     * @return true if successful, false if failed
     */
    bool paste(QWidget *widget, SignalManager &signal_manager,
               sample_index_t offset, sample_index_t length);

    /**
     * Clears the internal buffers. The clipboard will be empty after this
     * function returns and the sample rate will be set to zero.
     */
    void clear();

    /**
     * Returns true if the buffer is empty.
     */
    bool isEmpty();

signals:

    /**
     * Emitted whenever the clipboard has changed from empty
     * to non-empty (with decodeable data) or vice versa.
     * @param data_available if true: contains decodeable data,
     *                       if false: nothing decodeable or empty
     */
    void clipboardChanged(bool data_available);

public slots:

    /**
     * emits clipboardChanged() whenever the clipboard has changed
     * from empty to filled (with decodeable data) or vice versa
     * (connected to the Qt clipboard)
     */
    void slotChanged(QClipboard::Mode mode);

private:

    /** number of tracks of the content */
    unsigned int m_tracks;

};

#endif /* _CLIP_BOARD_H_ */
