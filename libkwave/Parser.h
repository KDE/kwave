/***************************************************************************
                Parser.h -  parser for Kwave's internal commands
                             -------------------
    begin                : Sat Feb  3 2001
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

#ifndef PARSER_H
#define PARSER_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "libkwave/Sample.h" // for sample_index_t

//*****************************************************************************
namespace Kwave
{
    class LIBKWAVE_EXPORT Parser
    {
    public:
        /**
         * Constructor. Parses the given string into an optional
         * command part and a list of parameters.
         */
        Parser(const QString &init);

        /** Destructor. */
        virtual ~Parser();

        /** Returns the command part of the line */
        inline QString command() {
            return m_command;
        }

        /** Returns the complete list of parameters */
        inline const QStringList &params() {
            return m_param;
        }

        /** Returns true if the parameter list is not empty. */
        inline bool hasParams() {
            return (m_param.count() != 0);
        }

        /** Returns true if a list of commands was parsed */
        inline bool hasMultipleCommands() {
            return (m_commands.count() > 1);
        }

        /** Returns the list of commands */
        inline QStringList commandList() { return m_commands; }

        /** Returns true if the end of the parameter list has been reached. */
        inline bool isDone () const {
            return (static_cast<int>(m_current) >= m_param.count());
        }

        /** Returns the number of parameters. */
        inline unsigned int count() const {
            return static_cast<unsigned int>(m_param.count());
        }

        /**
         * Returns the first parameter and sets the current
         * position to the next.
         */
        const QString &firstParam();

        /**
         * Returns the next parameter and increases the current
         * position if not already at the end of the parameter
         * list. If the end of the parameter list has been
         * reached, the return value will be a zero-length string.
         */
        const QString &nextParam();

        /** Returns all remaining parameters in a string list */
        QStringList remainingParams();

        /**
         * Skips a parameter and advances the current position
         * by one if the end has not already been reached.
         */
        void skipParam();

        /**
         * Gets the next parameter through calling nextParam() and
         * interpretes it as a "bool" value. It will recognize the
         * strings "true" and "false" (not case-sensitive) and
         * numeric values (true means not zero). On errors the
         * return value will be false.
         */
        bool toBool();

        /**
         * Gets the next parameter through calling nextParam() and
         * interpretes it as an "int" value. On errors the
         * return value will be zero.
         */
        int toInt();

        /**
         * Gets the next parameter through calling nextParam() and
         * interpretes it as an "unsigned int" value. On errors the
         * return value will be zero.
         */
        unsigned int toUInt();

        /**
         * Gets the next parameter through calling nextParam() and
         * interpretes it as a "sample_index_t" value. On errors the
         * return value will be zero.
         */
        sample_index_t toSampleIndex();

        /**
         * Gets the next parameter through calling nextParam() and
         * interpretes it as a "double" value. On errors the
         * return value will be zero.
         */
        double toDouble();

        /**
         * Escapes all characters that might be critical when parsing
         *
         * @param text a unicode text to escape
         * @return text with certain characters escaped
         */
        static QString escape(const QString &text);

        /**
         * Escape all parts of a string that might be dangerous for using
         * within a file name, like "/" and similar.
         *
         * @param text a unicode text to escape
         * @return escaped string, which should be safe to use in a file name
         */
        static QString escapeForFileName(const QString &text);

        /**
         * Un-escapes all characters in a string previously escaped with
         * escape()
         *
         * @param text a unicode text to un-escape
         * @return the original text without escaped characters
         */
        static QString unescape(const QString &text);

        /**
         * Converts a string command into a URL suitable for passing to
         * Kwave as command line parameter
         * @param command the string command to encode as URL
         * @return a QUrl with the kwave:// scheme
         */
        static QUrl toUrl(const QString &command);

        /**
         * Converts a kwave:// URL into a string command
         * @param url a URL using the kwave:// scheme
         * @return a Kwave string command parsed from the URL
         */
        static QString fromUrl(const QUrl &url);

    protected:

        /** Splits a line into a list of commands */
        QStringList splitCommands(QString &line);

    private:
        /** the command part of the line */
        QString m_command;

        /** list of parsed parameters */
        QStringList m_param;

        /** number of the "current" parameter */
        unsigned int m_current;

        /** if it has multiple commands, the list of command strings */
        QStringList m_commands;

    };
}

#endif /* PARSER_H */

//***************************************************************************
//***************************************************************************
