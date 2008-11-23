/***************************************************************************
     UndoInsertAction.h  -  UndoAction for insertion of a range of samples
			     -------------------
    begin                : Jun 14 2001
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

#ifndef _UNDO_INSERT_ACTION_H_
#define _UNDO_INSERT_ACTION_H_

#include "config.h"

#include <QList>
#include <QObject>
#include <QString>

#include "libkwave/undo/UndoAction.h"

class QWidget;
class SignalManager;

class UndoInsertAction: public QObject, public UndoAction
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent_widget the widget used as parent for displaying
     *                      error messages
     * @param track_list list of affected tracks
     * @param offset index of the first inserted sample
     * @param length number of inserted samples
     */
    UndoInsertAction(QWidget *parent_widget,
                     const QList<unsigned int> &track_list,
                     unsigned int offset,
                     unsigned int length);

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /**
     * @see UndoAction::store()
     */
    virtual bool store(SignalManager &manager);

    /**
     * Removes samples from the track.
     * @see UndoAction::undo()
     */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

public slots:

    /**
     * Can be connected to a SampleWriter's <c>sigSamplesWritten</c> signal
     * if the writer has been opened in insert or append mode. In these
     * cases the undo action's length only is determined when the writer
     * gets closed.
     * @see SampleWriter::sigSamplesWritten
     */
    void setLength(unsigned int length);

protected:

    /** parent widget for showing error messages */
    QWidget *m_parent_widget;

    /** list of affected tracks */
    QList<unsigned int> m_track_list;

    /** first sample */
    unsigned int m_offset;

    /** number of samples */
    unsigned int m_length;

};

#endif /* _UNDO_INSERT_ACTION_H_ */
