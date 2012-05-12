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

#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopWidget>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QMap>
#include <QPixmap>
#include <QSizePolicy>
#include <QStringList>
#include <QTextStream>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <ktoolbar.h>

#include "libkwave/ApplicationContext.h"
#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/KwavePlugin.h" // for some helper functions
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Parser.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"

#include "libgui/MenuManager.h"
#include "libgui/KwaveFileDialog.h"
#include "libgui/SignalWidget.h" // for MouseMode

#include "KwaveApp.h"
#include "KwaveSplash.h"
#include "MainWidget.h"
#include "PlayerToolBar.h"
#include "TopWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {

/** toolbar name: file operations */
#define TOOLBAR_FILE        "MainWidget File"

/** toolbar name: edit operations */
#define TOOLBAR_EDIT        "MainWidget Edit"

/** toolbar name: record and playback */
#define TOOLBAR_RECORD_PLAY "MainWidget Record/Playback"

/** toolbar name: zoom controls */
#define TOOLBAR_ZOOM        "MainWidget Zoom"

/** role value for entries in the zoom combo box, "predefined" flag (bool) */
#define ZOOM_DATA_PREDEFINED (Qt::UserRole + 0)

/** role value for entries in the zoom combo box, "time" in ms (double) */
#define ZOOM_DATA_TIME       (Qt::UserRole + 1)

/** returns the number of elements of an array */
#define ELEMENTS_OF(__x__) (sizeof(__x__) / sizeof(__x__[0]))

//***************************************************************************
//***************************************************************************
TopWidget::TopWidget(Kwave::ApplicationContext &context)
    :KMainWindow(), m_context(context),
     m_main_widget(0), m_toolbar_record_playback(0), m_zoomselect(0),
     m_menu_manager(0), m_action_undo(0), m_action_redo(0),
     m_action_zoomselection(0), m_action_zoomin(0), m_action_zoomout(0),
     m_action_zoomnormal(0), m_action_zoomall(0), m_action_zoomselect(0),
     m_lbl_status_size(0), m_lbl_status_mode(0), m_lbl_status_cursor(0)
{
    // status bar items
    KStatusBar *status_bar = statusBar();
    Q_ASSERT(status_bar);
    if (!status_bar) return;

    QLabel *spacer = new QLabel(this);
    const int frame_style = QFrame::StyledPanel | QFrame::Sunken;
    status_bar->addWidget(spacer);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setFrameStyle(frame_style);
    QSizePolicy policy = spacer->sizePolicy();
    policy.setHorizontalStretch(100);
    spacer->setSizePolicy(policy);
    policy.setHorizontalStretch(0);

    m_lbl_status_cursor = new QLabel(this);
    status_bar->addWidget(m_lbl_status_cursor);
    m_lbl_status_cursor->setSizePolicy(policy);
    m_lbl_status_cursor->setFrameStyle(frame_style);

    m_lbl_status_mode = new QLabel(this);
    status_bar->addWidget(m_lbl_status_mode);
    m_lbl_status_mode->setSizePolicy(policy);
    m_lbl_status_mode->setFrameStyle(frame_style);

    m_lbl_status_size = new QLabel(this);
    status_bar->addWidget(m_lbl_status_size);
    m_lbl_status_size->setSizePolicy(policy);
    m_lbl_status_size->setFrameStyle(frame_style);

    // direct all kind of focus to this window per default
    setFocusPolicy(Qt::WheelFocus);
}

//***************************************************************************
bool TopWidget::init()
{
    KIconLoader icon_loader;
    const int max_s = KIconLoader::SizeEnormous;

    showInSplashSreen(i18n("Loading main menu..."));
    KMenuBar *menubar = menuBar();
    Q_ASSERT(menubar);
    if (!menubar) return false;
    m_menu_manager = new MenuManager(this, *menubar);
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return false;

    // connect clicked menu entries with main communication channel of kwave
    connect(m_menu_manager, SIGNAL(sigMenuCommand(const QString &)),
	    this, SLOT(executeCommand(const QString &)));
    connect(&ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
	    this, SLOT(clipboardChanged(bool)));

    // load the menu from file
    QFile menufile(KStandardDirs::locate("data", "kwave/menus.config"));
    menufile.open(QIODevice::ReadOnly);
    QTextStream stream(&menufile);
    Q_ASSERT(!stream.atEnd());
    if (!stream.atEnd()) parseCommands(stream);
    menufile.close();

    m_main_widget = new MainWidget(this, m_context);
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return false;
    if (!(m_main_widget->isOK())) {
	qWarning("TopWidget::TopWidget(): failed at creating main widget");
	delete m_main_widget;
	m_main_widget = 0;
	return false;
    }

    // connect the main widget
    connect(m_main_widget, SIGNAL(sigCommand(const QString &)),
            this, SLOT(executeCommand(const QString &)));
    connect(&m_context.signalManager()->playbackController(),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this, SLOT(updatePlaybackPos(sample_index_t)));
    connect(&m_context.signalManager()->playbackController(),
            SIGNAL(sigSeekDone(sample_index_t)),
            m_main_widget, SLOT(scrollTo(sample_index_t)));

    // connect the sigCommand signal to ourself, this is needed
    // for the plugins
    connect(this, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(executeCommand(const QString &)));

    // --- set up the toolbar ---

    showInSplashSreen(i18n("Initializing toolbar..."));
    KToolBar *toolbar_file = toolBar(TOOLBAR_FILE);
    Q_ASSERT(toolbar_file);
    if (!toolbar_file) return false;

    // --- file open and save ---

    toolbar_file->addAction(
	icon_loader.loadIcon("document-new", KIconLoader::Toolbar),
	i18n("Create a new empty file"),
	this, SLOT(toolbarFileNew()));

    toolbar_file->addAction(
	icon_loader.loadIcon("document-open", KIconLoader::Toolbar),
	i18n("Open an existing file"),
	this, SLOT(toolbarFileOpen()));

    toolbar_file->addAction(
	icon_loader.loadIcon("document-save", KIconLoader::Toolbar),
	i18n("Save the current file"),
	this, SLOT(toolbarFileSave()));

    // --- edit, cut&paste ---

    KToolBar *toolbar_edit = toolBar(TOOLBAR_EDIT);
    Q_ASSERT(toolbar_edit);
    if (!toolbar_edit) return false;

    m_action_undo = toolbar_edit->addAction(
	icon_loader.loadIcon("edit-undo", KIconLoader::Toolbar),
	i18n("Undo"),
	this, SLOT(toolbarEditUndo()));

    m_action_redo = toolbar_edit->addAction(
	icon_loader.loadIcon("edit-redo", KIconLoader::Toolbar),
	i18n("Redo"),
	this, SLOT(toolbarEditRedo()));

    toolbar_edit->addAction(
	icon_loader.loadIcon("edit-cut", KIconLoader::Toolbar),
	i18n("Cut"),
	this, SLOT(toolbarEditCut()));

    toolbar_edit->addAction(
	icon_loader.loadIcon("edit-copy", KIconLoader::Toolbar),
	i18n("Copy"),
	this, SLOT(toolbarEditCopy()));

    QAction *btPaste = toolbar_edit->addAction(
	icon_loader.loadIcon("edit-paste", KIconLoader::Toolbar),
	i18n("Insert"),
	this, SLOT(toolbarEditPaste()));
    btPaste->setEnabled(!ClipBoard::instance().isEmpty());
    connect(&ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
            btPaste, SLOT(setEnabled(bool)));

    toolbar_edit->addAction(
	icon_loader.loadIcon("draw-eraser", KIconLoader::Toolbar),
	i18n("Mute selection"),
	this, SLOT(toolbarEditErase()));

    toolbar_edit->addAction(
	icon_loader.loadIcon("edit-delete", KIconLoader::Toolbar),
	i18n("Delete selection"),
	this, SLOT(toolbarEditDelete()));

//                  Zoom
//                  Previous Page/Back
//                  Next Page/Forward
//                  Go To Page/Home

//                  Help

    // --- record/playback controls ---

    m_toolbar_record_playback = new Kwave::PlayerToolBar(
	this, TOOLBAR_RECORD_PLAY,
	m_context.signalManager()->playbackController(),
	*m_menu_manager);

    Q_ASSERT(m_toolbar_record_playback);
    if (!m_toolbar_record_playback) return false;

    connect(m_toolbar_record_playback, SIGNAL(sigCommand(const QString &)),
            this,                    SLOT(executeCommand(const QString &)));
    connect(m_main_widget, SIGNAL(sigVisibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)),
	    m_toolbar_record_playback, SLOT(visibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)) );

    // --- zoom controls ---

    KToolBar *toolbar_zoom = toolBar(TOOLBAR_ZOOM);
    Q_ASSERT(toolbar_zoom);
    if (!toolbar_zoom) return false;

    m_action_zoomselection = toolbar_zoom->addAction(
	icon_loader.loadIcon("kwave_viewmag", KIconLoader::Toolbar, max_s),
	i18n("Zoom to selection"),
	m_main_widget, SLOT(zoomSelection()));

    m_action_zoomin = toolbar_zoom->addAction(
	icon_loader.loadIcon("kwave_zoom_in", KIconLoader::Toolbar, max_s),
	i18n("Zoom in"),
	m_main_widget, SLOT(zoomIn()));

    m_action_zoomout = toolbar_zoom->addAction(
	icon_loader.loadIcon("kwave_zoom_out", KIconLoader::Toolbar, max_s),
	i18n("Zoom out"),
	m_main_widget, SLOT(zoomOut()));

    m_action_zoomnormal = toolbar_zoom->addAction(
	icon_loader.loadIcon("kwave_zoom_original", KIconLoader::Toolbar, max_s),
	i18n("Zoom to 100%"),
	m_main_widget, SLOT(zoomNormal()));

    m_action_zoomall = toolbar_zoom->addAction(
	icon_loader.loadIcon("kwave_viewmagfit", KIconLoader::Toolbar, max_s),
	i18n("Zoom to all"),
	m_main_widget, SLOT(zoomAll()));

    // zoom selection combo box
    m_zoomselect = new KComboBox(this);
    Q_ASSERT(m_zoomselect);
    if (!m_zoomselect) return false;
    m_zoomselect->setToolTip(i18n("Select zoom factor"));
    m_zoomselect->setInsertPolicy(QComboBox::InsertAtTop);
    m_zoomselect->setEditable(false);

    /** Initialized list of zoom factors */
    struct {
	const QString text;
	unsigned int ms;
    } zoom_factors[] = {
	{ i18n("%1 ms",   1),            1L},
	{ i18n("%1 ms",  10),           10L},
	{ i18n("%1 ms", 100),          100L},
	{ i18n("%1 sec",  1),         1000L},
	{ i18n("%1 sec", 10),     10L*1000L},
	{ i18n("%1 sec", 30),     30L*1000L},
	{ i18n("%1 min",  1),  1L*60L*1000L},
	{ i18n("%1 min",  3),  3L*60L*1000L},
	{ i18n("%1 min",  5),  5L*60L*1000L},
	{ i18n("%1 min", 10), 10L*60L*1000L},
	{ i18n("%1 min", 30), 30L*60L*1000L},
	{ i18n("%1 min", 60), 60L*60L*1000L},
    };

    for (unsigned int i = 0; i < ELEMENTS_OF(zoom_factors); i++) {
	m_zoomselect->addItem(zoom_factors[i].text);
	int index = m_zoomselect->count() - 1;
	unsigned int time = zoom_factors[i].ms;
	m_zoomselect->setItemData(index, QVariant(true), ZOOM_DATA_PREDEFINED);
	m_zoomselect->setItemData(index, QVariant(time), ZOOM_DATA_TIME);
    }

    m_action_zoomselect = toolbar_zoom->addWidget(m_zoomselect);
    connect(m_zoomselect, SIGNAL(activated(int)),
	    this, SLOT(selectZoom(int)));
    connect(m_main_widget, SIGNAL(sigZoomChanged(double)),
            this, SLOT(setZoomInfo(double)));
    int h = m_zoomselect->sizeHint().height();
    m_zoomselect->setMinimumWidth(h * 5);
    m_zoomselect->setFocusPolicy(Qt::FocusPolicy(Qt::ClickFocus | Qt::TabFocus));

    // connect the signal manager
    SignalManager *signal_manager = m_context.signalManager();
    connect(&(signal_manager->selection()),
            SIGNAL(changed(sample_index_t, sample_index_t)),
            this,
            SLOT(selectionChanged(sample_index_t,sample_index_t)));
    connect(signal_manager, SIGNAL(sigUndoRedoInfo(const QString&,
                                                   const QString&)),
            this, SLOT(setUndoRedoInfo(const QString&, const QString&)));
    connect(signal_manager, SIGNAL(sigModified(bool)),
            this,           SLOT(modifiedChanged(bool)));
    connect(signal_manager, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            this,           SLOT(metaDataChanged(Kwave::MetaDataList)));
    connect(signal_manager, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            m_toolbar_record_playback,
                            SLOT(metaDataChanged(Kwave::MetaDataList)));

    // connect the plugin manager
    Kwave::PluginManager *plugin_manager = m_context.pluginManager();
    connect(plugin_manager, SIGNAL(sigCommand(const QString &)),
            this,           SLOT(executeCommand(const QString &)));
    connect(plugin_manager, SIGNAL(sigProgress(const QString &)),
            this,           SLOT(showInSplashSreen(const QString &)));

    showInSplashSreen(i18n("Scanning plugins..."));
    plugin_manager->findPlugins();

    // set the MainWidget as the main view
    setCentralWidget(m_main_widget);

    // set a nice initial size
    int w = m_main_widget->minimumSize().width();
    w = qMax(w, m_main_widget->sizeHint().width());
    w = qMax(w, width());
    h = qMax(m_main_widget->sizeHint().height(), (w * 6) / 10);
    h = qMax(h, height());
    resize(w, h);

    metaDataChanged(Kwave::MetaData());
    setUndoRedoInfo(0,0);
    selectionChanged(0,0);
    updateMenu();
    updateToolbar();
    updateRecentFiles();

    // now we are initialized, load all plugins now
    showInSplashSreen(i18n("Loading plugins..."));
    statusBar()->showMessage(i18n("Loading plugins..."));
    plugin_manager->loadAllPlugins();
    statusBar()->showMessage(i18n("Ready"), 1000);

    updateMenu();

    // make sure that everything of our window is visible
    QRect desk = qApp->desktop()->rect();
    QRect g    = this->geometry();
    if (!desk.contains(g)) {
	// KDE's stupid geometry management has failed ?
	// this happens when one passes "-geometry <WIDTH>x<HEIGTH>" without
	// specifying a target position!?
	// passing "-geometry <WIDTH>x<HEIGHT>-<LEFT>-<TOP>" works...
	g = desk.intersect(g);
	setGeometry(g);
    }

    // enable saving of window size and position for next startup
    setAutoSaveSettings();

    // workaround for KDE4: detect first startup and set all toolbars
    // to "only symbols" mode
    KConfigGroup cfg = KGlobal::config()->group("MainWindow");
    QString magic = "2";
    if (cfg.readEntry("toolbars") != magic) {
	qDebug("toolbar layout changed => resetting toolbars to defaults");

	// unlock all toolbars
	KToolBar::setToolBarsLocked(false);

	// reset all toolbars to default layout
	resetToolbarToDefaults();

	cfg.writeEntry("toolbars", magic);
    }

    // make sure we have the focus, not the zoom combo box
    m_zoomselect->clearFocus();
    setFocus();

    return true;
}

//***************************************************************************
TopWidget::~TopWidget()
{

    // close the current file (no matter what the user wants)
    closeFile();

    if (m_main_widget) delete m_main_widget;
    m_main_widget = 0;

    if (m_menu_manager) delete m_menu_manager;
    m_menu_manager = 0;

    m_context.application().closeWindow(this);
}

//***************************************************************************
int TopWidget::executeCommand(const QString &line)
{
    int result = 0;
    bool use_recorder = true;
    QString command = line;

//    qDebug("TopWidget::executeCommand(%s)", command.toLocal8Bit().data());
    if (!command.length()) return 0; // empty line -> nothing to do
    if (command.trimmed().startsWith("#")) return 0; // only a comment

    Kwave::PluginManager *plugin_manager = m_context.pluginManager();

    // special case: if the command contains ";" it is a list of
    // commands -> macro !
    Parser parse_list(command);
    if (parse_list.hasMultipleCommands()) {
	QStringList macro = parse_list.commandList();
	foreach (const QString &it, macro) {
	    result = executeCommand("nomacro:" + it);
	    Q_ASSERT(!result);
	    if (result) {
		qWarning("macro execution of '%s' failed: %d",
		         it.toLocal8Bit().data(), result);
		return result; // macro failed :-(
	    }

	    // wait until the command has completed !
	    Q_ASSERT(plugin_manager);
	    if (plugin_manager) plugin_manager->sync();
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

    // exclude menu commands from the recorder
    if (parser.command() == "menu") use_recorder = false;

    // only record plugin:execute, not plugin without parameters
    if (parser.command() == "plugin") use_recorder = false;

    // playback commands are always possible
    if ( (parser.command() == "playback") && (m_toolbar_record_playback) )
    {
	return m_toolbar_record_playback->executeCommand(parser.firstParam());
    }

    // let through all commands that handle zoom/view or playback like fwd/rew
    bool allow_always =
	parser.command().startsWith("view:") ||
	parser.command().startsWith("playback:") ||
	parser.command().startsWith("select_track:") ||
	(parser.command() == "close") ||
	(parser.command() == "quit")
	;

    // all others only if no plugin is currently running
    if (!allow_always && plugin_manager && plugin_manager->onePluginRunning())
    {
	qWarning("TopWidget::executeCommand('%s') - currently not possible, "\
		 "a plugin is running :-(",
		 parser.command().toLocal8Bit().data());
	return -1;
    }

    if (use_recorder) {
	// append the command to the macro recorder
	// @TODO macro recording...
	qDebug("# %s ", command.toLocal8Bit().data());
    }

    if (m_context.application().executeCommand(command)) {
	return 0;
    CASE_COMMAND("about_kde")
	// Help / About KDE
	KHelpMenu *dlg = new KHelpMenu(this, "Kwave");
	if (dlg) dlg->aboutKDE();
    CASE_COMMAND("plugin")
	QString name = parser.firstParam();
	QStringList params;

	int cnt=parser.count();
	if (cnt > 1) {
	    while (cnt--) {
		const QString &par = parser.nextParam();
		qDebug("TopWidget::executeCommand(): %s",
		    par.toLocal8Bit().data());
		params.append(par);
	    }
	}
	qDebug("TopWidget::executeCommand(): loading plugin '%s'",
	       name.toLocal8Bit().data());
	qDebug("TopWidget::executeCommand(): with %d parameter(s)",
		params.count());
	Q_ASSERT(plugin_manager);
	if (plugin_manager)
	    result = plugin_manager->executePlugin(name,
		params.count() ? &params : 0);
    CASE_COMMAND("plugin:execute")
	QStringList params;
	int cnt = parser.count();
	QString name(parser.firstParam());
	while (--cnt > 0) {
	    params.append(parser.nextParam());
	}
	Q_ASSERT(plugin_manager);
	result = (plugin_manager) ?
	          plugin_manager->executePlugin(name, &params) :
	          -ENOMEM;
    CASE_COMMAND("plugin:setup")
	QString name(parser.firstParam());
	Q_ASSERT(plugin_manager);
	if (plugin_manager) result = plugin_manager->setupPlugin(name);
    CASE_COMMAND("menu")
	Q_ASSERT(m_menu_manager);
	if (m_menu_manager) /*result = */m_menu_manager->executeCommand(command);
    CASE_COMMAND("newsignal")
	sample_index_t samples = parser.toUInt();
	double         rate    = parser.toDouble();
	unsigned int   bits    = parser.toUInt();
	unsigned int   tracks  = parser.toUInt();
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
    CASE_COMMAND("save")
	result = saveFile();
    CASE_COMMAND("close")
	result = closeFile();
    CASE_COMMAND("revert")
	result = revert();
    CASE_COMMAND("saveas")
	result = saveFileAs(false);
    CASE_COMMAND("loadbatch")
	result = loadBatch(parser.nextParam());
    CASE_COMMAND("saveselect")
	result = saveFileAs(true);
    CASE_COMMAND("quit")
	result = (close()) ? 0 : -1;
    CASE_COMMAND("reset_toolbars")
	if ((result = (Kwave::MessageBox::questionYesNo(this,
	    i18n("Reset the toolbar to default settings?"))
	    == KMessageBox::Yes) ? 1 : 0))
	{
	    resetToolbarToDefaults();
	}
    } else {
	// try to forward the command to the main widget
	Q_ASSERT(m_main_widget);
	return (m_main_widget) ? m_main_widget->executeCommand(command) : -1;
    }

    return result;
}

//***************************************************************************
int TopWidget::loadBatch(const KUrl &url)
{
    // open the URL, read-only mode is enough
    QFile file(url.path());
    if (!file.open(QIODevice::ReadOnly)) {
	qWarning("unable to open source in read-only mode!");
	return -EIO;
    }

    // use a text stream for parsing the commands
    QTextStream stream(&file);
    int result = parseCommands(stream);

    file.close();

    // successful run -> add URL to recent files
    m_context.application().addRecentFile(url.path());
    updateMenu();
    updateToolbar();

    return result;
}

//***************************************************************************
int TopWidget::parseCommands(QTextStream &stream)
{
    int result = 0;
    QMap<QString, qint64> labels;

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    while (!stream.atEnd() && !result) {
	QString line = stream.readLine().simplified();
	if (line.startsWith("#")) continue; // skip comments
	if (!line.length()) continue;       // skip empty lines

	// remove stuff after the "#'" (comments)
	if (line.contains('#')) {
	}

	if (line.endsWith(':')) {
	    // this line seems to be a "label"
	    line = line.left(line.length() - 1).simplified();
	    if (!labels.contains(line)) {
		qDebug("new label '%s' at %u", line.toLocal8Bit().data(),
		       static_cast<unsigned int>(stream.pos()));
		labels[line] = stream.pos();
	    }
	    continue;
	}

	Parser parser(line);

	// the "goto" command
	if (line.split(' ').at(0) == "goto") {
	    qDebug(">>> detected 'goto'");
	    QString label = line.split(' ').at(1).simplified();
	    if (labels.contains(label)) {
		qDebug(">>> goto '%s' @ offset %u", label.toLocal8Bit().data(),
		       static_cast<unsigned int>(labels[label]));
		stream.seek(labels[label]);
	    } else {
		qWarning("label '%s' not found", label.toLocal8Bit().data());
		break;
	    }
	    continue;
	}

	// synchronize before the command
	if (m_context.pluginManager()) m_context.pluginManager()->sync();

	// the "msgbox" command (useful for debugging)
	if (parser.command() == "msgbox") {
	    QApplication::restoreOverrideCursor();
	    result = (Kwave::MessageBox::questionYesNo(this,
		parser.firstParam()) == KMessageBox::Yes) ? 0 : 1;
	    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	    continue;
	}

	// prevent this command from being re-added to the macro recorder
	if (!line.startsWith("nomacro:", Qt::CaseInsensitive))
	    line = "nomacro:" + line;

	// emit the command
	result = executeCommand(line);
// 	qDebug(">>> '%s' - result=%d", line.toLocal8Bit().data(), result);

	// synchronize after the command
	if (m_context.pluginManager()) m_context.pluginManager()->sync();

	// special handling of the "quit" command
	if (parser.command() == "quit") {
	    break;
	}
    }

    // remove hourglass
    QApplication::restoreOverrideCursor();

    return result;
}

//***************************************************************************
int TopWidget::revert()
{
    KUrl url(signalName());
    if (!url.isValid()) return -EINVAL;

    return loadFile(url);
}

//***************************************************************************
bool TopWidget::closeFile()
{
    SignalManager        *signal_manager = m_context.signalManager();
    Kwave::PluginManager *plugin_manager = m_context.pluginManager();

    if (plugin_manager && !plugin_manager->canClose())
    {
	qWarning("TopWidget::closeFile() - currently not possible, "\
	         "a plugin is running :-(");
	return false;
    }

    if (signal_manager && signal_manager->isModified()) {
	int res =  Kwave::MessageBox::warningYesNoCancel(this,
	    i18n("This file has been modified.\nDo you want to save it?"));
	if (res == KMessageBox::Cancel) return false;
	if (res == KMessageBox::Yes) {
	    // user decided to save
	    res = saveFile();
	    qDebug("TopWidget::closeFile()::saveFile, res=%d",res);
	    if (res) return false;
	}
    }

    // close all plugins that still might use the current signal
    if (plugin_manager) {
	plugin_manager->stopAllPlugins();
	plugin_manager->signalClosed();
    }

    signal_manager->close();

    updateCaption();
    if (m_zoomselect) m_zoomselect->clearEditText();
    emit sigSignalNameChanged(signalName());

    updateMenu();
    updateToolbar();
    metaDataChanged(Kwave::MetaData());

    return true;
}

//***************************************************************************
int TopWidget::loadFile(const KUrl &url)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return -1;

    // abort if new file not valid and local
    if (!url.isLocalFile()) return -1;

    // detect whether it is a macro (batch) file
    QFileInfo file(url.fileName());
    QString suffix = file.suffix();
    if (suffix == "kwave") {
	return loadBatch(url);
    }

    // try to close the previous file
    if (!closeFile()) return -1;

    emit sigSignalNameChanged(url.path());

    int res = -ENOMEM;
    if (signal_manager && !(res = signal_manager->loadFile(url))) {
	// succeeded
	updateCaption();

	// enable revert after successful load
	m_menu_manager->setItemEnabled("ID_FILE_REVERT", true);
    } else {
	qWarning("TopWidget::loadFile() failed: result=%d", res);
	QString reason;
	switch (res) {
	    case -ENOMEM:
		reason = i18n("Out of memory");
		break;
	    case -EIO:
		reason = i18nc("error message after opening a file failed",
			       "Unable to open '%1'", url.prettyUrl());
		break;
	    case -EINVAL:
		reason = i18nc("error message after opening a file failed",
			       "Invalid or unknown file type: '%1'",
			       url.prettyUrl());
		break;
	    default:
		reason = "";
	}

	// show an error message box if the reason was known
	if (reason.length()) {
	    Kwave::MessageBox::error(this, reason);
	}

	// load failed
	closeFile();
    }
    m_context.application().addRecentFile(signalName());
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
    QString filter = CodecManager::decodingFilter();
    KwaveFileDialog dlg("kfiledialog:///kwave_open_dir", filter, this, true);
    dlg.setMode(static_cast<KFile::Modes>(KFile::File | KFile::ExistingOnly));
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Open"));
    if (dlg.exec() == QDialog::Accepted)
	return loadFile(dlg.selectedUrl());
    else
	return -1;
}

//***************************************************************************
int TopWidget::saveFile()
{
    int res = 0;
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return -EINVAL;
    if (!signal_manager) return -EINVAL;

    if (signalName() != NEW_FILENAME) {
	KUrl url;
	url = signalName();
	res = signal_manager->save(url, false);

	// if saving in current format is not possible (no encoder),
	// then try to "save/as" instead...
	if (res == -EINVAL) res = saveFileAs(false);
    } else res = saveFileAs(false);

    updateCaption();
    updateMenu();

    // enable "revert" after successful "save"
    if (!res) m_menu_manager->setItemEnabled("ID_FILE_REVERT", true);

    return res;
}

//***************************************************************************
int TopWidget::saveFileAs(bool selection)
{
    SignalManager *signal_manager = m_context.signalManager();
    int res = 0;
    Q_ASSERT(m_main_widget);
    Q_ASSERT(signal_manager);
    if (!m_main_widget || !signal_manager) return -EINVAL;

    KUrl current_url;
    current_url = signalName();
    KwaveFileDialog dlg("kfiledialog:///kwave_save_as", CodecManager::encodingFilter(),
        this, true, current_url.prettyUrl(), "*.wav");
    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setCaption(i18n("Save As"));
    if (dlg.exec() != QDialog::Accepted) return -1;

    KUrl url = dlg.selectedUrl();
    if (url.isEmpty()) return 0;

    QString name = url.path();
    QFileInfo path(name);

    // add the correct extension if necessary
    if (!path.suffix().length()) {
	QString ext = dlg.selectedExtension();
	QStringList extensions = ext.split(" ");
	ext = extensions.first();
	name += ext.mid(1);
	path = name;
	url.setPath(name);
    }

    // check if the file exists and ask before overwriting it
    // if it is not the old filename
    if ((name != signalName()) && (path.exists())) {
	if (Kwave::MessageBox::warningYesNo(this,
	    i18n("The file '%1' already exists. Do you really "\
	    "want to overwrite it?", name)) != KMessageBox::Yes)
	{
	    return -1;
	}
    }

    // maybe we now have a new mime type
    QString previous_mimetype_name =
	FileInfo(signal_manager->metaData()).get(INF_MIMETYPE).toString();

    QString new_mimetype_name;
    new_mimetype_name = CodecManager::whatContains(url);

    if (new_mimetype_name != previous_mimetype_name) {
	// saving to a different mime type
	// now we have to do as if the mime type and file name
	// has already been selected to satisfy the fileinfo
	// plugin
	qDebug("TopWidget::saveAs(%s) - [%s] (previous:'%s')",
	    url.prettyUrl().toLocal8Bit().data(),
	    new_mimetype_name.toLocal8Bit().data(),
	    previous_mimetype_name.toLocal8Bit().data() );

	// set the new mimetype
	FileInfo info(signal_manager->metaData());
	info.set(INF_MIMETYPE, new_mimetype_name);
	// save the old filename and set the new one
	QString old_filename = info.get(INF_FILENAME).toString();
	info.set(INF_FILENAME, url.prettyUrl());
	signal_manager->setFileInfo(info, false);

	// now call the fileinfo plugin with the new filename and
	// mimetype
	Q_ASSERT(m_context.pluginManager());
	res = (m_context.pluginManager()) ?
	    m_context.pluginManager()->setupPlugin("fileinfo") : -1;

	// restore the mime type and the filename
	info = FileInfo(signal_manager->metaData());
	info.set(INF_MIMETYPE, previous_mimetype_name);
	info.set(INF_FILENAME, url.prettyUrl());
	signal_manager->setFileInfo(info, false);
    }

    if (!res) res = signal_manager->save(url, selection);

    updateCaption();
    m_context.application().addRecentFile(signalName());
    updateMenu();

    if (!res && !selection) {
	// enable "revert" after successful "save as"
	// of the whole file (not only selection)
	m_menu_manager->setItemEnabled("ID_FILE_REVERT", true);
    }

    return res;
}

//***************************************************************************
int TopWidget::newSignal(sample_index_t samples, double rate,
                         unsigned int bits, unsigned int tracks)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return -1;

    // abort if the user pressed cancel
    if (!closeFile()) return -1;
    emit sigSignalNameChanged(signalName());

    signal_manager->newSignal(samples, rate, bits, tracks);

    updateCaption();
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
void TopWidget::selectZoom(int index)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    Q_ASSERT(m_main_widget);
    Q_ASSERT(m_zoomselect);
    if (!signal_manager) return;
    if (!m_main_widget) return;
    if (!m_zoomselect) return;
    if (index < 0) return;
    if (index >= m_zoomselect->count()) return;

    QVariant v = m_zoomselect->itemData(index, ZOOM_DATA_TIME);
    unsigned int ms = 1;
    bool ok = false;
    if (v.isValid()) ms = v.toUInt(&ok);
    if (!ok) ms = 1;

    const double rate = signal_manager->rate();
    unsigned int width = m_main_widget->displayWidth();
    Q_ASSERT(width > 1);
    if (width <= 1) width = 2;
    const double new_zoom = rint(((rate * ms) / 1.0E3) -1 ) /
	static_cast<double>(width - 1);
    m_main_widget->setZoom(new_zoom);

    // force the zoom factor to be set, maybe the current selection
    // has been changed/corrected to the previous value so that we
    // don't get a signal.
    setZoomInfo(m_main_widget->zoom());
}

//***************************************************************************
void TopWidget::setZoomInfo(double zoom)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(zoom >= 0);
    Q_ASSERT(m_zoomselect);

    if (zoom <= 0.0) return; // makes no sense or signal is empty
    if (!m_zoomselect) return;

    double rate = (signal_manager) ? signal_manager->rate() : 0.0;
    double ms   = (rate > 0) ?
	(((m_main_widget->displaySamples()) * 1E3) / rate) : 0;

    QString strZoom;
    if ((signal_manager) && (signal_manager->tracks())) {
	if (rate > 0) {
	    // time display mode
	    int s = static_cast<int>(ms) / 1000;
	    int m = s / 60;

	    if (ms >= 60*1000) {
		strZoom = strZoom.sprintf("%02d:%02d min", m, s % 60);
	    } else if (ms >= 1000) {
		strZoom = strZoom.sprintf("%d sec", s);
	    } else if (ms >= 1) {
		strZoom = strZoom.sprintf("%d ms",
		    static_cast<int>(round(ms)));
	    } else if (ms >= 0.01) {
		strZoom = strZoom.sprintf("%0.3g ms", ms);
	    }
	} else {
	    // percent mode
	    double percent = 100.0 / zoom;
	    strZoom = Kwave::zoom2string(percent);
	}
    }

    m_zoomselect->blockSignals(true);

    // if the text is equal to an entry in the current list -> keep it
    if (m_zoomselect->contains(strZoom)) {
	// select existing entry, string match
	m_zoomselect->setCurrentIndex(m_zoomselect->findText(strZoom));
    } else {
	// remove user defined entries and scan for more or less exact match
	int i = 0;
	int match = -1;
	while (i < m_zoomselect->count()) {
	    QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_PREDEFINED);
	    if (!v.isValid() || !v.toBool()) {
		m_zoomselect->removeItem(i);
	    } else {
		QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_TIME);
		bool ok = false;
		double t = v.toDouble(&ok);
		if (ok && (t > 0) && fabs(1 - (t / ms)) < (1.0 / 60.0)) {
		    match = i;
		}
		i++;
	    }
	}

	if (match >= 0) {
	    // use an exact match from the list
	    i = match;
	} else if (rate > 0) {
	    // time mode:
	    // find the best index where to insert the new user defined value
	    for (i = 0; i < m_zoomselect->count(); i++) {
		QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_TIME);
		bool ok = false;
		double t = v.toDouble(&ok);
		if (!ok) continue;
		if (t > ms) break;
	    }
	    m_zoomselect->insertItem(i, strZoom);
	    m_zoomselect->setItemData(i, QVariant(ms), ZOOM_DATA_TIME);
	} else {
	    // percent mode -> just insert at top
	    m_zoomselect->insertItem(-1, strZoom);
	    i = 0;
	}
	m_zoomselect->setCurrentIndex(i);
    }

    m_zoomselect->blockSignals(false);
}

//***************************************************************************
void TopWidget::metaDataChanged(Kwave::MetaDataList meta_data)
{
    Q_ASSERT(statusBar());
    Q_ASSERT(m_menu_manager);
    if (!statusBar() || !m_menu_manager) return;
    double ms;
    QString txt;

    const FileInfo info(meta_data);
    sample_index_t length = info.length();
    unsigned int tracks   = info.tracks();
    double rate           = info.rate();
    unsigned int bits     = info.bits();

    // length in milliseconds
    if (length) {
	ms = (rate) ? (static_cast<double>(length) /
	    static_cast<double>(rate) * 1E3) : 0;
	txt = " " + i18nc(
	    "Length, as in total duration of loaded song",
	    "Length: %1 (%2 samples)",
	    Kwave::ms2string(ms), Kwave::dottedNumber(length)
	) + " ";
    } else txt = "";
    m_lbl_status_size->setText(txt);

    // sample rate and resolution
    if (bits) {
	QString khz = "%0.3f";
	khz = khz.sprintf("%0.3f", static_cast<double>(rate) * 1E-3);
	txt = " " + i18n("Mode: %1 kHz @ %2 Bit", khz, bits) + " ";
    } else txt = "";
    m_lbl_status_mode->setText(txt);

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

    // revert is not possible if no signal at all is present
    if (!have_signal) {
	m_menu_manager->setItemEnabled("ID_FILE_REVERT", false);
    }

    // remove selection/position display on file close
    if (!have_signal) selectionChanged(0, 0);

    // update the menu
    updateMenu();

    // update the toolbar as well
    updateToolbar();
}

//***************************************************************************
void TopWidget::selectionChanged(sample_index_t offset, sample_index_t length)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    Q_ASSERT(statusBar());
    if (!statusBar()) return;

    const double rate = signal_manager->rate();

    if (length > 1) {
	// show offset and length
	// Selected: 2000...3000 (1000 samples)
	// Selected: 02:00...05:00 (3 min)
	bool sample_mode = false;

	sample_index_t last = offset + ((length) ? length-1 : 0);
	if (rate == 0) sample_mode = true; // force sample mode if rate==0
	QString txt = " ";
	if (sample_mode) {
	    txt += i18nc(
	        "%1=first sample, %2=last sample, %3=number of samples, "\
	        "example: 'Selected: 2000...3000 (1000 samples)'",
	        "Selected: %1...%2 (%3 samples)",
	        Kwave::dottedNumber(offset),
	        Kwave::dottedNumber(last),
	        Kwave::dottedNumber(length)
	    );
	} else {
	    double ms_first = static_cast<double>(offset)   * 1E3 / rate;
	    double ms_last  = static_cast<double>(last + 1) * 1E3 / rate;
	    double ms = (ms_last - ms_first);
	    txt += i18nc(
	        "%1=start time, %2=end time, %3=time span, "\
	        "example: 'Selected: 02:00...05:00 (3 min)'",
	        "Selected: %1...%2 (%3)",
	        Kwave::ms2string(ms_first),
	        Kwave::ms2string(ms_last),
	        Kwave::ms2string(ms)
	    );
	}

	m_lbl_status_cursor->setText("");
	statusBar()->showMessage(txt, 4000);
	m_menu_manager->setItemEnabled("@SELECTION", true);
    } else {
	// show cursor position
	// Position: 02:00
	bool sample_mode = false;

	if (rate == 0) sample_mode = true; // force sample mode if rate==0
	if (sample_mode || !signal_manager->tracks()) {
	    m_lbl_status_cursor->setText("");
	} else {
	    double ms_first = static_cast<double>(offset) * 1E3 / rate;
	    QString txt = i18n("Position: %1",
		Kwave::ms2string(ms_first));
	    m_lbl_status_cursor->setText(txt);
	}

	m_menu_manager->setItemEnabled("@SELECTION", false);
    }
}

//***************************************************************************
void TopWidget::updatePlaybackPos(sample_index_t offset)
{
    if (!m_context.pluginManager()) return;
    if (!m_main_widget) return;

    bool playing = m_context.signalManager()->playbackController().running();
    if (!playing) return;
    QString txt;
    double rate = m_context.pluginManager()->signalRate();
    if (rate > 0) {
	double ms = static_cast<double>(offset) * 1E3 / rate;
	txt = i18n("Playback: %1", Kwave::ms2string(ms));
    } else {
	txt = i18n("Playback: %1 samples",
	            Kwave::dottedNumber(offset));
    }
    statusBar()->showMessage(txt, 2000);

    // make sure that the current playback position is visible
    m_main_widget->scrollTo(offset);
}

//***************************************************************************
void TopWidget::setUndoRedoInfo(const QString &undo, const QString &redo)
{
    QString txt;
    bool undo_enabled = (undo.length() != 0);
    bool redo_enabled = (redo.length() != 0);

    // set the state and tooltip of the undo toolbar button
    if (m_action_undo) {
	txt = (undo_enabled) ?
	    i18nc("tooltip of the undo toolbar button if undo enabled",
		  "Undo (%1)", undo) :
	    i18nc("tooltip of the undo toolbar button if undo disabled",
		  "Undo");
	m_action_undo->setToolTip(txt);
	m_action_undo->setEnabled(undo_enabled);
    }

    // set the state and tooltip of the redo toolbar button
    if (m_action_redo) {
	txt = (redo_enabled) ?
	    i18nc("tooltip of the redo toolbar button, redo enabled",
		  "Redo (%1)", redo) :
	    i18nc("tooltip of the redo toolbar button, redo disabled",
		  "Redo");
	m_action_redo->setToolTip(txt);
	m_action_redo->setEnabled(redo_enabled);
    }

    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled("ID_EDIT_UNDO", undo_enabled);
    txt = (undo_enabled) ?
	i18nc("menu entry for undo if undo enabled",  "U&ndo (%1)", undo) :
	i18nc("menu entry for undo if undo disabled", "U&ndo");
    m_menu_manager->setItemText("ID_EDIT_UNDO", txt);

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled("ID_EDIT_REDO", redo_enabled);
    txt = (redo_enabled) ?
	i18nc("menu entry for redo if redo enabled",  "R&edo (%1)", redo) :
	i18nc("menu entry for redo if redo disabled", "R&edo");
    m_menu_manager->setItemText("ID_EDIT_REDO", txt);
}

//***************************************************************************
void TopWidget::mouseChanged(Kwave::MouseMark::Mode mode)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    switch (mode) {
	case (Kwave::MouseMark::MouseAtSelectionBorder) :
	case (Kwave::MouseMark::MouseInSelection) :
	{
	    sample_index_t offset = signal_manager->selection().offset();
	    sample_index_t length = signal_manager->selection().length();
	    selectionChanged(offset, length);
	    break;
	}
	default:
	    ;
    }
}

//***************************************************************************
void TopWidget::clipboardChanged(bool data_available)
{
    if (!m_menu_manager) return;
    m_menu_manager->setItemEnabled("@CLIPBOARD", data_available);
}

//***************************************************************************
void TopWidget::updateRecentFiles()
{
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    m_menu_manager->clearNumberedMenu("ID_FILE_OPEN_RECENT");

    QStringList recent_files = m_context.application().recentFiles();
    QStringList::Iterator it;
    for (it = recent_files.begin(); it != recent_files.end(); ++it) {
	m_menu_manager->addNumberedMenuEntry("ID_FILE_OPEN_RECENT", *it);
    }
}

//***************************************************************************
void TopWidget::updateMenu()
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // enable/disable all items that depend on having a file
    bool have_file = (signalName().length() != 0);
    m_menu_manager->setItemEnabled("@NOT_CLOSED", have_file);

    // enable/disable all items that depend on having a label
    LabelList labels(signal_manager->metaData());
    bool have_labels = (!labels.isEmpty());
    m_menu_manager->setItemEnabled("@LABELS", have_labels);

    // enable/disable all items that depend on having something in the
    // clipboard
    bool have_clipboard_data = !ClipBoard::instance().isEmpty();
    clipboardChanged(have_clipboard_data);
}

//***************************************************************************
void TopWidget::resetToolbarToDefaults()
{
    KToolBar *toolbar_file        = toolBar(TOOLBAR_FILE);
    KToolBar *toolbar_edit        = toolBar(TOOLBAR_EDIT);
    KToolBar *toolbar_record_play = toolBar(TOOLBAR_RECORD_PLAY);
    KToolBar *toolbar_zoom        = toolBar(TOOLBAR_ZOOM);

    int icon_size_def = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    int icon_size_big = style()->pixelMetric(QStyle::PM_LargeIconSize);

    // change style to "symbols only mode" and set standard size
    foreach(KToolBar *bar, toolBars()) {
	bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	bar->setIconSize( QSize(icon_size_def, icon_size_def) );
    }

    // re-order the tool bars:
    // ---------------------------
    // file  |  edit  |  zoom  |
    // ---------------------------
    // record/play  |
    // ---------------------------
    insertToolBar(toolbar_record_play, toolbar_zoom);
    insertToolBar(toolbar_zoom,        toolbar_edit);
    insertToolBar(toolbar_edit,        toolbar_file);

    // move record/playback into a seperate line, below file/edit
    insertToolBarBreak(toolbar_record_play);

    // let record/playback and zoom use bigger icons
    toolbar_record_play->setIconSize(QSize(icon_size_big, icon_size_big));
    toolbar_zoom->setIconSize(QSize(icon_size_big, icon_size_big));

    foreach(KToolBar *bar, toolBars()) {
	bar->update();
	bar->show();
    }
}

//***************************************************************************
void TopWidget::updateToolbar()
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    bool have_signal = signal_manager->tracks();

    if (m_action_zoomselection)
	m_action_zoomselection->setEnabled(have_signal);
    if (m_action_zoomin)
        m_action_zoomin->setEnabled(have_signal);
    if (m_action_zoomout)
        m_action_zoomout->setEnabled(have_signal);
    if (m_action_zoomnormal)
        m_action_zoomnormal->setEnabled(have_signal);
    if (m_action_zoomall)
        m_action_zoomall->setEnabled(have_signal);
    if (m_action_zoomselect)
        m_action_zoomselect->setEnabled(have_signal);
}

//***************************************************************************
void TopWidget::modifiedChanged(bool)
{
    updateCaption();
}

//***************************************************************************
void TopWidget::updateCaption()
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    bool modified = signal_manager->isModified();

    // shortcut if no file loaded
    if (signalName().length() == 0) {
	setCaption(QString());
	return;
    }

    if (modified)
	setCaption(i18nc(
	    "%1 = Path to modified file",
	    "* %1 (modified)",
	    signalName())
	);
    else
	setCaption(signalName());
}

//***************************************************************************
void TopWidget::closeEvent(QCloseEvent *e)
{
    (closeFile()) ? e->accept() : e->ignore();
}

//***************************************************************************
bool TopWidget::haveSignal()
{
    SignalManager *signal_manager = m_context.signalManager();
    return (signal_manager) ? (signal_manager->tracks()) : false;
}

//***************************************************************************
QString TopWidget::signalName() const
{
    if (!m_context.pluginManager()) return QString();
    return m_context.pluginManager()->signalManager().signalName();
}

//***************************************************************************
void TopWidget::showInSplashSreen(const QString &message)
{
    KwaveSplash::showMessage(message);
}

//***************************************************************************
#include "TopWidget.moc"
//***************************************************************************
//***************************************************************************
