
#include "config.h"

#include <qfile.h>
#include <qlist.h>
#include <qstring.h>

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kcrash.h>
#include <kglobal.h>

#include <libkwave/Parser.h>

#include "ClipBoard.h"
#include "PluginManager.h"
#include "TopWidget.h"
#include "KwaveApp.h"

//***************************************************************************
playback_param_t KwaveApp::m_playback_params = {
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
    m_recent_files(),
    m_topwidget_list()
{
    KCrash::setCrashHandler(0); // ###

    m_playback_params.rate = 44100;
    m_playback_params.channels = 2;
    m_playback_params.bits_per_sample = 16;
    m_playback_params.device = "/dev/dsp";
    m_playback_params.bufbase = 5;

    m_topwidget_list.setAutoDelete(false);

    readConfig();

    // load the list of plugins
    PluginManager::findPlugins();

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
	newWindow();
    } else {
	// open a window for each file specified in the
	// command line an load it
	QString filename;
	for (unsigned int i = 0; i < argc; i++) {
	    filename = QFile::decodeName(args->arg(i));
	    newWindow(&filename);
	}
    }
    if (args) args->clear();

    return 0;
}

//***************************************************************************
bool KwaveApp::isOK()
{
    ASSERT(!m_topwidget_list.isEmpty());
    return (!m_topwidget_list.isEmpty());
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
    QStringList::Iterator old = m_recent_files.find(newfile);

    // remove old entry if present
    if (old != 0) m_recent_files.remove(old);

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
bool KwaveApp::newWindow(const QString *filename)
{
    TopWidget *new_top_widget = new TopWidget(*this);
    ASSERT(new_top_widget);
    if (!new_top_widget) return false;

    if ( !(new_top_widget->isOK()) ) {
	warning("KwaveApp::newWindow() failed!");
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

    if (filename) new_top_widget->loadFile(*filename);

    return true;
}

//***************************************************************************
bool KwaveApp::closeWindow(TopWidget *todel)
{
    debug("KwaveApp::closeWindow(TopWidget *todel)");
    ASSERT(todel);

    // save the configuration, including the list of recent files
    saveConfig();

    m_topwidget_list.setAutoDelete(false);
    if (todel) m_topwidget_list.removeRef(todel);

    // if list is empty -> no more windows there -> exit application
    return (m_topwidget_list.isEmpty());
}

//***************************************************************************
void KwaveApp::saveRecentFiles()
{
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");

    QString num;
    for (unsigned int i = 0 ; i < m_recent_files.count(); i++) {
	num.setNum(i);
	cfg->writeEntry(num, m_recent_files[i]);
    }

    cfg->sync();
}

//***************************************************************************
void KwaveApp::saveConfig()
{
    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return;

    // playback settings
    cfg->setGroup("Playback Settings");
    cfg->writeEntry("SampleRate", m_playback_params.rate);
    cfg->writeEntry("Channels", m_playback_params.channels);
    cfg->writeEntry("BitsPerSample", m_playback_params.bits_per_sample);
    cfg->writeEntry("Device", m_playback_params.device);
    cfg->writeEntry("BufferBase", m_playback_params.bufbase);

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

//***************************************************************************
void KwaveApp::readConfig()
{
    QString result;
    QString key;

    KConfig *cfg = config();
    ASSERT(cfg);
    if (!cfg) return;

    cfg->setGroup("Recent Files");
    for (unsigned int i = 0 ; i < 20; i++) {
	key = QString::number(i);        // generate number
	result = cfg->readEntry(key);    // and read corresponding entry
	if (!result.isNull()) {
	    QFile file(result);
	    //check if file exists and insert it if not already present
	    if (file.exists() && (m_recent_files.findIndex(result) == -1))
		m_recent_files.append(result);
	}
    }

    // playback settings
    cfg->setGroup("Playback Settings");
    result = cfg->readEntry("SampleRate");
    m_playback_params.rate = !result.isNull() ? result.toInt() : 44100;
    result = cfg->readEntry("Channels");
    m_playback_params.channels = !result.isNull() ? result.toInt() : 2;
    result = cfg->readEntry("BitsPerSample");
    m_playback_params.bits_per_sample = !result.isNull() ? result.toInt() : 16;
    result = cfg->readEntry("Device");
	m_playback_params.device = (result.length() ? result : QString("/dev/dsp"));
    result = cfg->readEntry("BufferBase");
    m_playback_params.bufbase = !result.isNull() ? result.toInt() : 5;
    ASSERT(m_playback_params.device.length());

//    cfg->setGroup ("Memory Settings");
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
    m_recent_files.clear();
    m_playback_params.device = QString(0);
}

//***************************************************************************
//***************************************************************************
