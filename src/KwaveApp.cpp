
#include "config.h"
#include <qdir.h>
#include <qstring.h>
#include <qstrlist.h>

#include <ktopwidget.h>

#include <libkwave/Global.h>
#include <libkwave/String.h>
#include <libkwave/Parser.h>
#include <libkwave/DynamicLoader.h>

#include "SignalManager.h"
#include "TopWidget.h"
#include "KwaveApp.h"

//*****************************************************************************
//definitions of global variables needed/changed by read/save config routines

struct Global globals;

//extern int mmap_threshold;    //threshold in MB for using mmapping
//extern char *mmap_dir;        //storage of dir name
///* ### extern ### */
//char *mmapallocdir;    //really used directory

playback_param_t KwaveApp::playback_params = {
    44100,
    2,
    16,
    0,
    5
};


//*****************************************************************************
KwaveApp::KwaveApp(int argc, char **argv)
    :KApplication(argc, argv),
    recentFiles(true)
{
    globals.localconfigDir = 0;
    globals.globalconfigDir = 0;
    globals.timeplugins = 0;
    globals.dialogplugins = 0;
    globals.filterDir = 0;

    playback_params.rate = 44100;
    playback_params.channels = 2;
    playback_params.bits_per_sample = 16;
    playback_params.device = duplicateString("/dev/dsp");
    playback_params.bufbase = 5;
    ASSERT(playback_params.device);

    int file;

    // put directory pointers into Global structure
    if (!findDirectories()) return;

    globals.markertypes.setAutoDelete(true);
    globals.markertypes.clear();

    topwidgetlist.setAutoDelete(false);
    recentFiles.setAutoDelete(true);

    readConfig();

    // only one parameter -> open with empty window
    if (argc <= 1) {
	newWindow();
    } else {
	// open a window for each file specified in the
	// command line an load it
	for (file = 1; file < argc; file++) {
	    newWindow(argv[file]);
	}
    }
}

//*****************************************************************************
bool KwaveApp::isOK()
{
    ASSERT(!topwidgetlist.isEmpty());
    ASSERT(globals.localconfigDir);
    ASSERT(globals.globalconfigDir);
    ASSERT(globals.filterDir);

    return (!topwidgetlist.isEmpty() &&
	globals.localconfigDir && globals.globalconfigDir &&
	globals.filterDir );
}

//*****************************************************************************
bool KwaveApp::findDirectories()
{

    QDir localconfig(localkdedir().data());
    if (localconfig.cd("share")) {
	if (!localconfig.cd("apps")) {
	    localconfig.mkdir("apps");
	    localconfig.cd("apps");
	}
	if (!localconfig.cd("kwave")) {
	    localconfig.mkdir("kwave");
	    localconfig.cd("kwave");
	}
	globals.localconfigDir = duplicateString(localconfig.absPath());
    } else warning("no local user kdedir found !\n");

    QDir globalconfig(kde_datadir().data());
    if (!globalconfig.cd("kwave"))
	warning("no global kwave config dir found !\n");
    globals.globalconfigDir = duplicateString(globalconfig.absPath());

    debug("globals.globalconfigDir=%s",globals.globalconfigDir);
    QDir timePluginDir(globals.globalconfigDir);
    if (timePluginDir.cd("modules"))
	if (timePluginDir.cd("time"))
	    globals.timeplugins = DynamicLoader::getPlugins(
				      timePluginDir.absPath().data());

    QDir dialogPluginDir (globals.globalconfigDir);
    if (dialogPluginDir.cd("modules"))
	if (dialogPluginDir.cd("dialogs"))
	    globals.dialogplugins = DynamicLoader::getPlugins(
					dialogPluginDir.absPath().data());

    QDir filter(globals.localconfigDir);
    if (!filter.cd ("presets")) {
	filter.mkdir ("presets");
	filter.cd ("presets");
    }
    if (!filter.cd ("filters")) {
	filter.mkdir ("filters");
	filter.cd ("filters");
    }
    globals.filterDir = duplicateString(filter.absPath());

    return true;
}

//****************************************************************************
bool KwaveApp::executeCommand(const char* command)
{
//    debug("KwaveApp::executeCommand(%s)", command);    // ###

    if (matchCommand(command, "newwindow")) {
	newWindow();
    } else if (matchCommand(command, "help")) {
	invokeHTMLHelp("kwave/index.html", "");
    } else {
	return false;
    }
    return true;
}

//****************************************************************************
void KwaveApp::addRecentFile(const char* newfile)
{
    debug("KwaveApp::addRecentFile(%s)", newfile);

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
bool KwaveApp::newWindow(const char *filename)
{
    TopWidget *new_top_widget = new TopWidget(*this, recentFiles);
    ASSERT(new_top_widget);
    if (!new_top_widget) return false;

    if ( !(new_top_widget->isOK()) ) {
	debug("KwaveApp::newWindow(%s) failed!", filename);
	delete new_top_widget;
	return false;
    }

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

    topwidgetlist.append(new_top_widget);
    new_top_widget->show();

    // inform the widget about changes in the list of recent files
    connect(this, SIGNAL(recentFilesChanged()),
            new_top_widget, SLOT(updateRecentFiles()));

    if (filename) new_top_widget->setSignal(filename);

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
    char buf[256];
    KConfig *config = getConfig();
    ASSERT(config);
    if (!config) return;

    config->setGroup ("Recent Files");

    for (unsigned int i = 0 ; i < recentFiles.count(); i++) {
	snprintf(buf, sizeof(buf), "%d", i);
	config->writeEntry(buf, recentFiles.at(i));
    }

    config->sync();
}

//*****************************************************************************
void KwaveApp::saveConfig()
{
    char buf[256];
    KConfig *config = getConfig();
    ASSERT(config);
    if (!config) return;

    // playback settings
    config->setGroup("Playback Settings");
    config->writeEntry("SampleRate", playback_params.rate);
    config->writeEntry("Channels", playback_params.channels);
    config->writeEntry("BitsPerSample", playback_params.bits_per_sample);
    config->writeEntry("Device", playback_params.device);
    config->writeEntry("BufferBase", playback_params.bufbase);

//    config->setGroup("Memory Settings");
//    config->writeEntry("Mmap threshold", mmap_threshold);
//    config->writeEntry("Mmap dir", mmap_dir);

    config->setGroup ("Labels");
    for (unsigned int i = 0 ; i < globals.markertypes.count(); i++) {
	snprintf(buf, sizeof(buf), "%dCommand", i);
	config->writeEntry (buf, globals.markertypes.at(i)->getCommand());
    }

    config->sync();

    // also save the list of recent files
    saveRecentFiles();
}

//*****************************************************************************
// reads user config via KConfig, sets global variables accordingly
void KwaveApp::readConfig()
{
    QString result;
    char buf[64];

    KConfig *config = getConfig();
    ASSERT(config);
    if (!config) return;

    config->setGroup("Recent Files");
    for (unsigned int i = 0 ; i < 20; i++) {
	snprintf(buf, sizeof(buf), "%d", i); //generate number
	result = config->readEntry(buf);    //and read corresponding entry
	if (!result.isNull()) {
	    QFile file(result.data());
	    //check if file exists and insert it if not already present
	    if (file.exists() && (recentFiles.find(result.data()) == -1))
		recentFiles.append(result.data());
	}
    }

    // playback settings
    if (playback_params.device) delete[] playback_params.device;

    config->setGroup("Playback Settings");
    result = config->readEntry("SampleRate");
    playback_params.rate = !result.isNull() ? result.toInt() : 44100;
    result = config->readEntry("Channels");
    playback_params.channels = !result.isNull() ? result.toInt() : 2;
    result = config->readEntry("BitsPerSample");
    playback_params.bits_per_sample = !result.isNull() ? result.toInt() : 16;
    result = config->readEntry("Device");
	playback_params.device = duplicateString(!result.isNull() ?
	    result.data() : "/dev/dsp");
    result = config->readEntry("BufferBase");
    playback_params.bufbase = !result.isNull() ? result.toInt() : 5;
    ASSERT(playback_params.device);

    config->setGroup ("Memory Settings");
//    result = config->readEntry ("Mmap threshold");
//    if (!result.isNull()) mmap_threshold = result.toInt();
//    result = config->readEntry ("Mmap dir");
//    if (!result.isNull()) mmap_dir = duplicateString(result.data());
//    mmapallocdir = mmap_dir;

    config->setGroup ("Labels");
    for (unsigned int i = 0 ; i < 20; i++) {
	snprintf (buf, sizeof(buf), "%dCommand", i);
	QString name = config->readEntry (buf);
	if (!name.isEmpty()) {
	    LabelType *marker = new LabelType(name.data());
	    ASSERT(marker);
	    if (marker) globals.markertypes.append(marker);
	}
    }

}

//*****************************************************************************
KwaveApp::~KwaveApp()
{
//    debug("KwaveApp::~KwaveApp()");
    ASSERT(KApplication::getKApplication());
    saveConfig();

//    debug("KwaveApp::~KwaveApp(): %d topwidgets", topwidgetlist.count());

    topwidgetlist.setAutoDelete(false);
    while (!topwidgetlist.isEmpty()) {
	TopWidget *todel = topwidgetlist.last();
	topwidgetlist.removeRef(todel);
	delete todel;
    }
    recentFiles.clear();

    if (globals.localconfigDir) delete[] globals.localconfigDir;
    globals.localconfigDir = 0;

    if (globals.globalconfigDir) delete[] globals.globalconfigDir;
    globals.globalconfigDir = 0;

    if (globals.timeplugins) delete[] globals.timeplugins;
    globals.timeplugins = 0;

    if (globals.dialogplugins) delete[] globals.dialogplugins;
    globals.dialogplugins = 0;

    if (globals.filterDir) delete[] globals.filterDir ;
    globals.filterDir = 0;

    if (playback_params.device) delete[] playback_params.device;
    playback_params.device = 0;

//    debug("KwaveApp::~KwaveApp(): done.");
}

//*****************************************************************************
//*****************************************************************************
