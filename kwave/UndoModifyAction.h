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

#include <qarray.h>
#include <qstring.h>

#include "libkwave/Sample.h"
#include "UndoAction.h"

class UndoModifyAction: public UndoAction
{
public:

    /**
     * Constructor.
     * @param track index of the track
     * @param left index of the first modified sample
     * @param right index of the first modified sample
     */
    UndoModifyAction(unsigned int track, unsigned int left,
                    unsigned int right);

    /** Destructor */
    virtual ~UndoModifyAction();

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::size() */
    virtual unsigned int size();

    /**
     * @see UndoAction::store()
     * @todo implementation
     */
    virtual void store();

    /**
     * Exchange samples from the current signal and the internal undo
     * buffer. So this instance will be re-used for redo and so does not
     * require any extra memory for redo.
     * @see UndoAction::undo()
     */
    virtual UndoAction *undo(SignalManager &manager);

private:

    /** index of the modified track */
    unsigned int m_track;

    /** first sample */
    unsigned int m_left;

    /** last sample */
    unsigned int m_right;

    /** buffer with undo data */
    QArray<sample_t> m_buffer;

};
