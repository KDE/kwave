/***************************************************************************
          TopWidget.cpp  -  Toplevel widget of Kwave
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>

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

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <kapp.h>

#include <qkeycode.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qevent.h>
#include <qframe.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <kcombobox.h>
#include <kfiledialog.h>
#include <kfilefilter.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kstddirs.h>
#include <ktoolbarbutton.h>

#include "libkwave/FileLoader.h"
#include "libkwave/LineParser.h"
#include "libkwave/Parser.h"

#include "libgui/MenuManager.h"
#include "libgui/KwaveFileDialog.h"
#include "libgui/KwavePlugin.h" // for some helper functions

#include "mt/ThreadsafeX11Guard.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "CodecManager.h"
#include "MainWidget.h"
#include "TopWidget.h"
#include "PlaybackController.h"
#include "PluginManager.h"
#include "SignalWidget.h" // for MouseMode

//#include "toolbar/filenew.xpm"
//#include "toolbar/fileopen.xpm"
//#include "toolbar/filefloppy.xpm"
//#include "toolbar/filesaveas.xpm"

//#include "toolbar/editcut.xpm"
//#include "toolbar/editcopy.xpm"
//#include "toolbar/editpaste.xpm"
//#include "toolbar/eraser.xpm"
//#include "toolbar/delete.xpm"

#include "pics/playback_loop.xpm"
#include "pics/playback_pause.xpm"
#include "pics/playback_pause2.xpm"
#include "pics/playback_start.xpm"
#include "pics/playback_stop.xpm"

#include "toolbar/zoomrange.xpm"
#include "toolbar/zoomin.xpm"
#include "toolbar/zoomout.xpm"
#include "toolbar/zoomnormal.xpm"
#include "toolbar/zoomall.xpm"

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {


#define STATUS_ID_SIZE     1 /**< index of status item for size [samples,ms]*/
#define STATUS_ID_MODE     2 /**< index of status item for mode */
#define STATUS_ID_SELECTED 3 /**< index of status item for selection */

#define NEW_FILENAME i18n("New File")

//***************************************************************************
//***************************************************************************
TopWidget::ZoomListPrivate::ZoomListPrivate()
    :QStringList(), m_milliseconds()
{
    append("1 ms",              1L);
    append("10 ms",            10L);
    append("100 ms",          100L);
    append("1 sec",          1000L);
    append("10 sec",     10L*1000L);
    append("30 sec",     30L*1000L);
    append("1 min",   1L*60L*1000L);
    append("10 min", 10L*60L*1000L);
    append("30 min", 30L*60L*1000L);
    append("60 min", 60L*60L*1000L);
};

//***************************************************************************
void TopWidget::ZoomListPrivate::append(const char *text, unsigned int ms)
{
    QStringList::append(text);
    m_milliseconds.append(ms);
}

//***************************************************************************
unsigned int TopWidget::ZoomListPrivate::ms(int index)
{
    return m_milliseconds[index];
}

//***************************************************************************
//***************************************************************************
TopWidget::TopWidget(KwaveApp &main_app)
    :KMainWindow(), m_app(main_app), m_url()
{
    int id=1000; // id of toolbar items
    KIconLoader icon_loader;

    m_blink_on = false;
    m_id_undo = -1;
    m_id_redo = -1;
    m_id_zoomselection = -1;
    m_id_zoomin = -1;
    m_id_zoomout = -1;
    m_id_zoomnormal = -1;
    m_id_zoomall = -1;
    m_id_zoomselect = -1;
    m_main_widget = 0;
    m_menu_manager = 0;
    m_pause_timer = 0;
    m_toolbar = 0;
    m_zoomselect = 0;

    KMenuBar *menubar = menuBar();
    ASSERT(menubar);
    if (!menubar) return;
    m_menu_manager = new MenuManager(this, *menubar);
    ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    m_plugin_manager = new PluginManager(*this);
    ASSERT(m_plugin_manager);
    if (!m_plugin_manager) return;
    if (!m_plugin_manager->isOK()) {
	delete m_plugin_manager;
	m_plugin_manager=0;
	return;
    }

    connect(m_plugin_manager, SIGNAL(sigCommand(const QString &)),
            this, SLOT(executeCommand(const QString &)));
//    connect(this, SIGNAL(sigSignalNameChanged(const QString &)),
//	    m_plugin_manager, SLOT(setSignalName(const QString &)));

    // connect clicked menu entries with main communication channel of kwave
    connect(m_menu_manager, SIGNAL(sigMenuCommand(const QString &)),
	    this, SLOT(executeCommand(const QString &)));


    KStatusBar *status_bar = statusBar();
    ASSERT(status_bar);
    if (!status_bar) return;
    status_bar->insertItem("", STATUS_ID_SIZE);
    status_bar->insertItem("", STATUS_ID_MODE);
    status_bar->insertItem("", STATUS_ID_SELECTED);
    setStatusInfo(SAMPLE_MAX,99,196000,24); // affects the menu !

    // load the menu from file
    QString menufile = locate("data", "kwave/menus.config");
    FileLoader loader(menufile);
    ASSERT(loader.buffer());
    if (loader.buffer()) parseCommands(loader.buffer());
    updateMenu();

    updateRecentFiles();

    m_main_widget = new MainWidget(this);
    ASSERT(m_main_widget);
    if (!m_main_widget) return;
    if (!(m_main_widget->isOK())) {
	warning("TopWidget::TopWidget(): failed at creating main widget");
	delete m_main_widget;
	m_main_widget=0;
	return;
    }

    // connect the main widget
    connect(m_main_widget, SIGNAL(sigCommand(const QString &)),
            this, SLOT(executeCommand(const QString &)));
    connect(m_main_widget, SIGNAL(selectedTimeInfo(unsigned int,
            unsigned int, double)),
            this, SLOT(setSelectedTimeInfo(unsigned int, unsigned int,
            double)));
    connect(m_main_widget, SIGNAL(sigTrackCount(unsigned int)),
            this, SLOT(setTrackInfo(unsigned int)));
    connect(m_main_widget, SIGNAL(sigMouseChanged(int)),
            this, SLOT(mouseChanged(int)));

    // connect the sigCommand signal to ourself, this is needed
    // for the plugins
    connect(this, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(executeCommand(const QString &)));

    // --- set up the toolbar ---

    m_toolbar = new KToolBar(this, "toolbar", true, true);
    ASSERT(m_toolbar);
    if (!m_toolbar) return;
    m_toolbar->setBarPos(KToolBar::Top);
    m_toolbar->setHorizontalStretchable(false);
    this->addToolBar(m_toolbar);
    m_toolbar->insertSeparator(-1);

    // --- file open and save ---

    m_toolbar->insertButton(
	icon_loader.loadIcon("filenew.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarFileNew()), true,
	i18n("create a new empty file"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("fileopen.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarFileOpen()), true,
	i18n("open an existing file"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("filesave.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarFileSave()), true,
	i18n("save the current file"));

    // separator between file and edit
    QFrame *separator1 = new QFrame(m_toolbar, "separator line");
    ASSERT(separator1);
    if (!separator1) return;
    separator1->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    separator1->setFixedWidth(separator1->sizeHint().width());
    m_toolbar->insertSeparator(-1);
    m_toolbar->insertWidget(0, separator1->sizeHint().width(), separator1);
    m_toolbar->insertSeparator(-1);

    // --- edit, cut&paste ---

    m_toolbar->insertButton(
	icon_loader.loadIcon("undo.png", KIcon::Toolbar),
	id, SIGNAL(clicked()),
	this, SLOT(toolbarEditUndo()), true,
	i18n("Undo"));
    m_id_undo = id++;

    m_toolbar->insertButton(
	icon_loader.loadIcon("redo.png", KIcon::Toolbar),
	id, SIGNAL(clicked()),
	this, SLOT(toolbarEditRedo()), true,
	i18n("Redo"));
    m_id_redo = id++;

    m_toolbar->insertButton(
	icon_loader.loadIcon("editcut.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarEditCut()), true,
	i18n("cut the current selection and move it to the clipboard"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("editcopy.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarEditCopy()), true,
	i18n("copy the current selection to the clipboard"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("editpaste.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarEditPaste()), true,
	i18n("insert the content of clipboard"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("eraser.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarEditErase()), true,
	i18n("mute the current selection"));

    m_toolbar->insertButton(
	icon_loader.loadIcon("edittrash.png", KIcon::Toolbar),
	-1, SIGNAL(clicked()),
	this, SLOT(toolbarEditDelete()), true,
	i18n("delete the current selection"));

//                  Zoom
//                  Previous Page/Back
//                  Next Page/Forward
//                  Go To Page/Home

//                  Help

    // separator between edit and playback
    QFrame *separator = new QFrame(m_toolbar, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    separator->setFixedWidth(separator->sizeHint().width());
    m_toolbar->insertSeparator(-1);
    m_toolbar->insertWidget(0, separator->sizeHint().width(), separator);
    m_toolbar->insertSeparator(-1);

    // --- playback controls ---

    QObject *playback = &(m_main_widget->playbackController());
    m_toolbar->insertButton(
	QPixmap(xpm_play), id, SIGNAL(clicked()),
	playback, SLOT(playbackStart()), true,
	i18n("start playback"));
    m_id_play = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_loop), id, SIGNAL(clicked()),
	playback, SLOT(playbackLoop()), true,
	i18n("start playback and loop"));
    m_id_loop = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_pause), id, SIGNAL(clicked()),
	this, SLOT(pausePressed()), true,
	i18n("pause playback"));
    m_id_pause = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_stop), id, SIGNAL(clicked()),
	playback, SLOT(playbackStop()), true,
	i18n("stop playback or loop"));
    m_id_stop = id++;

    // separator between playback and zoom
    QFrame *separator3 = new QFrame(m_toolbar, "separator line");
    ASSERT(separator3);
    if (!separator3) return;
    separator3->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    separator3->setFixedWidth(separator3->sizeHint().width());
    m_toolbar->insertSeparator(-1);
    m_toolbar->insertWidget(0, separator3->sizeHint().width(), separator3);
    m_toolbar->insertSeparator(-1);

    // --- zoom controls ---

    m_toolbar->insertButton(
	QPixmap(xpm_zoomrange), id, SIGNAL(clicked()),
	m_main_widget, SLOT(zoomSelection()), true,
	i18n("zoom to selection"));
    m_id_zoomselection = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomin),
	id, SIGNAL(clicked()),
	m_main_widget, SLOT(zoomIn()), true,
	i18n("zoom in"));
    m_id_zoomin = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomout),
	id, SIGNAL(clicked()),
	m_main_widget, SLOT(zoomOut()), true,
	i18n("zoom out"));
    m_id_zoomout = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomnormal), id, SIGNAL(clicked()),
	m_main_widget, SLOT(zoomNormal()), true,
	i18n("zoom to 100%"));
    m_id_zoomnormal = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomall), id, SIGNAL(clicked()),
	m_main_widget, SLOT(zoomAll()), true,
	i18n("zoom to all"));
    m_id_zoomall = id++;

    // add a dummy placeholder, otherwise the minimum size will be wrong
    QStringList factors(m_zoom_factors);
    factors.append(" 99:99 min ");
    m_id_zoomselect = id++;
    m_toolbar->insertCombo(factors, m_id_zoomselect,
	true, SIGNAL(activated(int)),
	this, SLOT(selectZoom(int)), true,
	i18n("select zoom factor"));
    connect(m_main_widget, SIGNAL(sigZoomChanged(double)),
            this, SLOT(setZoomInfo(double)));
    m_zoomselect = m_toolbar->getCombo(m_id_zoomselect);
    ASSERT(m_zoomselect);
    if (!m_zoomselect) return;

    int h = m_zoomselect->sizeHint().height();
    m_zoomselect->setFocusPolicy(QWidget::NoFocus);

    m_toolbar->setMinimumHeight(max(m_zoomselect->sizeHint().height()+2,
	m_toolbar->sizeHint().height()));
    m_toolbar->insertSeparator(-1);
    updateToolbar();

    // connect the playback controller
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackStarted()),
            this, SLOT(updatePlaybackControls()));
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackPaused()),
            this, SLOT(playbackPaused()));
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackStopped()),
            this, SLOT(updatePlaybackControls()));

    // connect the signal manager
    SignalManager *signal_manager = &(m_main_widget->signalManager());
    connect(signal_manager, SIGNAL(sigStatusInfo(unsigned int, unsigned int,
	unsigned int, unsigned int)),
	this, SLOT(setStatusInfo(unsigned int, unsigned int,
	unsigned int, unsigned int)));
    connect(signal_manager, SIGNAL(sigUndoRedoInfo(const QString&,
	const QString&)),
	this, SLOT(setUndoRedoInfo(const QString&, const QString&)));
    connect(signal_manager, SIGNAL(sigModified(bool)),
            this, SLOT(modifiedChanged(bool)));

    // set the MainWidget as the main view
    setCentralWidget(m_main_widget);

    // limit the window to a reasonable minimum size
    int w = m_main_widget->minimumSize().width();
    h = max(m_main_widget->minimumSize().height(), 150);
    setMinimumSize(w, h);

    // Find out the width for which the menu bar would only use
    // one line. This is tricky because sizeHint().width() always
    // returns -1  :-((     -> just try and find out...
    int wmax = max(w,100) * 10;
    int wmin = w;
    int hmin = menuBar()->heightForWidth(wmax);
    while (wmax-wmin > 5) {
	w = (wmax + wmin) / 2;
	int mh = menuBar()->heightForWidth(w);
	if (mh > hmin) {
	    wmin = w;
	} else {
	    wmax = w;
	    hmin = mh;
	}
    }

    // set a nice initial size
    w = wmax;
    w = max(w, m_main_widget->minimumSize().width());
    w = max(w, m_main_widget->sizeHint().width());
    w = max(w, m_toolbar->sizeHint().width());
    h = max(m_main_widget->sizeHint().height(), w*6/10);
    resize(w, h);

    setStatusInfo(0,0,0,0);
    setUndoRedoInfo(0,0);
    setSelectedTimeInfo(0,0,0);

    // check if the aRts dispatcher is functional. if not, we better
    // should exit now, as most of the plugins would not work
    if (!m_plugin_manager->artsDispatcher()) {
	warning("no aRts dispatcher found -> exit !!!");
	KMessageBox::error(this, i18n(
	    "<b>Sorry, but since version 0.6.2 you need a running "\
	    "aRts sound server for using kwave.</b><br>"\
	    "You can setup aRts through the KDE control panel. "\
	    "For more information, please refer to "\
	    "the documentation shipped with KDE or "\
	    "<a href=http://www.arts-project.org>"\
	    "http://www.arts-project.org</a>."
	));
	qApp->exit();
	return;
    };

    // now we are initialized, load all plugins now
    statusBar()->message(i18n("Loading plugins..."));
    m_plugin_manager->loadAllPlugins();
    statusBar()->message(i18n("Ready."), 1000);

    setTrackInfo(0);
    updateMenu();

    // layout is finished, remove dummy/placeholder zoom entry
    m_zoomselect->removeItem(m_zoomselect->count()-1);
}

//***************************************************************************
bool TopWidget::isOK()
{
    ASSERT(m_menu_manager);
    ASSERT(m_main_widget);
    ASSERT(m_plugin_manager);
    ASSERT(m_toolbar);
    ASSERT(m_zoomselect);

    return ( m_menu_manager && m_main_widget &&
	m_plugin_manager && m_toolbar && m_zoomselect &&
        (m_plugin_manager->artsDispatcher()) );
}

//***************************************************************************
TopWidget::~TopWidget()
{
    ThreadsafeX11Guard x11_guard;

    // close the current file (no matter what the user wants)
    closeFile();

    // close all plugins and the plugin manager itself
    if (m_plugin_manager) delete m_plugin_manager;
    m_plugin_manager = 0;

    if (m_pause_timer) delete m_pause_timer;
    m_pause_timer = 0;

    if (m_main_widget) delete m_main_widget;
    m_main_widget=0;

    if (m_menu_manager) delete m_menu_manager;
    m_menu_manager=0;

    m_app.closeWindow(this);
}

//***************************************************************************
int TopWidget::executeCommand(const QString &line)
{
    int result = 0;
    bool use_recorder = true;
    QString command = line;

//    debug("TopWidget::executeCommand(%s)", command.data()); // ###
    if (!command.length()) return 0; // empty line -> nothing to do
    if (command.stripWhiteSpace().startsWith("#")) return 0; // only a comment

    // special case: if the command contains ";" it is a list of
    // commands -> macro !
    Parser parse_list(command);
    if (parse_list.hasMultipleCommands()) {
	QStringList macro = parse_list.commandList();
	for (QStringList::Iterator it=macro.begin(); it!=macro.end(); ++it) {
	    result = executeCommand("nomacro:"+(*it));
	    ASSERT(!result);
	    if (result) {
		warning("macro execution of '%s' failed: %d",
		        (*it).data(), result);
		return result; // macro failed :-(
	    }
	
	    // wait until the command has completed !
	    ASSERT(m_plugin_manager);
	    if (m_plugin_manager) m_plugin_manager->sync();
	}
	return result;
    }

    // check if the macro recorder has to be disabled for this command
    if (command.startsWith("nomacro:")) {
	use_recorder = false;
	command = command.mid(QString("nomacro:").length());
    }

    // parse one single command
    Parser parser(command);

    if (m_app.executeCommand(command)) {
	return 0;
    CASE_COMMAND("plugin")
	QString name = parser.firstParam();
	QStringList *params = 0;

	int cnt=parser.count();
	if (cnt > 1) {
	    params = new QStringList();
	    ASSERT(params);
	    while (params && cnt--) {
		const QString &par = parser.nextParam();
		debug("TopWidget::executeCommand(): %s", par.data());
		params->append(par);
	    }
	}
	debug("TopWidget::executeCommand(): loading plugin '%s'",name.data());
	debug("TopWidget::executeCommand(): with %d parameter(s)",
		(params) ? params->count() : 0);
	ASSERT(m_plugin_manager);
	if (m_plugin_manager)
	    result = m_plugin_manager->executePlugin(name, params);
    CASE_COMMAND("plugin:execute")
	QStringList params;
	int cnt = parser.count();
	QString name(parser.firstParam());
	while (--cnt > 0) {
	    params.append(parser.nextParam());
	}
	ASSERT(m_plugin_manager);
	result = (m_plugin_manager) ?
	          m_plugin_manager->executePlugin(name, &params) : -ENOMEM;
    CASE_COMMAND("plugin:setup")
	QString name(parser.firstParam());
	ASSERT(m_plugin_manager);
	if (m_plugin_manager) result = m_plugin_manager->setupPlugin(name);
    CASE_COMMAND("menu")
	ASSERT(m_menu_manager);
	if (m_menu_manager) /*result = */m_menu_manager->executeCommand(command);
    CASE_COMMAND("newsignal");
	unsigned int samples = parser.toUInt();
	double       rate    = parser.toDouble();
	unsigned int bits    = parser.toUInt();
	unsigned int tracks  = parser.toUInt();
	result = newSignal(samples, rate, bits, tracks);
    CASE_COMMAND("open")
	QString filename = parser.nextParam();
	if (!filename.isEmpty()) {
	    // open the selected file
	    result = loadFile(filename);
	} else {
	    // show file open dialog
	    result = openFile();
	}
    CASE_COMMAND("openrecent")
	result = openRecent(command);
    CASE_COMMAND("playback")
	result = executePlaybackCommand(parser.firstParam()) ? 0 : -1;
    CASE_COMMAND("save")
	result = saveFile();
    CASE_COMMAND("close")
	result = closeFile();
    CASE_COMMAND("revert")
	result = revert();
    CASE_COMMAND("saveas")
	result = saveFileAs(false);
    CASE_COMMAND("loadbatch")
	result = loadBatch(command);
    CASE_COMMAND("saveselect")
	result = saveFileAs(true);
    CASE_COMMAND("quit")
	result = close();
    } else {
	ASSERT(m_main_widget);
	result = (m_main_widget &&
	    m_main_widget->executeCommand(command)) ? 0 : -1;
    }

    if (use_recorder) {
	// append the command to the macro recorder
        // @TODO macro recording...
    }

    return result;
}

//***************************************************************************
int TopWidget::loadBatch(const QString &str)
{
    Parser parser(str);
    FileLoader loader(parser.firstParam());
    return parseCommands(loader.buffer());
}

//***************************************************************************
int TopWidget::executePlaybackCommand(const QString &command)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return -1;

    PlaybackController &controller = m_main_widget->playbackController();
    if (command == "start") {
	controller.playbackStart();
    } else if (command == "loop") {
	controller.playbackLoop();
    } else if (command == "stop") {
	controller.playbackStop();
    } else if (command == "pause") {
	pausePressed();
    } else if (command == "continue") {
	pausePressed();
    } else {
	return -EINVAL;
    }
    return 0;
}

//***************************************************************************
SignalManager &TopWidget::signalManager()
{
    ASSERT(m_main_widget);
    return m_main_widget->signalManager();
}

//***************************************************************************
int TopWidget::parseCommands(const QByteArray &buffer)
{
    int result = 0;

    LineParser lineparser(buffer);
    QString line = lineparser.nextLine();
    while (line.length() && !result) {
	result = executeCommand(line);
	line = lineparser.nextLine();
    }
    return result;
}

//***************************************************************************
int TopWidget::revert()
{
    ASSERT(m_url.isValid() && !m_url.isMalformed());
    if (!m_url.isValid() || m_url.isMalformed()) return -EINVAL;

    KURL url(m_url);
    return loadFile(url);
}

//***************************************************************************
bool TopWidget::closeFile()
{
    ThreadsafeX11Guard x11_guard;

    ASSERT(m_main_widget);
    if (signalManager().isModified()) {
	int res =  KMessageBox::warningYesNoCancel(this,
	    i18n("This file has been modified.\nDo you want to save it?"));
	if (res == KMessageBox::Cancel) return false;
	if (res == KMessageBox::Yes) {
	    // user decided to save
	    res = saveFile();
	    debug("TopWidget::closeFile()::saveFile, res=%d",res);
	    if (res) return false;
	}
    }

    if (m_main_widget) m_main_widget->closeSignal();

    m_url = KURL();
    updateCaption();
    m_zoomselect->clearEdit();
    emit sigSignalNameChanged(signalName());

    updateMenu();
    updateToolbar();
    setTrackInfo(0);

    return true;
}

//***************************************************************************
int TopWidget::loadFile(const KURL &url)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return -1;

    // abort if new file not valid and local
    ASSERT(url.isLocalFile());
    if (!url.isLocalFile()) return -1;

    // try to close the previous file
    if (!closeFile()) return -1;

    emit sigSignalNameChanged(url.path());

    if (!m_main_widget->loadFile(url)) {
	// succeeded
	m_url = url;
	updateCaption();
    } else {
	// load failed
	closeFile();
    }
    m_app.addRecentFile(signalName());
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
int TopWidget::openRecent(const QString &str)
{
    Parser parser(str);
    return loadFile(parser.firstParam());
}

//***************************************************************************
int TopWidget::openFile()
{
    KwaveFileDialog dlg(":<kwave_open_dir>", CodecManager::decodingFilter(),
        this, "Kwave open file", true);
    dlg.setMode(static_cast<KFile::Mode>(KFile::File | KFile::ExistingOnly));
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Open"));
    if (dlg.exec() == QDialog::Accepted) return loadFile(dlg.selectedURL());
    return -1;
}

//***************************************************************************
int TopWidget::saveFile()
{
    int res = 0;
    ASSERT(m_main_widget);
    if (!m_main_widget) return -EINVAL;

    if (signalName() != NEW_FILENAME) {
	res = m_main_widget->saveFile(m_url, false);
    } else res = saveFileAs(false);

    updateCaption();
    updateMenu();

    return res;
}

//***************************************************************************
int TopWidget::saveFileAs(bool selection)
{
    int res = 0;
    ASSERT(m_main_widget);
    if (!m_main_widget) return -EINVAL;

    KwaveFileDialog dlg(":<kwave_save_dir>", CodecManager::encodingFilter(),
        this, "Kwave save file", true);
    dlg.setKeepLocation(true);
//    dlg.setMode(static_cast<KFile::Mode>(KFile::));
    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setCaption(i18n("Save As"));
    if (dlg.exec() != QDialog::Accepted) return -1;

    KURL url = dlg.selectedURL();
//    getSaveURL(":<kwave-savedir>",
//	CodecManager::encodingFilter(), m_main_widget,
//	"Kwave open file", true);

    if (!url.isEmpty()) {
	QString name = url.path();
	QFileInfo path(name);
	
	// add the (default) extension .wav if necessary
	if (path.extension(false) == "") {
	    name += ".wav";
	    path = name;
	    url.setPath(name);
	}
	
	// check if the file exists and ask before overwriting it
	// if it is not the old filename
	if ((name != signalName()) && (path.exists())) {
	    if (KMessageBox::warningYesNo(this,
	        i18n("The file '%1' already exists. Do you really "\
	        "want to overwrite it?").arg(name)) != KMessageBox::Yes)
	    {
		return -1;
	    }
	}
	
	m_url = url;
	res = m_main_widget->saveFile(m_url, selection);
	
	updateCaption();
	m_app.addRecentFile(signalName());
	updateMenu();
    }

    return res;
}

//***************************************************************************
int TopWidget::newSignal(unsigned int samples, double rate,
                         unsigned int bits, unsigned int tracks)
{
    // abort if the user pressed cancel
    if (!closeFile()) return -1;

    m_url = "file:"+NEW_FILENAME;
    emit sigSignalNameChanged(signalName());

    m_main_widget->newSignal(samples, rate, bits, tracks);

    updateCaption();
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
QString TopWidget::signalName()
{
    return (m_url.isValid() && !m_url.isMalformed()) ?
            m_url.path() : QString("");
}

//***************************************************************************
void TopWidget::selectZoom(int index)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return;

    if (index < 0) return;
    if ((unsigned int)index >= m_zoom_factors.count())
	index = m_zoom_factors.count()-1;

    const double rate = signalManager().rate();
    const double ms = m_zoom_factors.ms(index);
    unsigned int width = m_main_widget->displayWidth();
    ASSERT(width > 1);
    if (width <= 1) width = 2;
    const double new_zoom = rint(((rate*ms)/1E3)-1) / (double)(width-1);
    m_main_widget->setZoom(new_zoom);

    // force the zoom factor to be set, maybe the current selection
    // has been changed/corrected to the previous value so that we
    // don't get a signal.
    setZoomInfo(m_main_widget->zoom());
}

//***************************************************************************
void TopWidget::setZoomInfo(double zoom)
{
    ASSERT(zoom >= 0);
    ASSERT(m_zoomselect);

    if (zoom <= 0.0) return; // makes no sense or signal is empty
    if (!m_zoomselect) return;

    QString strZoom;
    if ((m_main_widget) && (m_main_widget->tracks())) {
	double rate = signalManager().rate();
	if (rate > 0) {
	    // time display mode
	    double ms = m_main_widget->displaySamples()*1E3/(double)rate;
	    int s = (int)floor(ms / 1000.0);
	    int m = (int)floor(s / 60.0);
	
	    if (m >= 1) {
		strZoom = strZoom.sprintf("%02d:%02d min", m, s % 60);
	    } else if (s >= 1) {
		strZoom = strZoom.sprintf("%d sec", s);
	    } else if (ms >= 1) {
		strZoom = strZoom.sprintf("%d ms", (int)round(ms));
	    } else if (ms >= 0.01) {
		strZoom = strZoom.sprintf("%0.3g ms", ms);
	    }
	} else {
	    // percent mode
	    double percent = (double)100.0 / zoom;
	    strZoom = KwavePlugin::zoom2string(percent);
	}
    }

    (strZoom.length()) ? m_zoomselect->setEditText(strZoom) :
                         m_zoomselect->clearEdit();
}

//***************************************************************************
void TopWidget::setStatusInfo(unsigned int length, unsigned int /*tracks*/,
                              unsigned int rate, unsigned int bits)
{
    ASSERT(statusBar());
    ASSERT(m_menu_manager);
    if (!statusBar() || !m_menu_manager) return;
    double ms;
    QString txt;

    // length in milliseconds
    if (length) {
	txt = " "+i18n("Length: %1")+" "+i18n("(%2 samples)")+" ";
	ms = (rate) ? (((double)length / (double)rate) * 1E3) : 0;
	txt = txt.arg(KwavePlugin::ms2string(ms));
	txt = txt.arg(KwavePlugin::dottedNumber(length));
    } else txt = "";
    statusBar()->changeItem(txt, STATUS_ID_SIZE);

    // sample rate and resolution
    if (bits) {
	txt = " "+i18n("Mode: %0.3f kHz@%u bit")+" ";
	txt = txt.sprintf(txt, (double)rate *1E-3, bits);
    } else txt = "";
    statusBar()->changeItem(txt, STATUS_ID_MODE);

}

//***************************************************************************
void TopWidget::setTrackInfo(unsigned int tracks)
{
    ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // update the list of deletable tracks
    m_menu_manager->clearNumberedMenu("ID_EDIT_TRACK_DELETE");
    QString buf;
    for (unsigned int i = 0; i < tracks; i++) {
	m_menu_manager->addNumberedMenuEntry("ID_EDIT_TRACK_DELETE",
	                                     buf.setNum(i));
    }

    // enable/disable all items that depend on having a signal
    bool have_signal = (tracks != 0);
    m_menu_manager->setItemEnabled("@SIGNAL", have_signal);
}

//***************************************************************************
void TopWidget::setSelectedTimeInfo(unsigned int offset, unsigned int length,
                                    double rate)
{
    ASSERT(statusBar());
    if (!statusBar()) return;

    if (length > 1) {
// show offset and length
// Selected: 02:00...05:00 (3 min)
// Selected: 2000...3000 (1000 samples)
	bool sample_mode = false;
	
	unsigned int last = offset + ((length) ? length-1 : 0);
	if (rate == 0) sample_mode = true; // force sample mode if rate==0
	QString txt = " "+i18n("Selected")+": %1...%2 (%3) ";
	if (sample_mode) {
	    txt = txt.arg(offset).arg(last).arg(
	          KwavePlugin::dottedNumber(length) + " " +
	          i18n("samples"));
	} else {
	    double ms_first = (double)offset * 1E3 / rate;
	    double ms_last  = (double)(last+1)   * 1E3 / rate;
	    double ms = (ms_last - ms_first);
	    txt = txt.arg(KwavePlugin::ms2string(ms_first)).arg(
		KwavePlugin::ms2string(ms_last)).arg(
		KwavePlugin::ms2string(ms));
	}
	
	statusBar()->message(txt, 4000);
	m_menu_manager->setItemEnabled("@SELECTION", true);
    } else {
	m_menu_manager->setItemEnabled("@SELECTION", false);
    }
}

//***************************************************************************
void TopWidget::setUndoRedoInfo(const QString &undo, const QString &redo)
{
    ASSERT(m_toolbar);
    if (!m_toolbar) return;

    QString txt;
    QToolButton *button;
    bool undo_enabled = (undo.length() != 0);
    bool redo_enabled = (redo.length() != 0);

    // set the state and tooltip of the undo toolbar button
    m_toolbar->setItemEnabled(m_id_undo, undo_enabled);
    txt = i18n("Undo");
    if (undo_enabled) txt += " (" + undo + ")";
    button = m_toolbar->getButton(m_id_undo);
    ASSERT(button);
    if (button) {
	QToolTip::remove(button);
	QToolTip::add(button, txt);
    }

    // set the state and tooltip of the redo toolbar button
    m_toolbar->setItemEnabled(m_id_redo, redo_enabled);
    txt = i18n("Redo");
    if (redo_enabled) txt += " (" + redo + ")";
    button = m_toolbar->getButton(m_id_redo);
    ASSERT(button);
    if (button) {
	QToolTip::remove(button);
	QToolTip::add(button, txt);
    }

    ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled("ID_EDIT_UNDO", undo_enabled);
    txt = i18n("U&ndo");
    if (undo_enabled) txt += " (" + undo + ")";
    m_menu_manager->setItemText("ID_EDIT_UNDO", txt);

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled("ID_EDIT_REDO", redo_enabled);
    txt = i18n("R&edo");
    if (redo_enabled) txt += " (" + redo + ")";
    m_menu_manager->setItemText("ID_EDIT_REDO", txt);
}

//***************************************************************************
void TopWidget::mouseChanged(int mode)
{
    switch (static_cast<SignalWidget::MouseMode>(mode)) {
	case (SignalWidget::MouseAtSelectionBorder) :
	case (SignalWidget::MouseInSelection) :
	{
	    double rate         = signalManager().rate();
	    unsigned int offset = signalManager().selection().offset();
	    unsigned int length = signalManager().selection().length();
	    setSelectedTimeInfo(offset, length, rate);
	    break;
	}
	default: ;
    }
}

//***************************************************************************
void TopWidget::updateRecentFiles()
{
    ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    m_menu_manager->clearNumberedMenu("ID_FILE_OPEN_RECENT");

    QStringList recent_files = m_app.recentFiles();
    QStringList::Iterator it;
    for (it = recent_files.begin(); it != recent_files.end(); ++it) {
	ASSERT(it != 0);
	if (it == 0) break;
	m_menu_manager->addNumberedMenuEntry("ID_FILE_OPEN_RECENT", *it);
    }
}

//***************************************************************************
void TopWidget::updateMenu()
{
    ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // enable/disable all items that depend on having a file
    bool have_file = (signalName().length() != 0);
    m_menu_manager->setItemEnabled("@NOT_CLOSED", have_file);
}

//***************************************************************************
void TopWidget::updateToolbar()
{
    ASSERT(m_toolbar);
    ASSERT(m_main_widget);
    if (!m_toolbar) return;
    if (!m_main_widget) return;

    bool have_signal = m_main_widget->tracks();

    m_toolbar->setItemEnabled(m_id_zoomselection, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomin, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomout, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomnormal, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomall, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomselect, have_signal);

    updatePlaybackControls();
}

//***************************************************************************
void TopWidget::updatePlaybackControls()
{
    ASSERT(m_toolbar);
    ASSERT(m_main_widget);
    if (!m_toolbar) return;
    if (!m_main_widget) return;

    bool have_signal = m_main_widget->tracks();
    bool playing = m_main_widget->playbackController().running();
    bool paused  = m_main_widget->playbackController().paused();

    if (m_pause_timer) {
	m_pause_timer->stop();
	delete m_pause_timer;
	m_pause_timer = 0;
	
	// NOTE: working with toolbar->getButton is ugly and NOT
	//       recommended, but the only way this works in KDE3 :-(
	KToolBarButton *button = m_toolbar->getButton(m_id_pause);
	ASSERT(button);
	if (!button) return;
	button->setDefaultPixmap(xpm_pause);
	
	// doesn't work with the damned buggy KDE3
	// m_toolbar->setButtonPixmap(m_id_pause, xpm_pause);
    }

    // enable/disable the buttons

    m_toolbar->setItemEnabled(m_id_play,  have_signal && !playing);
    m_toolbar->setItemEnabled(m_id_loop,  have_signal && !playing);
    m_toolbar->setItemEnabled(m_id_pause, have_signal && (playing || paused));
    m_toolbar->setItemEnabled(m_id_stop,  have_signal && (playing || paused));

    m_menu_manager->setItemEnabled("ID_PLAYBACK_START",
	have_signal && !playing);
    m_menu_manager->setItemEnabled("ID_PLAYBACK_LOOP",
	have_signal && !playing);
    m_menu_manager->setItemEnabled("ID_PLAYBACK_PAUSE",
	have_signal && (playing));
    m_menu_manager->setItemEnabled("ID_PLAYBACK_CONTINUE",
	have_signal && (paused));
    m_menu_manager->setItemEnabled("ID_PLAYBACK_STOP",
	have_signal && (playing || paused));

}

//***************************************************************************
void TopWidget::playbackPaused()
{
    updateToolbar();

    if (!m_pause_timer) {
	m_pause_timer = new QTimer(this);
	ASSERT(m_pause_timer);
	if (!m_pause_timer) return;

	m_pause_timer->start(500, false);
	connect(m_pause_timer, SIGNAL(timeout()),
	        this, SLOT(blinkPause()));
	m_toolbar->setButtonPixmap(m_id_pause, xpm_pause2);
	m_blink_on = true;
    }
}

//***************************************************************************
void TopWidget::blinkPause()
{
    ASSERT(m_toolbar);
    if (!m_toolbar) return;

    // NOTE: working with toolbar->getButton is ugly and NOT
    //       recommended, but the only way this works in KDE3 :-(
    KToolBarButton *button = m_toolbar->getButton(m_id_pause);
    ASSERT(button);
    if (!button) return;
    button->setDefaultPixmap(m_blink_on ? xpm_pause2 : xpm_pause);

    // this would be the correct way, but KDE-3.0.1 is too buggy
    // to let it work...
    //    m_toolbar->setButtonPixmap(m_id_pause,
    //        m_blink_on ? xpm_pause2 : xpm_pause);

    m_blink_on = !m_blink_on;
}

//***************************************************************************
void TopWidget::pausePressed()
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return;

    bool have_signal = (m_main_widget->tracks());
    bool playing = m_main_widget->playbackController().running();

    if (!have_signal) return;

    if (playing) {
	m_main_widget->playbackController().playbackPause();
    } else {
	m_main_widget->playbackController().playbackContinue();
    }

}

//***************************************************************************
void TopWidget::modifiedChanged(bool)
{
    updateCaption();
}

//***************************************************************************
void TopWidget::updateCaption()
{
    bool modified = signalManager().isModified();

    // shortcut if no file loaded
    if (signalName().length() == 0) {
	setCaption(0);
	return;
    }

    if (modified)
	setCaption("* "+signalName()+i18n(" (modified)"));
    else
	setCaption(signalName());
}

//***************************************************************************
void TopWidget::closeEvent(QCloseEvent *e)
{
    (closeFile()) ? e->accept() : e->ignore();
}

//***************************************************************************
//***************************************************************************
