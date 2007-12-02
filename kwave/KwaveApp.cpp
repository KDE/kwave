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
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kcmdlineargs.h>
#include <kconfig.h>

#ifdef HAVE_ARTS_SUPPORT
#include <kcrash.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <arts/artsflow.h>
#include <artsc/artsc.h> // for arts_init()
#endif /* HAVE_ARTS_SUPPORT */

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

#ifdef HAVE_ARTS_SUPPORT
    // initialize the aRts daemon, start him if necessary
    initArts();
#endif /* HAVE_ARTS_SUPPORT */

    // load the list of plugins
    PluginManager::findPlugins();

    // close when the last window closed
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

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
	for (unsigned int i = 0; i < argc; i++) {
	    QString name;
	    name = QString::fromLocal8Bit(args->arg(i));
	    QFileInfo file(name);

	    newWindow(file.absFilePath());
	}
    }
    if (args) args->clear();

    return 0;
}

//***************************************************************************
bool KwaveApp::isOK()
{
    Q_ASSERT(!m_topwidget_list.isEmpty());
    return (!m_topwidget_list.isEmpty());
}

//***************************************************************************
bool KwaveApp::executeCommand(const QString &command)
{
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
bool KwaveApp::newWindow(const KURL &url)
{
    TopWidget *new_top_widget = new TopWidget(*this);
    Q_ASSERT(new_top_widget);
    if (!new_top_widget) return false;

    if ( !(new_top_widget->isOK()) ) {
	qWarning("KwaveApp::newWindow() failed!");
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

    if (!url.isEmpty()) new_top_widget->loadFile(url);

    return true;
}

//***************************************************************************
bool KwaveApp::closeWindow(TopWidget *todel)
{
    Q_ASSERT(todel);

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
    Q_ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");

    QString num;
    for (unsigned int i = 0 ; i < m_recent_files.count(); i++) {
	num.setNum(i);
	cfg->writeEntry(num, m_recent_files[i].utf8().data());
    }

    cfg->sync();
}

//***************************************************************************
void KwaveApp::saveConfig()
{
    // save the list of recent files
    saveRecentFiles();
}

//***************************************************************************
void KwaveApp::readConfig()
{
    QString result;
    QString key;

    KConfig *cfg = config();
    Q_ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");
    for (unsigned int i = 0 ; i < 20; i++) {
	key = QString::number(i);        // generate number

	// read corresponding entry, which is stored in UTF-8
	result = QString::fromUtf8(cfg->readEntry(key));
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
#ifdef HAVE_ARTS_SUPPORT
void KwaveApp::initArts()
{
    arts_init();

    if (!Arts::Dispatcher::the()) {
	qWarning("aRts daemon isn't running. Starting it...");

	QString path;
	QStringList args;

	path = QFile::encodeName(KStandardDirs::findExe("kcminit"));
	if (!path.length()) path = "kcminit";

	args.append("arts");

	kdeinitExec(path, args);

	int time = 0;
	do {
	    ::sleep(time/2);
	    arts_init();
	    // every time it fails, we should wait a little longer
	    // between tries
	    time++;
	} while (time < 6 && !Arts::Dispatcher::the());
    }
}
#endif /* HAVE_ARTS_SUPPORT */

//***************************************************************************
#include "KwaveApp.moc"
//***************************************************************************
//***************************************************************************
