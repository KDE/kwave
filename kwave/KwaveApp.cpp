
#include "config.h"
#include <qdir.h>
#include <qlist.h>
#include <qstring.h>
#include <qstrlist.h>

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kcrash.h>
#include <kglobal.h>
#include <kmainwindow.h>
#include <kstddirs.h>

#include <libkwave/Parser.h>

#include "ClipBoard.h"
#include "PluginManager.h"
#include "SignalManager.h"
#include "TopWidget.h"
#include "KwaveApp.h"

//***************************************************************************
playback_param_t KwaveApp::playback_params = {
    44100,
    2,
    16,
    0,
    5
};

//***************************************************************************
static ClipBoard _clipboard;
ClipBoard &KwaveApp::m_clipboard(_clipboard);

//***************************************************************************
KwaveApp::KwaveApp()
#ifdef UNIQUE_APP
   :KUniqueApplication(),
#else // UNIQUE_APP
   :KApplication(),
#endif // UNIQUE_APP
    recentFiles(true),
    topwidgetlist()
{
//    KCrash::setCrashHandler(0);

    debug("KwaveApp::KwaveApp() -- 1 --"); // ###

    playback_params.rate = 44100;
    playback_params.channels = 2;
    playback_params.bits_per_sample = 16;
    playback_params.device = "/dev/dsp";
    playback_params.bufbase = 5;

    topwidgetlist.setAutoDelete(false);
    recentFiles.setAutoDelete(true);

    debug("KwaveApp::KwaveApp() -- 2 --"); // ###
    readConfig();

    // load the list of plugins
    PluginManager::findPlugins();
    debug("KwaveApp::KwaveApp() -- 3 --"); // ###

#ifndef UNIQUE_APP
    newInstance();
#endif
    debug("KwaveApp::KwaveApp() -- 4 --"); // ###
}

//***************************************************************************
int KwaveApp::newInstance()
{
    debug("KwaveApp::newInstance() -- 1 --"); // ###
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    unsigned int argc = (args) ? args->count() : 0;

    // only one parameter -> open with empty window
    if (argc == 0) {
	debug("KwaveApp::newInstance() -- 2 --"); // ###
	newWindow();
	debug("KwaveApp::newInstance() -- 3 --"); // ###
    } else {
	// open a window for each file specified in the
	// command line an load it
	QString filename;
	for (unsigned int i = 0; i < argc; i++) {
	debug("KwaveApp::newInstance() -- 4 --"); // ###
	    filename = QFile::decodeName(args->arg(i));
	    debug("KwaveApp:newInstance(): file=`%s`",filename.data());
	    newWindow(&filename);
	    debug("KwaveApp::newInstance() -- 5 --"); // ###
	}
    }
    debug("KwaveApp::newInstance() -- 6 --"); // ###
    if (args) args->clear();
    debug("KwaveApp::newInstance() -- 7 --"); // ###

    return 0;
}

//***************************************************************************
bool KwaveApp::isOK()
{
    ASSERT(!topwidgetlist.isEmpty());
    return (!topwidgetlist.isEmpty());
}

//***************************************************************************
ClipBoard &KwaveApp::clipboard()
{
    return m_clipboard;
}

//***************************************************************************
bool KwaveApp::executeCommand(const QString &command)
{
//    debug("KwaveApp::executeCommand(%s)", command);    // ###
    Parser parser(command);
    if (parser.command() == "newwindow") {
	newWindow();
    } else if (parser.command() == "help") {
	invokeHTMLHelp("kwave/index.html", "");
    } else {
	return false;
    }
    return true;
}

//***************************************************************************
void KwaveApp::addRecentFile(const QString &newfile)
{
//    debug("KwaveApp::addRecentFile(%s)", newfile);

    int old = recentFiles.find(newfile);

    // remove old entry if present
    if (old != -1) recentFiles.remove(old);

    // shorten the list down to 19 entries
    while (recentFiles.count() > 19)
	recentFiles.remove(19);

    // insert the new entry at top
    recentFiles.insert(0, newfile);

    // save the list of recent files
    saveRecentFiles();

    // update all toplevel widgets
    emit recentFilesChanged();
}

//*****************************************************************************
bool KwaveApp::newWindow(const QString *filename)
{
    debug("KwaveApp::newWindow(const QString *filename) -- 1 --"); // ###
    TopWidget *new_top_widget = new TopWidget(*this, recentFiles);
    ASSERT(new_top_widget);
    if (!new_top_widget) return false;

    debug("KwaveApp::newWindow(const QString *filename) -- 2 --"); // ###
    if ( !(new_top_widget->isOK()) ) {
	debug("KwaveApp::newWindow() failed!");
	delete new_top_widget;
	return false;
    }

    debug("KwaveApp::newWindow(const QString *filename) -- 3 --"); // ###
    if (topwidgetlist.isEmpty()) {
	// the first widget is the main widget !
	setMainWidget(new_top_widget); // sets geometry and other properties
	setMainWidget(0);              // that's enough, dont quit on close !
    } else {
	// create a new widget with the same geometry as
	// the last created one
	const QRect &geom = topwidgetlist.last()->geometry();
	// tnew->setGeometry(geom); // would overlap :-(
	new_top_widget->resize(geom.width(), geom.height());
    }
    debug("KwaveApp::newWindow(const QString *filename) -- 4 --"); // ###

    topwidgetlist.append(new_top_widget);
    debug("KwaveApp::newWindow(const QString *filename) -- 5 --"); // ###
    new_top_widget->show();

    // inform the widget about changes in the list of recent files
    connect(this, SIGNAL(recentFilesChanged()),
            new_top_widget, SLOT(updateRecentFiles()));

    debug("KwaveApp::newWindow(const QString *filename) -- 6 --"); // ###
    if (filename) new_top_widget->setSignal(*filename);
    debug("KwaveApp::newWindow(const QString *filename) -- 7 --"); // ###

    return true;
}

//*****************************************************************************
bool KwaveApp::closeWindow(TopWidget *todel)
{
    debug("KwaveApp::closeWindow(TopWidget *todel)");
    ASSERT(todel);

    // save the configuration, including the list of recent files
    saveConfig();

    topwidgetlist.setAutoDelete(false);
    if (todel) topwidgetlist.removeRef(todel);

    //if list is empty -> no more windows there -> exit application
    return (topwidgetlist.isEmpty());
}

//*****************************************************************************
void KwaveApp::saveRecentFiles()
{
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup ("Recent Files");

    QString num;
    for (unsigned int i = 0 ; i < recentFiles.count(); i++) {
	num.setNum(i);
	cfg->writeEntry(num, recentFiles.at(i));
    }

    cfg->sync();
}

//*****************************************************************************
void KwaveApp::saveConfig()
{
//    char buf[256];
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    // playback settings
    cfg->setGroup("Playback Settings");
    cfg->writeEntry("SampleRate", playback_params.rate);
    cfg->writeEntry("Channels", playback_params.channels);
    cfg->writeEntry("BitsPerSample", playback_params.bits_per_sample);
    cfg->writeEntry("Device", playback_params.device);
    cfg->writeEntry("BufferBase", playback_params.bufbase);

//    cfg->setGroup("Memory Settings");
//    cfg->writeEntry("Mmap threshold", mmap_threshold);
//    cfg->writeEntry("Mmap dir", mmap_dir);

////    cfg->setGroup ("Labels");
////    for (unsigned int i = 0 ; i < globals.markertypes.count(); i++) {
////	snprintf(buf, sizeof(buf), "%dCommand", i);
////	cfg->writeEntry (buf, globals.markertypes.at(i)->getCommand());
////    }

    cfg->sync();

    // also save the list of recent files
    saveRecentFiles();
}

//*****************************************************************************
// reads user config via KConfig, sets global variables accordingly
void KwaveApp::readConfig()
{
    char *dummy = new char[256*1024];
//    QString *leak = new QString(dummy);
    int unitialized;
    int x;
    x=unitialized;
    int unused_a;
    int unsued_b;
    int array[2];
    for (int ii=0; ii < 4; ii++) { array[ii] = ii; };
    dummy++;
    dummy++;
    dummy++;
    dummy++;
    dummy++;
    // ###

    QString result;
    char buf[64];

    KConfig *cfg = config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");
    for (unsigned int i = 0 ; i < 20; i++) {
	snprintf(buf, sizeof(buf), "%d", i); //generate number
	result = cfg->readEntry(buf);    //and read corresponding entry
	if (!result.isNull()) {
	    QFile file(result.data());
	    //check if file exists and insert it if not already present
	    if (file.exists() && (recentFiles.find(result.data()) == -1))
		recentFiles.append(result.data());
	}
    }

    // playback settings
    cfg->setGroup("Playback Settings");
    result = cfg->readEntry("SampleRate");
    playback_params.rate = !result.isNull() ? result.toInt() : 44100;
    result = cfg->readEntry("Channels");
    playback_params.channels = !result.isNull() ? result.toInt() : 2;
    result = cfg->readEntry("BitsPerSample");
    playback_params.bits_per_sample = !result.isNull() ? result.toInt() : 16;
    result = cfg->readEntry("Device");
	playback_params.device = (result.length() ? result : QString("/dev/dsp"));
    result = cfg->readEntry("BufferBase");
    playback_params.bufbase = !result.isNull() ? result.toInt() : 5;
    ASSERT(playback_params.device);

    cfg->setGroup ("Memory Settings");
//    result = cfg->readEntry ("Mmap threshold");
//    if (!result.isNull()) mmap_threshold = result.toInt();
//    result = cfg->readEntry ("Mmap dir");
//    if (!result.isNull()) mmap_dir = duplicateString(result.data());
//    mmapallocdir = mmap_dir;

////    cfg->setGroup ("Labels");
////    for (unsigned int i = 0 ; i < 20; i++) {
////	snprintf (buf, sizeof(buf), "%dCommand", i);
////	QString name = cfg->readEntry (buf);
////	if (!name.isEmpty()) {
////	    LabelType *marker = new LabelType(name.data());
////	    ASSERT(marker);
////	    if (marker) globals.markertypes.append(marker);
////	}
////    }

}

//*****************************************************************************
KwaveApp::~KwaveApp()
{
//    debug("KwaveApp::~KwaveApp()");
    saveConfig();

//    debug("KwaveApp::~KwaveApp(): %d topwidgets", topwidgetlist.count());

    topwidgetlist.setAutoDelete(false);
    while (!topwidgetlist.isEmpty()) {
	TopWidget *todel = topwidgetlist.last();
	topwidgetlist.removeRef(todel);
	delete todel;
    }
    recentFiles.clear();

    playback_params.device = QString(0);

//    debug("KwaveApp::~KwaveApp(): done.");
}

//*****************************************************************************
//*****************************************************************************
