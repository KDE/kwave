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

#include <QFile>
#include <QString>
#include <QMetaType>

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ktoolinvocation.h>

#include "libkwave/ClipBoard.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/MemoryManager.h"
#include "libkwave/Parser.h"

#include "TopWidget.h"
#include "KwaveApp.h"
#include "KwaveSplash.h"

//***************************************************************************
KwaveApp::KwaveApp()
   :KUniqueApplication(),
    m_recent_files(),
    m_topwidget_list()
{
    qRegisterMetaType<Kwave::SampleArray>("Kwave::SampleArray");

    // connect the clipboard
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
	    &(ClipBoard::instance()), SLOT(slotChanged(QClipboard::Mode)));
}

//***************************************************************************
int KwaveApp::newInstance()
{
    static bool first_time = true;
    if (first_time) {
	first_time = false;

	KwaveSplash::showMessage(i18n("reading configuration..."));
	readConfig();

	// close when the last window closed
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    unsigned int argc = (args) ? args->count() : 0;

    // only one parameter -> open with empty window
    if (argc == 0) {
	newWindow(KUrl(0));
    } else {
	// open a window for each file specified in the
	// command line an load it
	for (unsigned int i = 0; i < argc; i++) {
	    QString name = args->arg(i);
	    QFileInfo file(name);
	    newWindow(file.absoluteFilePath());
	}
    }
    if (args) args->clear();

    return 0;
}

//***************************************************************************
bool KwaveApp::isOK()
{
    return (!m_topwidget_list.isEmpty());
}

//***************************************************************************
bool KwaveApp::executeCommand(const QString &command)
{
    Parser parser(command);
    if (parser.command() == "newwindow") {
	if (parser.hasParams()) {
	    newWindow(KUrl(parser.params().at(0)));
	} else {
	    newWindow(KUrl(0));
	}
    } else if (parser.command() == "help") {
	KToolInvocation::invokeHelp();
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
    m_recent_files.removeAll(newfile);

    // shorten the list down to 19 entries
    while (m_recent_files.count() > 19)
	m_recent_files.removeLast();

    // insert the new entry at top
    m_recent_files.prepend(newfile);

    // save the list of recent files
    saveRecentFiles();

    // update all toplevel widgets
    emit recentFilesChanged();
}

//***************************************************************************
bool KwaveApp::newWindow(const KUrl &url)
{
    KwaveSplash::showMessage(i18n("opening main window..."));
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
	setTopWidget(new_top_widget); // sets geometry and other properties
	setTopWidget(0);              // that's enough, dont quit on close !
    } else {
	// create a new widget with the same geometry as
	// the last created one
	const QRect &geom = m_topwidget_list.last()->geometry();
	// calling setGeometry(geom) would overlap :-(
	new_top_widget->resize(geom.width(), geom.height());
    }

    m_topwidget_list.append(new_top_widget);
    new_top_widget->show();

    // inform the widget about changes in the list of recent files
    connect(this, SIGNAL(recentFilesChanged()),
            new_top_widget, SLOT(updateRecentFiles()));

    if (!url.isEmpty()) {
	KwaveSplash::showMessage(i18n("loading file '%1'...",
	    url.prettyUrl()));
	new_top_widget->loadFile(url);
    }

    KwaveSplash::showMessage(i18n("startup done."));
    return true;
}

//***************************************************************************
bool KwaveApp::closeWindow(TopWidget *todel)
{
    Q_ASSERT(todel);

    // save the configuration, including the list of recent files
    saveConfig();

    if (todel) m_topwidget_list.removeAll(todel);

    // if list is empty -> no more windows there -> exit application
    return (m_topwidget_list.isEmpty());
}

//***************************************************************************
void KwaveApp::saveRecentFiles()
{
    KConfigGroup cfg = KGlobal::config()->group("Recent Files");

    QString num;
    for (int i = 0 ; i < m_recent_files.count(); i++) {
	num.setNum(i);
	cfg.writeEntry(num, m_recent_files[i]);
    }

    cfg.sync();
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
    const KConfigGroup cfg = KGlobal::config()->group("Recent Files");

    for (unsigned int i = 0 ; i < 20; i++) {
	key = QString::number(i);        // generate number

	// read corresponding entry, which is stored in UTF-8
	result = cfg.readEntry(key);
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

    while (!m_topwidget_list.isEmpty()) {
	TopWidget *todel = m_topwidget_list.takeLast();
	if (todel) delete todel;
    }
    m_recent_files.clear();
}

//***************************************************************************
#include "KwaveApp.moc"
//***************************************************************************
//***************************************************************************
