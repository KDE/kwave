/***************************************************************************
                App.cpp  -  The Kwave main application
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

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QMetaType>

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ktoolinvocation.h>

#include "libkwave/ApplicationContext.h"
#include "libkwave/ClipBoard.h"
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/MemoryManager.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/PluginManager.h"

#include "TopWidget.h"
#include "App.h"
#include "Splash.h"

//***************************************************************************
Kwave::App::App()
   :KUniqueApplication(),
    m_recent_files(),
    m_top_widgets()
{
    qRegisterMetaType<Kwave::SampleArray>("Kwave::SampleArray");
    qRegisterMetaType<Kwave::LabelList>("Kwave::LabelList");
    qRegisterMetaType<sample_index_t>("sample_index_t");
    qRegisterMetaType<Kwave::MetaDataList>("Kwave::MetaDataList");
    
    // connect the clipboard
    connect(QApplication::clipboard(),
            SIGNAL(changed(QClipboard::Mode)),
            &(Kwave::ClipBoard::instance()),
            SLOT(slotChanged(QClipboard::Mode)));
}

//***************************************************************************
int Kwave::App::newInstance()
{
    static bool first_time = true;
    if (first_time) {
	first_time = false;

	// open the log file if given on the command line
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (args && args->isSet("logfile")) {
	    if (!Kwave::Logger::open(args->getOption("logfile")))
		exit(-1);
	}

	Kwave::Splash::showMessage(i18n("Reading configuration..."));
	readConfig();

	// close when the last window closed
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    unsigned int argc = (args) ? args->count() : 0;

    // only one parameter -> open with empty window
    if (argc == 0) {
	newWindow(KUrl(QString()));
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
bool Kwave::App::isOK() const
{
    return (!m_top_widgets.isEmpty());
}

//***************************************************************************
bool Kwave::App::executeCommand(const QString &command)
{
    Kwave::Parser parser(command);
    if (parser.command() == _("newwindow")) {
	if (parser.hasParams()) {
	    newWindow(KUrl(parser.params().at(0)));
	} else {
	    newWindow(KUrl(QString()));
	}
    } else if (parser.command() == _("help")) {
	KToolInvocation::invokeHelp();
    } else {
	return false;
    }
    return true;
}

//***************************************************************************
void Kwave::App::addRecentFile(const QString &newfile)
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
bool Kwave::App::newWindow(const KUrl &url)
{
    Kwave::Splash::showMessage(i18n("Opening main window..."));

    Kwave::TopWidget *new_top_widget = new Kwave::TopWidget(*this);
    if (!new_top_widget || !new_top_widget->init()) {
	// init failed
	qWarning("ERROR: initialization of TopWidget failed");
	delete new_top_widget;
	return false;
    }

    if (m_top_widgets.isEmpty()) {
	// the first widget is the main widget !
	setTopWidget(new_top_widget); // sets geometry and other properties
	setTopWidget(0);              // that's enough, dont quit on close !
    } else {
	// create a new widget with the same geometry as
	// the last created one
	const QRect &geom = m_top_widgets.last()->geometry();
	// calling setGeometry(geom) would overlap :-(
	new_top_widget->resize(geom.width(), geom.height());
    }

    m_top_widgets.append(new_top_widget);
    new_top_widget->show();

    // inform the widget about changes in the list of recent files
    connect(this, SIGNAL(recentFilesChanged()),
            new_top_widget, SLOT(updateRecentFiles()));

    if (!url.isEmpty()) {
	Kwave::Splash::showMessage(i18n("Loading file '%1'...",
	    url.prettyUrl()));
	new_top_widget->loadFile(url);
    }

    Kwave::Splash::showMessage(i18n("Startup done"));
    return true;
}

//***************************************************************************
bool Kwave::App::closeWindow(Kwave::TopWidget *todel)
{
    Q_ASSERT(todel);

    // save the configuration, including the list of recent files
    saveConfig();

    // remove the toplevel widget from our list
    if (m_top_widgets.contains(todel))
	m_top_widgets.removeAll(todel);

    // if list is empty -> no more windows there -> exit application
    return (m_top_widgets.isEmpty());
}

//***************************************************************************
void Kwave::App::saveRecentFiles()
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
void Kwave::App::saveConfig()
{
    // save the list of recent files
    saveRecentFiles();
}

//***************************************************************************
void Kwave::App::readConfig()
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
Kwave::App::~App()
{
    saveConfig();
    m_recent_files.clear();
}

//***************************************************************************
#include "App.moc"
//***************************************************************************
//***************************************************************************
