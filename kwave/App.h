/***************************************************************************
                  App.h  -  The Kwave main application
                             -------------------
    begin                : Wed Feb 28 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef KWAVE_APP_H
#define KWAVE_APP_H

#include "config.h"

#include <QApplication>
#include <QList>
#include <QObject>
#include <QPair>
#include <QStringList>

class QCommandLineParser;
class QString;
class QUrl;

namespace Kwave
{

    class TopWidget;

    /**
     * This is the main application class for Kwave. It contains functions
     * for opening and saving files, opening new windows and holds global
     * configuration data.
     */
    class App :public QApplication
    {
        Q_OBJECT
    public:

        typedef enum {
            GUI_SDI, /**< single document interface (SDI)          */
            GUI_MDI, /**< multi document interface (MDI)           */
            GUI_TAB  /**< tabbed interface                         */
        /*  GUI_IDE       integrated development environment (IDE) */
        } GuiType;

        /**
         * pair of file name and instance
         * @see #openFiles()
         */
        typedef QPair<QString,int> FileAndInstance;

        /**
         * Constructor
         * @param argc number of cmdline args, must be >= 1
         * @param argv list of cmdline args, must be static
         */
        App(int &argc, char **argv);

        /**
         * process the command line settings, after setting up the application,
         * command line parser and about data
         * @param cmdline command line parser
         */
        void processCmdline(QCommandLineParser *cmdline);

        /**
         * Returns true if this instance was successfully initialized, or
         * false if something went wrong during initialization.
         */
        virtual bool isOK() const;

        /** Destructor. */
        ~App() override;

        /**
         * Adds a file to the top of the list of recent files. If it was
         * already contained in the list the previous occurrence is removed.
         * @param filename path to the file
         */
        void addRecentFile(const QString &filename);

        /**
         * Opens a new toplevel window. If a filename is specified the file
         * will be opened.
         * @param url URL of the file to be loaded, (optional, might be empty)
         * @retval 0 if succeeded
         * @retval ECANCELED if the application should shut down (e.g. when
         *                   a script was loaded and the script has called
         *                   quit() )
         * @see #toplevelWindowHasClosed()
         * @see TopWidget
         */
        int newWindow(const QUrl &url);

        /**
         * Called when a toplevel window has closed.
         * @param todel the toplevel window that has closed
         * @return true if it was the last toplevel window
         */
        bool toplevelWindowHasClosed(Kwave::TopWidget *todel);

        /** Returns a reference to the list of recent files */
        inline QStringList recentFiles() const { return m_recent_files; }

        /** Returns a list of currently opened files and their instance */
        QList<FileAndInstance> openFiles() const;

        /** returns the GUI type (e.g. SDI, MDI etc.) */
        GuiType guiType() const { return m_gui_type; }

        /**
         * Switches the GUI type to a new style, using the current toplevel
         * widget as start.
         * @param top the current toplevel widget
         * @param new_type the new GUI type
         */
        void switchGuiType(Kwave::TopWidget *top, GuiType new_type);

        /** Returns the command line parameters passed to the application */
        inline const QCommandLineParser *cmdline() const { return m_cmdline; }

    signals:
        /**
         * Will be emitted if the list of recent files has changed. Can
         * be used by toplevel widgets to update their menus.
         */
        void recentFilesChanged();

    public slots:

        /**
         * Connected to the DBus service to open a new window.
         * @retval 0 if the new instance was successfully created and the
         *           application should run
         * @retval ECANCELED if the application should shut down (e.g. when
         *                   a script was loaded and the script has called
         *                   quit() )
         */
        int newInstance(const QStringList &args, const QString &dir);

    protected:
        friend class Kwave::TopWidget;

        /**
         * process an application global Kwave text command
         * @param command the text command to handle
         * @retval 0 if succeeded
         * @retval ENOSYS if the command is unknown
         * @retval -EIO if failed
         */
        int executeCommand(const QString &command);

    private:

        /**
         * Reads the configuration settings and the list of recent files
         */
        void readConfig();

        /**
         * Saves the list of recent files to the kwave configuration file
         * @see KConfig
         */
        void saveRecentFiles();

    private:

        /** reference to a (static) command line parser */
        QCommandLineParser *m_cmdline;

        /**
         * Local list of recent files. This list will be synchronized
         * with the global list of recent files stored in the libkwave
         * library whenever there is a change.
         */
        QStringList m_recent_files;

        /** list of toplevel widgets */
        QList<Kwave::TopWidget *> m_top_widgets;

        /** the GUI type, e.g. SDI or MDI */
        GuiType m_gui_type;
    };
}

#endif // KWAVE_APP_H

//***************************************************************************
//***************************************************************************
