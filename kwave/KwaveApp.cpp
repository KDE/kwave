/***************************************************************************
           KwaveApp.cpp  -  The Kwave main application
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

#include "config.h"

#include <qfile.h>
#include <qlist.h>
#include <qstring.h>

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kcrash.h>
#include <kglobal.h>

#include "libkwave/Parser.h"

#include "ClipBoard.h"
#include "MemoryManager.h"
#include "PluginManager.h"
#include "TopWidget.h"
#include "KwaveApp.h"

#define UNIQUE_APP

//***************************************************************************
// some static initializers

static ClipBoard _clipboard;
static MemoryManager _memory_manager;

ClipBoard &KwaveApp::m_clipboard(_clipboard);
MemoryManager &KwaveApp::m_memory_manager(_memory_manager);

//***************************************************************************
KwaveApp::KwaveApp()
#ifdef UNIQUE_APP
   :KUniqueApplication(),
#else // UNIQUE_APP
   :KApplication(),
#endif // UNIQUE_APP
    m_recent_files(),
    m_topwidget_list()
{
    m_topwidget_list.setAutoDelete(false);
    readConfig();

    // load the list of plugins
    PluginManager::findPlugins();

#ifndef UNIQUE_APP
    newInstance();
#endif
}

//***************************************************************************
int KwaveApp::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    unsigned int argc = (args) ? args->count() : 0;

    // only one parameter -> open with empty window
    if (argc == 0) {
	newWindow(0);
    } else {
	// open a window for each file specified in the
	// command line an load it
	QString filename;
	for (unsigned int i = 0; i < argc; i++) {
	    filename = QFile::decodeName(args->arg(i));
	    newWindow(filename);
	}
    }
    if (args) args->clear();

    return 0;
}

//***************************************************************************
bool KwaveApp::isOK()
{
    ASSERT(!m_topwidget_list.isEmpty());
    return (!m_topwidget_list.isEmpty());
}

//***************************************************************************
bool KwaveApp::executeCommand(const QString &command)
{
//    debug("KwaveApp::executeCommand(%s)", command);    // ###
    Parser parser(command);
    if (parser.command() == "newwindow") {
	newWindow(0);
    } else if (parser.command() == "help") {
	invokeHelp();
    } else {
	return false;
    }
    return true;
}

//***************************************************************************
void KwaveApp::addRecentFile(const QString &newfile)
{
    ASSERT(newfile.length() != 0);
    if (!newfile.length()) return;

    // remove old entries if present
    m_recent_files.remove(newfile);

    // shorten the list down to 19 entries
    while (m_recent_files.count() > 19)
	m_recent_files.remove(m_recent_files.last());

    // insert the new entry at top
    m_recent_files.prepend(newfile);

    // save the list of recent files
    saveRecentFiles();

    // update all toplevel widgets
    emit recentFilesChanged();
}

//***************************************************************************
bool KwaveApp::newWindow(const QString &filename)
{
    TopWidget *new_top_widget = new TopWidget(*this);
    ASSERT(new_top_widget);
    if (!new_top_widget) return false;

    if ( !(new_top_widget->isOK()) ) {
	warning("KwaveApp::newWindow() failed!");
	delete new_top_widget;
	return false;
    }

    if (m_topwidget_list.isEmpty()) {
	// the first widget is the main widget !
	setMainWidget(new_top_widget); // sets geometry and other properties
	setMainWidget(0);              // that's enough, dont quit on close !
    } else {
	// create a new widget with the same geometry as
	// the last created one
	const QRect &geom = m_topwidget_list.last()->geometry();
	// tnew->setGeometry(geom); // would overlap :-(
	new_top_widget->resize(geom.width(), geom.height());
    }

    m_topwidget_list.append(new_top_widget);
    new_top_widget->show();

    // inform the widget about changes in the list of recent files
    connect(this, SIGNAL(recentFilesChanged()),
            new_top_widget, SLOT(updateRecentFiles()));

    if (filename.length()) new_top_widget->loadFile(filename);

    return true;
}

//***************************************************************************
bool KwaveApp::closeWindow(TopWidget *todel)
{
    debug("KwaveApp::closeWindow(TopWidget *todel)");
    ASSERT(todel);

    // save the configuration, including the list of recent files
    saveConfig();

    m_topwidget_list.setAutoDelete(false);
    if (todel) m_topwidget_list.removeRef(todel);

    // if list is empty -> no more windows there -> exit application
    return (m_topwidget_list.isEmpty());
}

//***************************************************************************
ClipBoard &KwaveApp::clipboard()
{
    return m_clipboard;
}

//***************************************************************************
MemoryManager &KwaveApp::memoryManager()
{
    return m_memory_manager;
}

//***************************************************************************
void KwaveApp::saveRecentFiles()
{
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");

    QString num;
    for (unsigned int i = 0 ; i < m_recent_files.count(); i++) {
	num.setNum(i);
	cfg->writeEntry(num, m_recent_files[i]);
    }

    cfg->sync();
}

//***************************************************************************
void KwaveApp::saveConfig()
{
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    // also save the list of recent files
    saveRecentFiles();
}

//***************************************************************************
void KwaveApp::readConfig()
{
    QString result;
    QString key;

    KConfig *cfg = config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");
    for (unsigned int i = 0 ; i < 20; i++) {
	key = QString::number(i);        // generate number
	result = cfg->readEntry(key);    // and read corresponding entry
	if (result.length()) {
	    QFile file(result);
	    //check if file exists and insert it if not already present
	    if (file.exists() && (m_recent_files.contains(result) == 0))
		m_recent_files.append(result);
	}
    }

}

//***************************************************************************
KwaveApp::~KwaveApp()
{
    saveConfig();

    m_topwidget_list.setAutoDelete(false);
    while (!m_topwidget_list.isEmpty()) {
	TopWidget *todel = m_topwidget_list.last();
	m_topwidget_list.removeRef(todel);
	delete todel;
    }
    m_clipboard.clear();
    m_recent_files.clear();
    m_memory_manager.close();
}

//***************************************************************************
//***************************************************************************
