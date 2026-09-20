/***************************************************************************
 libkwave/CommandHandler.cpp - Interface for a class with executeCommand(...)
                             -------------------
    begin                : 2026-09-20
    copyright            : (C) 2014 by Thomas.Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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

#include <libkwave/CommandHandler.h>
#include <libkwave/Parser.h>

//***************************************************************************
int Kwave::CommandHandler::handleCommandList(const List &commands,
                                             Kwave::Parser &parser)
{
    const QString &command = parser.command();

    if (!commands.contains(command))
        return ENOSYS;

    const Kwave::Command &cmd = commands[command];
    return cmd(parser);
}

//***************************************************************************
//***************************************************************************

