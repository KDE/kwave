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
#include <kapp.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qframe.h>
#include <drag.h>

#include <kmsgbox.h>
#include <kapp.h>
#include <kfiledialog.h>

#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include <libkwave/LineParser.h>
#include <libkwave/Global.h>
#include <libkwave/FileLoader.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/KwavePlugin.h" // for some helper functions
#include "sampleop.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "MainWidget.h"
#include "TopWidget.h"
#include "PlaybackController.h"
#include "PluginManager.h"

#include "toolbar/filenew.xpm"
#include "toolbar/fileopen.xpm"
#include "toolbar/filefloppy.xpm"
#include "toolbar/filesaveas.xpm"

#include "toolbar/editcut.xpm"
#include "toolbar/editcopy.xpm"
#include "toolbar/editpaste.xpm"
#include "toolbar/eraser.xpm"
#include "toolbar/delete.xpm"

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

extern Global globals;

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (matchCommand(command, x)) {

/**
 * Primitive class that holds a list of predefined zoom
 * factors.
 */
class ZoomListPrivate: public QStrList
{
public:
    ZoomListPrivate()
	:QStrList(false)
    {
	clear();
	append("400 %");
	append("200 %");
	append("100 %");
	append("33 %");
	append("10 %");
	append("3 %");
	append("1 %");
	append("0.1 %");
    };
};

/** list of predefined zoom factors */
static ZoomListPrivate zoom_factors;

//*****************************************************************************
TopWidget::TopWidget(KwaveApp &main_app, QStrList &recent_files)
    :KTMainWindow(),
    app(main_app),
    recentFiles(recent_files)
{
    int id=1000; // id of toolbar items

//    debug("TopWidget::TopWidget()");
    bits = 16;
    m_blink_on = false;
    caption = 0;
    dropZone = 0;
    m_id_zoomrange = -1;
    m_id_zoomin = -1;
    m_id_zoomout = -1;
    m_id_zoomnormal = -1;
    m_id_zoomall = -1;
    m_id_zoomselect = -1;
    loadDir = 0;
    mainwidget = 0;
    menu = 0;
    menu_bar = 0;
    m_pause_timer = 0;
    signalName = "";
    saveDir = 0;
    status_bar = 0;
    m_toolbar = 0;
    m_zoomselect = 0;

    menu_bar = new KMenuBar(this);
    ASSERT(menu_bar);
    if (!menu_bar) return;

    menu = new MenuManager(this, *menu_bar);
    ASSERT(menu);
    if (!menu) return;

    plugin_manager = new PluginManager(*this);
    ASSERT(plugin_manager);
    if (!plugin_manager) return;
    if (!plugin_manager->isOK()) {
	delete plugin_manager;
	plugin_manager=0;
	return;
    }
    connect(plugin_manager, SIGNAL(sigCommand(const char *)),
            this, SLOT(executeCommand(const char *)));
    connect(this, SIGNAL(sigSignalNameChanged(const QString &)),
	    plugin_manager, SLOT(setSignalName(const QString &)));

    status_bar = new KStatusBar(this);
    ASSERT(status_bar);
    if (!status_bar) return;

    status_bar->insertItem(i18n("Length: 0 ms           "), 1);
    status_bar->insertItem(i18n("Rate: 0 kHz         "), 2);
    status_bar->insertItem(i18n("Samples: 0             "), 3);
    status_bar->insertItem(i18n("selected: 0 ms        "), 4);
    status_bar->insertItem(i18n("Clipboard: 0 ms      "), 5);
    setStatusBar(status_bar);

    // connect clicked menu entries with main communication channel of kwave
    connect(menu, SIGNAL(sigMenuCommand(const char *)),
	    this, SLOT(executeCommand(const char *)));

    //enable drop of local files onto kwave window
    dropZone = new KDNDDropZone( this , DndURL);
    ASSERT(dropZone);
    if (!dropZone) return;
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *)),
	     this, SLOT( dropEvent( KDNDDropZone *)));

    // read menus and create them...
    ASSERT(globals.globalconfigDir);
    if (globals.globalconfigDir) {
	QDir configDir(globals.globalconfigDir);

	ASSERT(configDir.exists(configDir.absFilePath("menus.config")));
	if (configDir.exists(configDir.absFilePath("menus.config"))) {
	    FileLoader loader(configDir.absFilePath("menus.config"));
	    ASSERT(loader.getMem());
	    if (loader.getMem()) parseCommands(loader.getMem());
	}
    }
    setMenu(menu_bar);
    updateMenu();

    updateRecentFiles();

    mainwidget = new MainWidget(this, *menu, *status_bar);
    ASSERT(mainwidget);
    if (!mainwidget) return;
    if (!(mainwidget->isOK())) {
	warning("TopWidget::TopWidget(): failed at creating main widget");
	delete mainwidget;
	mainwidget=0;
	return;
    }

    connect(mainwidget, SIGNAL(sigCommand(const char*)),
	    this, SLOT(executeCommand(const char*)));

    // connect the sigCommand signal to ourself, this is needed
    // for the plugins
    connect(this, SIGNAL(sigCommand(const char *)),
	    this, SLOT(executeCommand(const char *)));

    // --- set up the toolbar ---

    m_toolbar = new KToolBar(this, i18n("toolbar"));
    ASSERT(m_toolbar);
    if (!m_toolbar) return;
    m_toolbar->setBarPos(KToolBar::Top);
    m_toolbar->setFullWidth(false);
    this->addToolBar(m_toolbar);
    m_toolbar->insertSeparator(-1);

    // --- file open and save ---

    m_toolbar->insertButton(
	QPixmap(xpm_filenew), -1, SIGNAL(clicked()),
	this, SLOT(toolbarFileNew()), true,
	i18n("create a new empty file"));

    m_toolbar->insertButton(
	QPixmap(xpm_fileopen), -1, SIGNAL(clicked()),
	this, SLOT(toolbarFileOpen()), true,
	i18n("open an existing file"));

    m_toolbar->insertButton(
	QPixmap(xpm_filefloppy), -1, SIGNAL(clicked()),
	this, SLOT(toolbarFileSave()), true,
	i18n("save the current file"));

    m_toolbar->insertButton(
	QPixmap(xpm_filesaveas), -1, SIGNAL(clicked()),
	this, SLOT(toolbarFileSaveAs()), true,
	i18n("save the current file under a different name"));

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

//  Undo
//  Redo

    m_toolbar->insertButton(
	QPixmap(xpm_editcut), -1, SIGNAL(clicked()),
	this, SLOT(toolbarEditCut()), true,
	i18n("cut the current selection and move it to the clipboard"));

    m_toolbar->insertButton(
	QPixmap(xpm_editcopy), -1, SIGNAL(clicked()),
	this, SLOT(toolbarEditCopy()), true,
	i18n("copy the current selection to the clipboard"));

    m_toolbar->insertButton(
	QPixmap(xpm_editpaste), -1, SIGNAL(clicked()),
	this, SLOT(toolbarEditPaste()), true,
	i18n("insert the content of clipboard"));

    m_toolbar->insertButton(
	QPixmap(xpm_eraser), -1, SIGNAL(clicked()),
	this, SLOT(toolbarEditErase()), true,
	i18n("mute the current selection"));

    m_toolbar->insertButton(
	QPixmap(xpm_delete), -1, SIGNAL(clicked()),
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

    m_toolbar->insertButton(
	QPixmap(xpm_play), id, SIGNAL(clicked()),
	mainwidget->playbackController(), SLOT(playbackStart()), true,
	i18n("start playback"));
    m_id_play = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_loop), id, SIGNAL(clicked()),
	mainwidget->playbackController(), SLOT(playbackLoop()), true,
	i18n("start playback and loop"));
    m_id_loop = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_pause), id, SIGNAL(clicked()),
	this, SLOT(pausePressed()), true,
	i18n("pause playback"));
    m_id_pause = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_stop), id, SIGNAL(clicked()),
	mainwidget->playbackController(), SLOT(playbackStop()), true,
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
	mainwidget, SLOT(zoomRange()), true,
	i18n("zoom to selection"));
    m_id_zoomrange = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomin), id, SIGNAL(clicked()),
	mainwidget, SLOT(zoomIn()), true,
	i18n("zoom in"));
    m_id_zoomin = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomout), id, SIGNAL(clicked()),
	mainwidget, SLOT(zoomOut()), true,
	i18n("zoom out"));
    m_id_zoomout = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomnormal), id, SIGNAL(clicked()),
	mainwidget, SLOT(zoomNormal()), true,
	i18n("zoom to 100%"));
    m_id_zoomnormal = id++;

    m_toolbar->insertButton(
	QPixmap(xpm_zoomall), id, SIGNAL(clicked()),
	mainwidget, SLOT(zoomAll()), true,
	i18n("zoom to all"));
    m_id_zoomall = id++;
	
    m_id_zoomselect = id++;
    m_toolbar->insertCombo(&zoom_factors, m_id_zoomselect,
	true, SIGNAL(activated(int)),
	this, SLOT(selectZoom(int)), true,
	i18n("select zoom factor"));
    connect(mainwidget, SIGNAL(sigZoomChanged(double)),
            this, SLOT(setZoom(double)));
    m_zoomselect = m_toolbar->getCombo(m_id_zoomselect);
    ASSERT(m_zoomselect);
    if (!m_zoomselect) return;

    m_zoomselect->adjustSize();
    int h = m_zoomselect->sizeHint().height();
    m_zoomselect->setFixedHeight(h);
    m_zoomselect->setMinimumWidth(max(m_zoomselect->sizeHint().width()+10, 3*h));
    m_zoomselect->setAutoResize(false);
    m_zoomselect->setFocusPolicy(QWidget::NoFocus);
    m_toolbar->setMinimumHeight(max(m_zoomselect->sizeHint().height()+2,
	m_toolbar->sizeHint().height()));

    m_toolbar->insertSeparator(-1);
    updateToolbar();

    // connect the playback controller
    connect(mainwidget->playbackController(), SIGNAL(sigPlaybackStarted()),
            this, SLOT(updateToolbar()));
    connect(mainwidget->playbackController(), SIGNAL(sigPlaybackPaused()),
            this, SLOT(playbackPaused()));
    connect(mainwidget->playbackController(), SIGNAL(sigPlaybackStopped()),
            this, SLOT(updateToolbar()));

    // set the MainWidget as the main view
    setView(mainwidget);

    // limit the window to a reasonable minimum size
    int w = mainwidget->minimumSize().width();
    h = max(mainwidget->minimumSize().height(), 150);
    setMinimumSize(w, h);

    // Find out the width for which the menu bar would only use
    // one line. This is tricky because sizeHint().width() always
    // returns -1  :-((     -> just try and find out...
    int wmax = max(w,100) * 10;
    int wmin = w;
    int hmin = menu_bar->heightForWidth(wmax);
    while (wmax-wmin > 5) {
	w = (wmax + wmin) / 2;
	int mh = menu_bar->heightForWidth(w);
	if (mh > hmin) {
	    wmin = w;
	} else {
	    wmax = w;
	    hmin = mh;
	}
    }

    // set a nice initial size
    w = wmax;
//    debug("TopWidget::TopWidget(): wmax = %d", wmax); // ###
    w = max(w, mainwidget->minimumSize().width());
    w = max(w, mainwidget->sizeHint().width());
    w = max(w, m_toolbar->sizeHint().width());
    h = max(mainwidget->sizeHint().height(), w*6/10);
    resize(w, h);

//    debug("TopWidget::TopWidget(): done.");
}

//*****************************************************************************
bool TopWidget::isOK()
{
    ASSERT(menu);
    ASSERT(menu_bar);
    ASSERT(mainwidget);
    ASSERT(dropZone);
    ASSERT(plugin_manager);
    ASSERT(status_bar);
    ASSERT(m_toolbar);
    ASSERT(m_zoomselect);

    return ( menu && menu_bar && mainwidget && dropZone && plugin_manager &&
    	     status_bar && m_toolbar && m_zoomselect );
}

//*****************************************************************************
TopWidget::~TopWidget()
{
//    debug("TopWidget::~TopWidget()");
    ASSERT(KApplication::getKApplication());

    // close the current file
    closeFile();

    // close all plugins and the plugin manager itself
    if (plugin_manager) delete plugin_manager;
    plugin_manager = 0;

    KTMainWindow::setCaption(0);
    if (caption) delete[] caption;
    caption=0;

    if (loadDir) delete[] loadDir;
    loadDir=0;

    if (m_pause_timer) delete m_pause_timer;
    m_pause_timer = 0;

    if (mainwidget) delete mainwidget;
    mainwidget=0;

    if (menu) delete menu;
    menu=0;

    if (menu_bar) delete menu_bar;
    menu_bar=0;

    signalName = "";

    if (saveDir) delete saveDir;
    saveDir=0;

    if (status_bar) delete status_bar;
    status_bar=0;

    app.closeWindow(this);

//    debug("TopWidget::~TopWidget(): done.");
}

//*****************************************************************************
void TopWidget::executeCommand(const char *command)
{
//    debug("TopWidget::executeCommand(%s)", command); // ###
    ASSERT(command);
    if (!command) return;

    if (command[0] == '#') {
	return; // only a comment
    } else if (app.executeCommand(command)) {
	return ;
    CASE_COMMAND("plugin")
	Parser parser(command);
	const char *name = parser.getFirstParam();
	QStrList *params = 0;
	
	int cnt=parser.countParams();
	if (cnt > 0) {
	    params = new QStrList();
	    ASSERT(params);
	    while (params && cnt--) {
		params->append(parser.getNextParam());
	    }
	}

	debug("TopWidget::executeCommand(): loading plugin '%s'", name);
	ASSERT(plugin_manager);
	if (plugin_manager) plugin_manager->executePlugin(name, params);
    CASE_COMMAND("plugin:execute")
	Parser parser(command);
	QStrList params;
	int cnt = parser.countParams();
	
	parser.getCommand(); // remove the command name
	QString name(parser.getFirstParam());
	while (--cnt) {
	    params.append(parser.getNextParam());
	}

#ifdef DEBUG
	debug("TopWidget::executeCommand(): executing plugin '%s'",
	    name.data());
#endif
	ASSERT(plugin_manager);
	if (plugin_manager) plugin_manager->executePlugin(name.data(), &params);
#ifdef DEBUG
	debug("TopWidget::executeCommand(): returned from plugin '%s'",
	    name.data());
#endif
    CASE_COMMAND("menu")
	ASSERT(menu);
	if (menu) menu->executeCommand(command);
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
	ASSERT(mainwidget);
        if ((mainwidget) && (mainwidget->executeCommand(command))) {
	    return ;
	}
    }

}

//*****************************************************************************
void TopWidget::loadBatch(const char *str)
{
    Parser parser(str);
    FileLoader loader(parser.getFirstParam());
    parseCommands(loader.getMem());
}

//*****************************************************************************
SignalManager *TopWidget::getSignalManager()
{
    return (mainwidget) ? mainwidget->getSignalManager() : 0;
}

//*****************************************************************************
void TopWidget::parseCommands(const char *str)
//parses a list a of commands separated by newlines
{
    LineParser lineparser(str);
    const char *line = lineparser.getLine();
    while (line) {
	executeCommand(line);
	line = lineparser.getLine();
    }
}

//*****************************************************************************
void TopWidget::revert()
{
    ASSERT(mainwidget);
    if (signalName.length() && mainwidget) {
	mainwidget->setSignal(signalName);
	bits = mainwidget->getBitsPerSample();
	updateMenu();
    }
}

//*****************************************************************************
void TopWidget::resolution(const char *str)
{
    Parser parser (str);
    int bps = parser.toInt();

    if ( (bps >= 0) && (bps <= 24) && (bps % 8 == 0)) {
	bits = bps;
	debug("bits=%d", bits);    // ###
    }
    else debug ("out of range\n");
}

//*****************************************************************************
void TopWidget::setCaption(char *filename)
{
    const char *old_caption = caption;

    int len = strlen(app.appName().data()) + strlen(filename) + strlen(" - ");
    len ++; // don't forget the terminating zero !
    caption = new char[len];
    if (filename) {
	snprintf(caption, len, "%s - %s",
	    app.appName().data(), filename);
    } else {
	snprintf(caption, len, "%s", app.appName().data());
    }

    KTMainWindow::setCaption(caption);
    if (old_caption) delete[] old_caption;
}

//*****************************************************************************
bool TopWidget::closeFile()
{
    ASSERT(mainwidget);
    if (mainwidget) mainwidget->closeSignal();
    //    if (!mainwidget->close()) {
    //  return false;
    //    }

    signalName = "";
    setCaption(0);
    setZoom(1.0);
    emit sigSignalNameChanged(signalName);
    updateMenu();
    updateToolbar();

    return true;
}

//*****************************************************************************
int TopWidget::loadFile(const char *filename, int type)
{
    ASSERT(mainwidget);

    // abort if new file not valid
    if (!filename) return -1;

    closeFile();    // close the previous file

    signalName = filename;
    emit sigSignalNameChanged(signalName);

    if (mainwidget) mainwidget->setSignal(filename, type);
    app.addRecentFile(signalName);
    setCaption(duplicateString(signalName));

    bits = (mainwidget) ? mainwidget->getBitsPerSample() : 0;
    updateMenu();
    updateToolbar();

    return 0;
}

//*****************************************************************************
void TopWidget::openRecent(const char *str)
{
    Parser parser (str);
    loadFile(parser.getFirstParam(), WAV);
}

//*****************************************************************************
void TopWidget::dropEvent(KDNDDropZone *drop)
{
    ASSERT(drop);
    if (!drop) return;

    QStrList &list = drop->getURLList();
    char *s = list.getFirst();
    if (s) {
	QString name = s;
	if ( name.left(5) == "file:")
	    loadFile(name.right(name.length() - 5), WAV);
    }
}

//*****************************************************************************
void TopWidget::openFile()
{
    loadFile(KFileDialog::getOpenFileName(0, "*.wav", this), WAV);
}

//*****************************************************************************
void TopWidget::importAsciiFile()
{
    loadFile(KFileDialog::getOpenFileName(0, "*.*", this), ASCII);
}

//*****************************************************************************
void TopWidget::exportAsciiFile()
{
    QString name = KFileDialog::getSaveFileName(0, "*.asc", mainwidget);
    if (!name.isNull()) {
	mainwidget->saveSignal(name, bits, ASCII, false);
    }
}

//*****************************************************************************
void TopWidget::saveFile()
{
    ASSERT(mainwidget);
    if (!mainwidget) return;

    if (signalName.length()) {
	mainwidget->saveSignal(signalName, bits, 0, false);
	setCaption(duplicateString(signalName));
	updateMenu();
    } else saveFileAs (false);
}

//*****************************************************************************
void TopWidget::saveFileAs(bool selection)
{
    KFileDialog *dialog;

    if (saveDir)
	dialog = new KFileDialog(saveDir->absPath().data(), "*.wav", this, 0, true);
    else
	dialog = new KFileDialog(0, "*.wav", this, 0, true);
    ASSERT(dialog);

    if (dialog) {
	dialog->exec();
	signalName = dialog->selectedFile();
	emit sigSignalNameChanged(signalName);
	
	if (signalName.length()) {
	    if (saveDir) delete saveDir;
	    saveDir = new QDir(dialog->dirPath());
	    ASSERT(saveDir);

	    mainwidget->saveSignal(signalName, bits, 0, selection);
	    setCaption(duplicateString(signalName));
	    app.addRecentFile(signalName);
	    updateMenu();
	}
	delete dialog;
    }
}

//*****************************************************************************
void TopWidget::setSignal(const char *newname)
{
    ASSERT(newname);
    if (!newname) return;
    loadFile(newname, WAV);
}

//*****************************************************************************
const QString &TopWidget::getSignalName()
{
    return signalName;
}

//*****************************************************************************
void TopWidget::setSignal(SignalManager *signal)
{
    ASSERT(mainwidget);
    if (!mainwidget) return;

    mainwidget->setSignal(signal);
}

//*****************************************************************************
void TopWidget::selectZoom(int index)
{
    ASSERT(mainwidget);
    if (!mainwidget) return;

    if (index < 0) return;
    if ((unsigned int)index >= zoom_factors.count())
	index = zoom_factors.count()-1;

    double new_zoom;
    const char *text = zoom_factors.at(index);
    new_zoom = strtod(text, 0);
    if (new_zoom != 0.0) new_zoom = (100.0 / new_zoom);
    mainwidget->setZoom(new_zoom);
}

//*****************************************************************************
void TopWidget::setZoom(double zoom)
{
//    debug("void TopWidget::setZoom(%0.5f)", zoom);
    ASSERT(zoom > 0);
    ASSERT(m_zoomselect);

    if (zoom <= 0.0) return; // makes no sense
    if (!m_zoomselect) return;

    double percent = (double)100.0 / zoom;
    char buf[256];
    buf[0] = 0;

    if (mainwidget) {
	if (mainwidget->getChannelCount() != 0)
	    KwavePlugin::zoom2string(buf,sizeof(buf),percent);

	mainwidget->setZoom(zoom);
    }
    (strlen(buf)) ? m_zoomselect->setText(buf) : m_zoomselect->clearEdit();
}

//*****************************************************************************
void TopWidget::updateRecentFiles()
{
    ASSERT(menu);
    if (!menu) return;

    menu->clearNumberedMenu("ID_FILE_OPEN_RECENT");
    for (unsigned int i = 0 ; i < recentFiles.count(); i++)
	menu->addNumberedMenuEntry("ID_FILE_OPEN_RECENT",
	    recentFiles.at(i));
}

//*****************************************************************************
void TopWidget::updateMenu()
{
    ASSERT(menu);
    if (!menu) return;

    char buffer[128];
    const char *format = "ID_FILE_SAVE_RESOLUTION_%d";
    snprintf(buffer, sizeof(buffer), format, bits);
    menu->selectItem("@BITS_PER_SAMPLE", (const char *)buffer);
}

//*****************************************************************************
void TopWidget::updateToolbar()
{
    ASSERT(m_toolbar);
    ASSERT(mainwidget);
    if (!m_toolbar) return;
    if (!mainwidget) return;

    bool have_signal = mainwidget && (mainwidget->getChannelCount());
    bool playing = mainwidget->playbackController()->running();
    bool paused  = mainwidget->playbackController()->paused();

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

    m_toolbar->setItemEnabled(m_id_zoomrange, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomin, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomout, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomnormal, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomall, have_signal);
    m_toolbar->setItemEnabled(m_id_zoomselect, have_signal);
}

//*****************************************************************************
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

//*****************************************************************************
void TopWidget::blinkPause()
{
    ASSERT(m_toolbar);
    if (!m_toolbar) return;

    m_toolbar->setButtonPixmap(m_id_pause, m_blink_on ? xpm_pause2 : xpm_pause);
    m_blink_on = !m_blink_on;
}

//*****************************************************************************
void TopWidget::pausePressed()
{
    ASSERT(mainwidget);
    if (!mainwidget) return;

    bool have_signal = mainwidget->getChannelCount();
    bool playing = mainwidget->playbackController()->running();

    if (!have_signal) return;
    if (!mainwidget->playbackController()) return;

    if (playing) {
	mainwidget->playbackController()->playbackPause();
    } else {
	mainwidget->playbackController()->playbackContinue();
    }

}

//*****************************************************************************
//*****************************************************************************
