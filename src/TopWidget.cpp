#include <stdio.h>
#include <unistd.h>

#include <kapp.h>
#include <qkeycode.h>
#include <qdir.h>
#include <qfiledlg.h>
#include <drag.h>

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

extern Global globals;

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
    debug("TopWidget::TopWidget()");
    bits = 16;
    caption = 0;
    dropZone = 0;
    loadDir = 0;
    mainwidget = 0;
    menu = 0;
    menu_bar = 0;
    name = 0;
    saveDir = 0;
    status_bar = 0;

    menu_bar = new KMenuBar(this);
    ASSERT(menu_bar);
    if (!menu_bar) return;

    menu = new MenuManager(this, *menu_bar);
    ASSERT(menu);
    if (!menu) return;

    status_bar = new KStatusBar(this);
    ASSERT(status_bar);
    if (!status_bar) return;

    status_bar->insertItem (i18n("Length: 0 ms           "), 1);
    status_bar->insertItem (i18n("Rate: 0 kHz         "), 2);
    status_bar->insertItem (i18n("Samples: 0             "), 3);
    status_bar->insertItem (i18n("selected: 0 ms        "), 4);
    status_bar->insertItem (i18n("Clipboard: 0 ms      "), 5);
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

    setView(mainwidget);
    debug("TopWidget::TopWidget(): done.");
}

//*****************************************************************************
bool TopWidget::isOK()
{
    ASSERT(menu);
    ASSERT(menu_bar);
    ASSERT(mainwidget);
    ASSERT(dropZone);
    ASSERT(status_bar);

    return ( menu && menu_bar && mainwidget &&
             dropZone && status_bar );
}

//*****************************************************************************
TopWidget::~TopWidget()
{
    debug("TopWidget::~TopWidget()");
    ASSERT(KApplication::getKApplication());

    closeFile();

    KTMainWindow::setCaption(0);
    if (caption) delete caption;
    caption=0;

    if (loadDir) delete loadDir;
    loadDir=0;

    if (mainwidget) delete mainwidget;
    mainwidget=0;

    if (menu) delete menu;
    menu=0;

    if (menu_bar) delete menu_bar;
    menu_bar=0;

    if (name) deleteString(name);
    name=0;

    if (saveDir) delete saveDir;
    saveDir=0;

    if (status_bar) delete status_bar;
    status_bar=0;

    app.closeWindow(this);

    debug("TopWidget::~TopWidget(): done.");
}

//*****************************************************************************
void TopWidget::executeCommand(const char *command)
{
//    debug("TopWidget::executeCommand(%s)", command); // ###
    ASSERT(command);
    if (!command) return;

    if (app.executeCommand(command)) {
	return ;
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
//void TopWidget::closeEvent(QCloseEvent *e)
//{
//    ASSERT(e);
//    if (e) e->accept();
//    app.closeWindow(this);
//}

//*****************************************************************************
void TopWidget::revert()
{
    ASSERT(mainwidget);
    if (name && mainwidget) {
	mainwidget->setSignal(name);
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
    if (old_caption) delete old_caption;
}

//*****************************************************************************
bool TopWidget::closeFile()
{
    ASSERT(mainwidget);
    if (mainwidget) mainwidget->closeSignal();
    //    if (!mainwidget->close()) {
    //  return false;
    //    }

    if (name) deleteString(name);
    name = 0;
    setCaption(0);
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

    name = duplicateString(filename);
    if (mainwidget) mainwidget->setSignal(filename, type);
    app.addRecentFile(name);
    setCaption(name);

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
    loadFile(QFileDialog::getOpenFileName(0, "*.wav", this), WAV);
}

//*****************************************************************************
void TopWidget::importAsciiFile()
{
    loadFile(QFileDialog::getOpenFileName(0, "*.*", this), WAV);
}

//*****************************************************************************
void TopWidget::exportAsciiFile()
{
    QString name = QFileDialog::getSaveFileName(0, "*.asc", mainwidget);
    if (!name.isNull()) {
	mainwidget->saveSignal(name, bits, ASCII, false);
    }
}

//*****************************************************************************
void TopWidget::saveFile()
{
    ASSERT(mainwidget);
    if (!mainwidget) return;

    if (name) {
	mainwidget->saveSignal(name, bits, 0, false);
	setCaption(name);
	updateMenu();
    } else saveFileAs (false);
}

//*****************************************************************************
void TopWidget::saveFileAs(bool selection)
{
    QFileDialog *dialog;

    if (saveDir)
	dialog = new QFileDialog (saveDir->absPath().data(), "*.wav", this, 0, true);
    else
	dialog = new QFileDialog (this, 0, true);
    ASSERT(dialog);

    if (dialog) {
	dialog->exec();
	name = duplicateString (dialog->selectedFile());
	if (name) {
	    if (saveDir) delete saveDir;
	    saveDir = new QDir(dialog->dirPath());
	    ASSERT(saveDir);

	    mainwidget->saveSignal(name, bits, 0, selection);
	    setCaption (name);
	    app.addRecentFile(name);
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
