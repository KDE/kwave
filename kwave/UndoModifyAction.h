/***************************************************************************
     UndoModifyAction.h  -  UndoAction for modifications on samples
			     -------------------
    begin                : May 25 2001
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

#ifndef _UNDO_MODIFY_ACTION_H_
#define _UNDO_MODIFY_ACTION_H_

#include "config.h"
#include <qmemarray.h>
#include <qstring.h>

#include "libkwave/Sample.h"
#include "UndoAction.h"

class UndoModifyAction: public UndoAction
{
public:

    /**
     * Constructor.
     * @param track index of the track
     * @param offset index of the first modified sample
     * @param length number of samples
     */
    UndoModifyAction(unsigned int track, unsigned int offset,
                     unsigned int length);

    /** Destructor */
    virtual ~UndoModifyAction();

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize() { return 0; } ;

    /**
     * @see UndoAction::store()
     */
    virtual void store(SignalManager &manager);

    /**
     * Exchange samples from the current signal and the internal undo
     * buffer. So this instance will be re-used for redo and so does not
     * require any extra memory for redo.
     * @see UndoAction::undo()
     */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

protected:

    /** index of the modified track */
    unsigned int m_track;

    /** first sample */
    unsigned int m_offset;

    /** number of samples */
    unsigned int m_length;

    /** buffer with undo data */
    QMemArray<sample_t> m_buffer;

};

#endif /* _UNDO_MODIFY_ACTION_H_ */
