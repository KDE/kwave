/***************************************************************************
  UndoReverseAction.cpp  -  undo action for the "reverse" effect
			     -------------------
    begin                : Wed Jun 24 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#include <klocale.h>

#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "UndoReverseAction.h"

//***************************************************************************
UndoReverseAction::UndoReverseAction(Kwave::PluginManager &plugin_manager)
    :m_plugin_manager(plugin_manager)
{
}

//***************************************************************************
UndoReverseAction::~UndoReverseAction()
{
}

//***************************************************************************
QString UndoReverseAction::description()
{
    return i18n("Reverse");
}

//***************************************************************************
unsigned int UndoReverseAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
int UndoReverseAction::redoSize()
{
    return sizeof(*this);
}

//***************************************************************************
bool UndoReverseAction::store(SignalManager &manager)
{
    Q_UNUSED(manager);
    return true;
}

//***************************************************************************
UndoAction *UndoReverseAction::undo(SignalManager &manager, bool with_redo)
{
    Q_UNUSED(manager);
    m_plugin_manager.enqueueCommand(
	"nomacro:plugin:execute(reverse,noundo)"
    );
    return (with_redo) ? this : 0;
}

//***************************************************************************
//***************************************************************************
