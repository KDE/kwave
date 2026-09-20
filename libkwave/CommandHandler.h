/***************************************************************************
    libkwave/CommandHandler.h  -  Interface for a class with executeCommand(...)
                             -------------------
    begin                : 2014-09-22
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

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "config.h"

#include <functional>

#include <QMap>
#include <QString>
#include <QStringList>

#include "libkwave_export.h"

/**
 * convenience macro for defining an entry in a command list
 * @param name an ASCII string with the name of the command
 */
#define KWAVE_COMMAND(name) QLatin1String(name), \
                            [this] (Kwave::Parser &p) -> int

/**
 * convenience macro for defining an entry in a command list, same
 * as KWAVE_COMMAND but without parameter (avoids the compiler
 * warning about the unused parameter "p")
 * @param name an ASCII string with the name of the command
 */
#define KWAVE_COMMAND_NP(name) QLatin1String(name), \
                               [this] (Kwave::Parser &) -> int

namespace Kwave
{
    // forward declaration
    class Parser;

    /**
     * function that handles a command
     * @param parser the parser which provides the commands
     *               and functions to access the parameters
     * @return 0 if succeeded or negative error code if failed
     */
    typedef std::function<int(Kwave::Parser &parser)> Command;

    class LIBKWAVE_EXPORT CommandHandler
    {
    public:

        typedef QMap<QString, Kwave::Command> List;

        /** default constructor */
        CommandHandler() = default;

        /** destructor */
        virtual ~CommandHandler() = default;

        /**
         * Execute a Kwave text command
         * @param command a text command
         * @retval zero if succeeded
         * @retval negative error code if failed
         * @retval ENOSYS is returned if the command is unknown in this
         *                component
         */
        virtual int executeCommand(const QString &command) = 0;

        /**
         * Handle a list of commands
         * @param commands a list of commands and their handlers
         * @param parser the command parser
         * @retval return value of executeCommand if the command was found
         * @retval ENOSYS if the command is unknown
         */
        int handleCommandList(const List &commands,
                              Kwave::Parser &parser);

    };
}

#endif /* COMMAND_HANDLER_H */
//***************************************************************************
//***************************************************************************

