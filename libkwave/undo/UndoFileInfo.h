/***************************************************************************
         UndoFileInfo.h  -  Undo action for file info
			     -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _UNDO_FILE_INFO_H_
#define _UNDO_FILE_INFO_H_

#include "config.h"

#include <QString>

#include "libkwave/FileInfo.h"
#include "libkwave/undo/UndoAction.h"

class SignalManager;

/**
 * This Undo action simply stores the current file info.
 */
class UndoFileInfo: public UndoAction
{

public:

    /**
     * Constructor.
     * @param manager reference to the SignalManager
     */
    UndoFileInfo(SignalManager &manager);

    /** virtual destructor */
    virtual ~UndoFileInfo();

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /** @see UndoAction::store() */
    virtual bool store(SignalManager &manager);

    /** @see UndoAction::undo() */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

protected:

    /**
     * Calculates the size in bytes needed for storing a FileInfo, but
     * not the FileInfo object itself.
     */
    unsigned int infoSize(const FileInfo &info);

private:

    /** signal manager, needed for calculating sizes */
    SignalManager &m_manager;

    /** Array with indices of selected tracks. */
    FileInfo m_info;

};

#endif /* _UNDO_FILE_INFO_H_ */
