
#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <kapp.h>
#include <qkeycode.h>
#include <qdir.h>
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

#include "sampleop.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "MainWidget.h"
#include "TopWidget.h"
#include "PluginManager.h"

extern Global globals;

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (matchCommand(command, x)) {

//*****************************************************************************
TopWidget::TopWidget(KwaveApp &main_app, QStrList &recent_files)
    :KTMainWindow(),
    app(main_app),
    recentFiles(recent_files)
{
//    debug("TopWidget::TopWidget()");
    bits = 16;
    caption = 0;
    dropZone = 0;
    loadDir = 0;
    mainwidget = 0;
    menu = 0;
    menu_bar = 0;
    signalName = "";
    saveDir = 0;
    status_bar = 0;

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

//    KToolBar *buttons = new KToolBar(this); // , i18n("playback"));
//    ASSERT(buttons);
//    if (!buttons) return;
//    buttons->setBarPos(KToolBar::Bottom);
//
//    QPixmap *pix = new QPixmap("/opt/kde/share/toolbar/forward.xpm");
//    /*playbutton = */buttons->insertButton(*pix, (int)0); // , i18n("Play"));
//
//    this->addToolBar(buttons);

    // set the MainWidget as the main view
    setView(mainwidget);

    // limit the window to a reasonable minimum size
    int w = mainwidget->minimumSize().width();
    int h = max(mainwidget->minimumSize().height(), 150);
    setMinimumSize(w, h);

    // Find out the width for which the menu bar would only use
    // one line. This is tricky because sizeHint().width() just
    // returns -1 :-(     -> just try and find out...
    int wmax = w*10;
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
    w = max(w, mainwidget->minimumSize().width());
    w = max(w, mainwidget->sizeHint().width());
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

    return ( menu && menu_bar && mainwidget &&
             dropZone && plugin_manager && status_bar );
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

    if (app.executeCommand(command)) {
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
    emit sigSignalNameChanged(signalName);
    updateMenu();

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
    loadFile(KFileDialog::getOpenFileName(0, "*.*", this), WAV);
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
//*****************************************************************************
