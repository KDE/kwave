/***************************************************************************
             KwaveApp.h  -  The Kwave main application
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

#ifndef _KWAVE_APP_H_
#define _KWAVE_APP_H_

#include "config.h"
#include <qptrlist.h>
#include <qstringlist.h>

#include <kuniqueapp.h>
#include <kapp.h>

class ClipBoard;
class KURL;
class MemoryManager;
class QString;
class TopWidget;

/**
 * This is the main application class for Kwave. It contains functions
 * for opening and saving files, opening new windows and holds global
 * configuration data.
 */
class KwaveApp :public KUniqueApplication
{
    Q_OBJECT
public:

    KwaveApp();

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /** Destructor. */
    ~KwaveApp();

    /**
     * Returns the name of the application
     */
    QString appName() { return name(); };

    /**
     * Adds a file to the top of the list of recent files. If it was
     * already contained in the list the previous occourence is removed.
     * @param filename path to the file
     */
    void addRecentFile(const QString &filename);

    /**
     * Overwritten for unique application to open a new window.
     */
    virtual int newInstance();

    /**
     * Opens a new toplevel window. If a filename is specified the will
     * will be opened (should be a .wav-file).
     * @param url URL of the file to be loaded, (optional, might be empty)
     * @return true if succeeded
     * @see #closeWindow()
     * @see TopWidget
     */
    bool newWindow(const KURL &url);

    /**
     * Closes a previously opened toplevel window.
     * @param todel the toplevel window that closes down
     * @return true if it was the last toplevel window
     * @see #newWindow()
     * @see TopWidget
     */
    bool closeWindow(TopWidget *todel);

    /** Returns a reference to the list of recent files */
    QStringList recentFiles() {
	return m_recent_files;
    };

    /** Returns a reference to Kwave's clipboard */
    static ClipBoard &clipboard();

    /** Returns a reference to Kwave's memory manager */
    static MemoryManager &memoryManager();

signals:
    /**
     * Will be emitted if the list of recent files has changed. Can
     * be used by toplevel widgets to update their menus.
     */
    void recentFilesChanged();

protected:
    friend class TopWidget;

    bool executeCommand(const QString &command);

protected:
    /**
     * Reads the configuration settings and the list of recent files,
     * opposite of saveConfig().
     * @see #saveConfig()
     */
    void readConfig();

    /**
     * Saves the list of recent files to the kwave configuration file
     * @see KConfig
     */
    void saveRecentFiles();

    /**
     * Saves the current configuration of kwave to the configuration file,
     * opposite of readConfig(). Also saves the list of recent files.
     * @see #saveRecentFiles()
     */
    void saveConfig();
    
    /** Initialises the aRts daemon */
    void initArts();

private:

    /**
     * Local list of recent files. This list will be synchronized
     * with the global list of recent files stored in the libkwave
     * library whenever there is a change.
     */
    QStringList m_recent_files;

    /** list of toplevel widgets */
    QPtrList<TopWidget> m_topwidget_list;

    /** Kwave's clipboard */
    static ClipBoard &m_clipboard;

    /** Kwave's memory manager */
    static MemoryManager &m_memory_manager;

};

#endif // _KWAVE_APP_H_
