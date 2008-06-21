/***************************************************************************
       UndoFileInfo.cpp  -  Undo action for file info
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

#include "config.h"

#include "libkwave/SignalManager.h"
#include "libkwave/UndoFileInfo.h"

//***************************************************************************
UndoFileInfo::UndoFileInfo(SignalManager &manager)
    :UndoAction(), m_manager(manager), m_info()
{
}

//***************************************************************************
UndoFileInfo::~UndoFileInfo()
{
}

//***************************************************************************
QString UndoFileInfo::description()
{
    return i18n("modify file information");
}

//***************************************************************************
unsigned int UndoFileInfo::infoSize(const FileInfo &info)
{
    unsigned int size = 0;
    QMap<FileProperty,QVariant>::ConstIterator it;
    for (it=info.properties().begin(); it!=info.properties().end(); ++it) {
	size += 2*sizeof(void*); // just a guess
	size += sizeof(it.key());
	size += sizeof(it.value());
    }
    return size;
}

//***************************************************************************
unsigned int UndoFileInfo::undoSize()
{
    return sizeof(*this) + infoSize(m_manager.fileInfo());
}

//***************************************************************************
int UndoFileInfo::redoSize()
{
    return sizeof(*this) + infoSize(m_info);
}

//***************************************************************************
bool UndoFileInfo::store(SignalManager &manager)
{
    m_info = manager.fileInfo();
    return true;
}

//***************************************************************************
UndoAction *UndoFileInfo::undo(SignalManager &manager, bool with_redo)
{
    UndoFileInfo *redo = with_redo ? new UndoFileInfo(manager) : 0;
    if (redo) redo->store(manager);

    manager.setFileInfo(m_info, false);

    return redo;
}

//***************************************************************************
//***************************************************************************
