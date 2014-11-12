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

#ifndef _KWAVE_APP_H_
#define _KWAVE_APP_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QStringList>

#include <kuniqueapplication.h>
#include <kapplication.h>

class QString;
class KUrl;

namespace Kwave
{

    class FileContext;
    class TopWidget;

    /**
     * This is the main application class for Kwave. It contains functions
     * for opening and saving files, opening new windows and holds global
     * configuration data.
     */
    class App :public KUniqueApplication
    {
	Q_OBJECT
    public:

	typedef enum {
	    GUI_SDI, /**< single document interface (SDI)          */
	    GUI_MDI, /**< multi document interface (MDI)           */
	    GUI_TAB  /**< tabbed interface                         */
	/*  GUI_IDE       integrated development environment (IDE) */
	} GuiType;

	/** Constructor */
	App();

	/**
	 * Returns true if this instance was successfully initialized, or
	 * false if something went wrong during initialization.
	 */
	virtual bool isOK() const;

	/** Destructor. */
	virtual ~App();

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
	 * Opens a new toplevel window. If a filename is specified the file
	 * will be opened.
	 * @param url URL of the file to be loaded, (optional, might be empty)
	 * @return true if succeeded, false if failed
	 * @see #closeWindow()
	 * @see TopWidget
	 */
	bool newWindow(const KUrl &url);

	/**
	 * Closes a previously opened toplevel window.
	 * @param todel the toplevel window that closes down
	 * @return true if it was the last toplevel window
	 * @see #newWindow()
	 * @see TopWidget
	 */
	bool closeWindow(Kwave::TopWidget *todel);

	/** Returns a reference to the list of recent files */
	inline QStringList recentFiles() const { return m_recent_files; }

	/** returns the GUI type (e.g. SDI, MDI etc.) */
	GuiType guiType() const { return m_gui_type; }

    signals:
	/**
	 * Will be emitted if the list of recent files has changed. Can
	 * be used by toplevel widgets to update their menus.
	 */
	void recentFilesChanged();

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

#endif // _KWAVE_APP_H_

//***************************************************************************
//***************************************************************************
