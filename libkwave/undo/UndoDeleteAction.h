/***************************************************************************
     UndoDeleteAction.h  -  UndoAction for deletion of a range of samples
			     -------------------
    begin                : Jun 08 2001
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

#ifndef _UNDO_DELETE_ACTION_H_
#define _UNDO_DELETE_ACTION_H_

#include "config.h"

#include <QList>
#include <QString>

#include "libkwave/KwaveMimeData.h"
#include "libkwave/undo/UndoAction.h"

class QWidget;
class SignalManager;

class UndoDeleteAction: public UndoAction
{
public:

    /**
     * Constructor.
     * @param parent_widget the widget used as parent for displaying
     *                      error messages
     * @param track_list list of affected tracks
     * @param offset index of the first deleted sample
     * @param length number of samples to delete
     */
    UndoDeleteAction(QWidget *parent_widget,
                     const QList<unsigned int> &track_list,
                     unsigned int offset, unsigned int length);

    /** Destructor */
    virtual ~UndoDeleteAction();

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /**
     * Stores the data needed for undo.
     * @param manager the SignalManager for modifying the signal
     * @note this is the second step, after size() has been called
     * @return true if successful, false if failed (e.g. out of memory)
     */
    virtual bool store(SignalManager &manager);

    /**
     * Copies the samples to be deleted to the internal buffer.
     * @see UndoAction::undo()
     */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

    /** dump, for debugging purposes */
    virtual void dump(const QString &indent);

private:

    /** parent widget for showing error messages */
    QWidget *m_parent_widget;

    /** list of affected tracks */
    QList<unsigned int> m_track_list;

    /** first deleted sample */
    unsigned int m_offset;

    /** number of deleted samples */
    unsigned int m_length;

    /**
     * Kwave::MimeData container that holds the whole range of samples
     * and deleted labels
     */
    Kwave::MimeData m_mime_data;

    /** memory needed for undo */
    unsigned int m_undo_size;

};

#endif /* _UNDO_DELETE_ACTION_H_ */
