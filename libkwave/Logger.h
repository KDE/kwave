/***************************************************************************
               Logger.h  -  Kwave log file handling
                             -------------------
    begin                : Sat May 17 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef KWAVE_LOGGER_H
#define KWAVE_LOGGER_H

#include "config.h"

#include <QtGlobal>

class QFile;
class QObject;
class QString;

namespace Kwave
{

    class FileContext;
    class TopWidget;

    /**
     * This is the main application class for Kwave. It contains functions
     * for opening and saving files, opening new windows and holds global
     * configuration data.
     */
    class Logger
    {
    public:

        typedef enum
        {
            Debug = 0,  /**< debug message                               */
            Info,       /**< info                                        */
            Warning,    /**< warning, something might have went wrong    */
            Error,      /**< error, something failed, but is recoverable */
            Fatal       /**< fatal error, not recoverable, have to exit  */
        } LogLevel;

        /**
         * Constructor
         */
        Logger();

        /** Destructor. */
        virtual ~Logger();

        /**
         * open a log file for writing
         * @note must only be called once, if there already is a log file,
         *       then the old log file will be closed and the new one will
         *       be opened.
         * @param filename name of the log file
         * @return true if succeed, false if failed
         */
        static bool Q_DECL_EXPORT open(const QString &filename);

        /**
         * log a message to the log file
         *
         * @param sender pointer to the sender of the message, must be
         *               derived from QObject
         * @param level the log level / severity
         * @param msg the message to log
         */
        static void Q_DECL_EXPORT log(const QObject *sender,
                                      LogLevel level,
                                      const QString &msg);

    private:

        /** log file that receives the log messages (Null is allowed) */
        static QFile *m_logfile;
    };
}

#endif // _KWAVE_LOGGER_H_

//***************************************************************************
//***************************************************************************
