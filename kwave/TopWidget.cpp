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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <kapp.h>

#include <qkeycode.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qframe.h>
#include <qtoolbutton.h>

#include <kcombobox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenubar.h>
#include <kstddirs.h>
#include <ktoolbarbutton.h>

#include "libkwave/FileFormat.h"
#include "libkwave/FileLoader.h"
#include "libkwave/LineParser.h"
#include "libkwave/Parser.h"

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/KwavePlugin.h" // for some helper functions

#include "KwaveApp.h"
#include "ClipBoard.h"
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

#include "toolbar/play.xpm"
#include "toolbar/loop.xpm"
#include "toolbar/pause.xpm"
#include "toolbar/pause2.xpm"
#include "toolbar/stop.xpm"

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


//***************************************************************************
//***************************************************************************
TopWidget::ZoomListPrivate::ZoomListPrivate()
:QStringList()
{
    clear();
    append("800 %");
    append("400 %");
    append("200 %");
    append("100 %");
    append("33 %");
    append("10 %");
    append("3 %");
    append("1 %");
    append("0.1 %");
    append("0.01 %");
};

/** list of predefined zoom factors */
static TopWidget::ZoomListPrivate zoom_factors;

//***************************************************************************
TopWidget::TopWidget(KwaveApp &main_app)
    :KMainWindow(),
    m_app(main_app)
{
    int id=1000; // id of toolbar items
    KIconLoader icon_loader;

    m_save_bits = 16;
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
    m_filename = "";
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

    KStatusBar *status_bar = statusBar();
    ASSERT(status_bar);
    if (!status_bar) return;
    status_bar->insertItem("", STATUS_ID_SIZE);
    status_bar->insertItem("", STATUS_ID_MODE);
    status_bar->insertItem("", STATUS_ID_SELECTED);
    setStatusInfo(SAMPLE_MAX,99,196000,24);

    // connect clicked menu entries with main communication channel of kwave
    connect(m_menu_manager, SIGNAL(sigMenuCommand(const QString &)),
	    this, SLOT(executeCommand(const QString &)));

//    //enable drop of local files onto kwave window
//    dropZone = new KDNDDropZone( this , DndURL);
//    ASSERT(dropZone);
//    if (!dropZone) return;
//    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *)),
//	     this, SLOT( dropEvent( KDNDDropZone *)));

    // load the menu from file
    QString menufile = locate("data", "kwave/menus.config");
    FileLoader loader(menufile);
    ASSERT(loader.buffer());
    if (loader.buffer()) parseCommands(loader.buffer());
    updateMenu();

    updateRecentFiles();

    m_main_widget = new MainWidget(this, *m_menu_manager);
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
    connect(m_main_widget, SIGNAL(selectedTimeInfo(double)),
	    this, SLOT(setSelectedTimeInfo(double)));
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
	
    m_id_zoomselect = id++;
    m_toolbar->insertCombo(zoom_factors, m_id_zoomselect,
	true, SIGNAL(activated(int)),
	this, SLOT(selectZoom(int)), true,
	i18n("select zoom factor"));
    connect(m_main_widget, SIGNAL(sigZoomChanged(double)),
            this, SLOT(setZoomInfo(double)));
    m_zoomselect = m_toolbar->getCombo(m_id_zoomselect);
    ASSERT(m_zoomselect);
    if (!m_zoomselect) return;

    m_zoomselect->adjustSize();
    int h = m_zoomselect->sizeHint().height();
    m_zoomselect->setFixedHeight(h);
    m_zoomselect->setMinimumWidth(
        max(m_zoomselect->sizeHint().width()+10, 3*h));
    m_zoomselect->setAutoResize(false);
    m_zoomselect->setFocusPolicy(QWidget::NoFocus);
    m_toolbar->setMinimumHeight(max(m_zoomselect->sizeHint().height()+2,
	m_toolbar->sizeHint().height()));

    m_toolbar->insertSeparator(-1);
    updateToolbar();

    // connect the playback controller
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackStarted()),
            this, SLOT(updateToolbar()));
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackPaused()),
            this, SLOT(playbackPaused()));
    connect(&(m_main_widget->playbackController()),
            SIGNAL(sigPlaybackStopped()),
            this, SLOT(updateToolbar()));

    // connect the signal manager
    SignalManager *signal_manager = &(m_main_widget->signalManager());
    connect(signal_manager, SIGNAL(sigStatusInfo(unsigned int, unsigned int,
	unsigned int, unsigned int)),
	this, SLOT(setStatusInfo(unsigned int, unsigned int,
	unsigned int, unsigned int)));
    connect(signal_manager, SIGNAL(sigUndoRedoInfo(const QString&,
	const QString&)),
	this, SLOT(setUndoRedoInfo(const QString&, const QString&)));

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

    // now we are initialized, load all plugins now
    statusBar()->message(i18n("Loading plugins..."));
    m_plugin_manager->loadAllPlugins();
    statusBar()->message(i18n("Ready."), 1000);

    debug("TopWidget::TopWidget(): done."); // ###
}

//***************************************************************************
bool TopWidget::isOK()
{
//    ASSERT(m_menu_manager);
//    ASSERT(m_main_widget);
//    ASSERT(m_plugin_manager);
//    ASSERT(m_toolbar);
//    ASSERT(m_zoomselect);
//
//    return ( m_menu_manager && m_main_widget &&
//	m_plugin_manager && m_toolbar && m_zoomselect );
    return true;
}

//***************************************************************************
TopWidget::~TopWidget()
{
//    debug("TopWidget::~TopWidget()");

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

//    debug("TopWidget::~TopWidget(): done.");
}

//***************************************************************************
void TopWidget::executeCommand(const QString &command)
{
//    debug("TopWidget::executeCommand(%s)", command.data()); // ###
    if (!command.length()) return;
    if (command.stripWhiteSpace().startsWith("#")) return; // only a comment

    Parser parser(command);

    if (m_app.executeCommand(command)) {
	return ;
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
	debug("TopWidget::executeCommand(): loading plugin '%s'",
            name.data());
	debug("TopWidget::executeCommand(): with %d parameter(s)",
		(params) ? params->count() : 0);
	ASSERT(m_plugin_manager);
	if (m_plugin_manager) m_plugin_manager->executePlugin(name, params);
    CASE_COMMAND("plugin:execute")
	QStringList params;
	int cnt = parser.count();
	QString name(parser.firstParam());
	while (--cnt > 0) {
	    params.append(parser.nextParam());
	}
	ASSERT(m_plugin_manager);
	if (m_plugin_manager) m_plugin_manager->executePlugin(
            name, &params);
    CASE_COMMAND("plugin:setup")
	QString name(parser.firstParam());
	ASSERT(m_plugin_manager);
	if (m_plugin_manager) m_plugin_manager->setupPlugin(name);
    CASE_COMMAND("menu")
	ASSERT(m_menu_manager);
	if (m_menu_manager) m_menu_manager->executeCommand(command);
    CASE_COMMAND("open")
	openFile();
    CASE_COMMAND("openrecent")
	openRecent(command);
    CASE_COMMAND("save")
	saveFile();
    CASE_COMMAND("close")
	closeFile();
    CASE_COMMAND("resolution")
	resolution(command);
    CASE_COMMAND("revert")
	revert();
    CASE_COMMAND("importascii")
	importAsciiFile();
    CASE_COMMAND("exportascii")
	exportAsciiFile();
    CASE_COMMAND("saveas")
	saveFileAs(false);
    CASE_COMMAND("loadbatch")
	loadBatch(command);
    CASE_COMMAND("saveselect")
	saveFileAs(true);
    CASE_COMMAND("quit")
	close();
    } else {
	ASSERT(m_main_widget);
	if ((m_main_widget) && (m_main_widget->executeCommand(command))) {
	    return ;
	}
    }

}

//***************************************************************************
void TopWidget::loadBatch(const QString &str)
{
    Parser parser(str);
    FileLoader loader(parser.firstParam());
    parseCommands(loader.buffer());
}

//***************************************************************************
SignalManager &TopWidget::signalManager()
{
    ASSERT(m_main_widget);
    return m_main_widget->signalManager();
}

//***************************************************************************
void TopWidget::parseCommands(const QByteArray &buffer)
{
    LineParser lineparser(buffer);
    QString line = lineparser.nextLine();
    while (line.length()) {
	executeCommand(line);
	line = lineparser.nextLine();
    }
}

//***************************************************************************
void TopWidget::revert()
{
    QString name = m_filename;
    loadFile(name, WAV);
}

//***************************************************************************
void TopWidget::resolution(const QString &str)
{
    Parser parser (str);
    int bps = parser.toInt();

    if ( (bps >= 0) && (bps <= 24) && (bps % 8 == 0)) {
	m_save_bits = bps;
	debug("m_save_bits=%d", m_save_bits);    // ###
    }
    else warning("out of range");
}

//***************************************************************************
bool TopWidget::closeFile()
{
    ASSERT(m_main_widget);
//    if (m_main_widget) {
//	// if this failed, the used pressed "cancel"
//	if (!m_main_widget->closeSignal()) return false;
//    }
    m_main_widget->closeSignal();

    m_filename = "";
    setCaption(0);
    m_zoomselect->clearEdit();
    emit sigSignalNameChanged(m_filename);
    updateMenu();
    updateToolbar();

    return true;
}

//***************************************************************************
int TopWidget::loadFile(const QString &filename, int type)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return -1;

    // abort if new file not valid
    ASSERT(filename.length());
    if (!filename.length()) return -1;

    // try to close the previous file
    if (!closeFile()) return -1;

    m_filename = filename;
    emit sigSignalNameChanged(m_filename);

    m_main_widget->loadFile(filename, type);
    m_app.addRecentFile(m_filename);

    setCaption(m_filename);

    m_save_bits = m_main_widget->bits();
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
void TopWidget::openRecent(const QString &str)
{
    Parser parser(str);
    loadFile(parser.firstParam(), WAV);
}

////*************************************************************************
//void TopWidget::dropEvent(KDNDDropZone *drop)
//{
//    ASSERT(drop);
//    if (!drop) return;
//
//    QStrList &list = drop->getURLList();
//    char *s = list.getFirst();
//    if (s) {
//	QString name = s;
//	if ( name.left(5) == "file:")
//	    loadFile(name.right(name.length() - 5), WAV);
//    }
//}

//***************************************************************************
void TopWidget::openFile()
{
    QString filename;
    QString dir = KwaveApp::defaultOpenDir();

    filename = KFileDialog::getOpenFileName(dir, "*.wav", this);
    if (filename.length()) {
	QFileInfo path(filename);
	KwaveApp::setDefaultOpenDir(path.dirPath());
	
	loadFile(filename, WAV);
    }
}

//***************************************************************************
void TopWidget::importAsciiFile()
{
    QString filename;
    QString dir = KwaveApp::defaultOpenDir();

    filename = KFileDialog::getOpenFileName(dir, "*.asc", this);
    if (filename.length()) {
	QFileInfo path(filename);
	KwaveApp::setDefaultOpenDir(path.dirPath());
	
	loadFile(filename, ASCII);
    }
}

//***************************************************************************
void TopWidget::exportAsciiFile()
{
    QString name;
    QString dir = KwaveApp::defaultOpenDir();

    name = KFileDialog::getSaveFileName(dir, "*.asc", this);
    if (name.length()) {
	QFileInfo path(name);
	KwaveApp::setDefaultSaveDir(path.dirPath());
	
	m_main_widget->saveFile(name, m_save_bits, ASCII, false);
    }
}

//***************************************************************************
void TopWidget::saveFile()
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return;

    if (m_filename.length()) {
	QFileInfo path(m_filename);
	KwaveApp::setDefaultSaveDir(path.dirPath());
	
	m_main_widget->saveFile(m_filename, m_save_bits, 0, false);
	setCaption(m_filename);
	updateMenu();
    } else saveFileAs(false);
}

//***************************************************************************
void TopWidget::saveFileAs(bool selection)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return;

    QString dir = KwaveApp::defaultSaveDir();
    QString name = KFileDialog::getSaveFileName(dir, "*.wav", m_main_widget);
    if (!name.isNull()) {
	QFileInfo path(name);
	KwaveApp::setDefaultSaveDir(path.dirPath());
	
	m_filename = name;
	m_main_widget->saveFile(m_filename, m_save_bits, 0, selection);
	setCaption(m_filename);
	m_app.addRecentFile(m_filename);
	updateMenu();
    }
}

//***************************************************************************
const QString &TopWidget::getSignalName()
{
    return m_filename;
}

//***************************************************************************
void TopWidget::selectZoom(int index)
{
    ASSERT(m_main_widget);
    if (!m_main_widget) return;

    if (index < 0) return;
    if ((unsigned int)index >= zoom_factors.count())
	index = zoom_factors.count()-1;

    double new_zoom;
    QStringList::Iterator text = zoom_factors.at(index);
    new_zoom = (text != 0) ? (*text).toDouble() : 0.0;
    if (new_zoom != 0.0) new_zoom = (100.0 / new_zoom);

    m_main_widget->setZoom(new_zoom);

    // force the zoom factor to be set, maybe the current selection
    // has been changed/corrected to the previous value so that we
    // don't get a signal.
    setZoomInfo(m_main_widget->zoom());
}

//***************************************************************************
void TopWidget::setZoomInfo(double zoom)
{
//    debug("void TopWidget::setZoomInfo(%0.5f)", zoom);
    ASSERT(zoom >= 0);
    ASSERT(m_zoomselect);

    if (zoom <= 0.0) return; // makes no sense or signal is empty
    if (!m_zoomselect) return;

    double percent = (double)100.0 / zoom;
    QString strZoom;

    if ((m_main_widget) && (m_main_widget->tracks())) {
	strZoom = KwavePlugin::zoom2string(percent);
    }

    (strZoom.length()) ? m_zoomselect->setEditText(strZoom) :
                         m_zoomselect->clearEdit();
}

//***************************************************************************
void TopWidget::setStatusInfo(unsigned int length, unsigned int /*tracks*/,
                              unsigned int rate, unsigned int bits)
{
    ASSERT(statusBar());
    if (!statusBar()) return;
    double ms;
    QString txt;

    // length in milliseconds
    if (length) {
	txt = " "+i18n("Length: %1")+" "+i18n("(%2 samples)")+" ";
	ms = (rate) ? (((double)length / (double)rate) * 1E3) : 0;
	txt = txt.arg(KwavePlugin::ms2string(ms));
	txt = txt.arg(length);
    } else txt = "";
    statusBar()->changeItem(txt, STATUS_ID_SIZE);

    // sample rate and resolution
    if (bits) {
	txt = " "+i18n("Mode: %u bit@%0.3f kHz")+" ";
	txt = txt.sprintf(txt, bits, (double)rate *1E-3);
    } else txt = "";
    statusBar()->changeItem(txt, STATUS_ID_MODE);
}

//***************************************************************************
void TopWidget::setSelectedTimeInfo(double ms)
{
    ASSERT(statusBar());
    if (!statusBar()) return;

    QString txt = " "+i18n("Selected: %1").arg(
	KwavePlugin::ms2string(ms))+" ";

    unsigned int rate = signalManager().rate();
    if (rate) {
	txt += i18n("(%2 samples)")+" ";
	txt = txt.arg((unsigned int)(ms*1E-3* rate));
    }
    statusBar()->message(txt, 4000);
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
    if (button) {
	QString old_text = button->textLabel();
	button->setTextLabel(txt, true);
	button->setTextLabel(old_text, false);
    }

    // set the state and tooltip of the redo toolbar button
    m_toolbar->setItemEnabled(m_id_redo, redo_enabled);
    txt = i18n("Redo");
    if (redo_enabled) txt += " (" + redo + ")";
    button = m_toolbar->getButton(m_id_redo);
    if (button) {
	QString old_text = button->textLabel();
	button->setTextLabel(txt, true);
	button->setTextLabel(old_text, false);
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
	case (SignalWidget::MouseNormal) :
	    break;
	case (SignalWidget::MouseAtSelectionBorder) :
	case (SignalWidget::MouseInSelection) :
	{
	    unsigned int rate = signalManager().rate();
	    if (!rate) break;
	    double ms = signalManager().selection().length() * 1E3 / rate;
	    setSelectedTimeInfo(ms);
	    break;
	}
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

    QString bps = QString("ID_FILE_SAVE_RESOLUTION_%1").arg(m_save_bits);
    m_menu_manager->selectItem("@BITS_PER_SAMPLE", bps);
}

//***************************************************************************
void TopWidget::updateToolbar()
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
	m_toolbar->setButtonPixmap(m_id_pause, xpm_pause);
    }

    // enable/disable the buttons

    m_toolbar->setItemEnabled(m_id_play,  have_signal && !playing);
    m_toolbar->setItemEnabled(m_id_loop,  have_signal && !playing);
    m_toolbar->setItemEnabled(m_id_pause, have_signal && (playing || paused));
    m_toolbar->setItemEnabled(m_id_stop,  have_signal && (playing || paused));

    m_toolbar->setItemEnabled(m_id_zoomselection, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomin, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomout, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomnormal, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomall, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomselect, have_signal);
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

    m_toolbar->setButtonPixmap(m_id_pause,
	m_blink_on ? xpm_pause2 : xpm_pause);
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
//***************************************************************************
