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

#include <KLocalizedString>

#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"

#include "UndoReverseAction.h"

//***************************************************************************
Kwave::UndoReverseAction::UndoReverseAction(
    Kwave::PluginManager &plugin_manager)
    :m_plugin_manager(plugin_manager)
{
}

//***************************************************************************
Kwave::UndoReverseAction::~UndoReverseAction()
{
}

//***************************************************************************
QString Kwave::UndoReverseAction::description()
{
    return i18n("Reverse");
}

//***************************************************************************
qint64 Kwave::UndoReverseAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
qint64 Kwave::UndoReverseAction::redoSize()
{
    return sizeof(*this);
}

//***************************************************************************
bool Kwave::UndoReverseAction::store(Kwave::SignalManager &manager)
{
    Q_UNUSED(manager);
    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoReverseAction::undo(Kwave::SignalManager &manager,
                                                  bool with_redo)
{
    Q_UNUSED(manager);
    m_plugin_manager.enqueueCommand(
	_("nomacro:plugin:execute(reverse,noundo)")
    );
    return (with_redo) ? this : 0;
}

//***************************************************************************
//***************************************************************************
