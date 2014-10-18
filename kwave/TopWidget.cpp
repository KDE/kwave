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

#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopWidget>
#include <QtCore/QFile>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtCore/QMap>
#include <QtGui/QPixmap>
#include <QtGui/QSizePolicy>
#include <QtCore/QLatin1Char>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <ktoolbar.h>

#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/Encoder.h"
#include "libkwave/FileContext.h"
#include "libkwave/Plugin.h" // for some helper functions
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"
#include "libgui/MenuManager.h"

#include "App.h"
#include "Splash.h"
#include "MainWidget.h"
#include "PlayerToolBar.h"
#include "TopWidget.h"
#include "ZoomToolBar.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == _(x)) {

/** toolbar name: file operations */
#define TOOLBAR_FILE        _("MainWidget File")

/** toolbar name: edit operations */
#define TOOLBAR_EDIT        _("MainWidget Edit")

/** toolbar name: record and playback */
#define TOOLBAR_RECORD_PLAY _("MainWidget Record/Playback")

/** toolbar name: zoom controls */
#define TOOLBAR_ZOOM        _("MainWidget Zoom")

//***************************************************************************

/**
 * struct for info about a label within a Kwave script
 * @internal
 */
namespace Kwave {
    typedef struct {
	qint64       pos;  /**< position within the stream      */
	unsigned int hits; /**< number of "goto"s to this label */
    } label_t;
}

//***************************************************************************
//***************************************************************************
Kwave::TopWidget::TopWidget(Kwave::App &app)
    :KMainWindow(), m_application(app), m_current_context(0),
     m_main_widget(0), m_toolbar_record_playback(0), m_toolbar_zoom(0),
     m_menu_manager(0), m_action_save(0), m_action_save_as(0),
     m_action_close(0), m_action_undo(0), m_action_redo(0),
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

    // start up iconified if requested
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool iconic = (args && args->isSet("iconic"));
    if (iconic) {
	showMinimized();
    }

    // direct all kind of focus to this window per default
    setFocusPolicy(Qt::WheelFocus);
}

//***************************************************************************
bool Kwave::TopWidget::newFileContext()
{
    Q_ASSERT(m_toolbar_zoom);
    if (!m_toolbar_zoom) return false;

    Kwave::FileContext *context = new Kwave::FileContext(m_application);
    if (!context)
	return false;
    if (!context->init(this)) {
	delete context;
	return false;
    }
    Q_ASSERT(context);

    // connect the zoom toolbar
    connect(context,   SIGNAL(sigZoomChanged(Kwave::FileContext *, double)),
            m_toolbar_zoom, SLOT(setZoomInfo(Kwave::FileContext *, double)));
    connect(this,  SIGNAL(sigFileContextSwitched(Kwave::FileContext *)),
            m_toolbar_zoom, SLOT(contextSwitched(Kwave::FileContext *)));
    connect(context,             SIGNAL(destroyed(Kwave::FileContext *)),
            m_toolbar_zoom, SLOT(contextDestroyed(Kwave::FileContext *)));

    // connect the status bar
    connect(context, SIGNAL(sigStatusBarMessage(const QString &, unsigned int)),
            this,    SLOT(showStatusBarMessage(const QString &, unsigned int)));

    // if we reach this point everything was ok, now we can safely switch
    // to the new context
    m_current_context = context;
    emit sigFileContextSwitched(m_current_context);

    return true;
}

//***************************************************************************
bool Kwave::TopWidget::init()
{
    KIconLoader icon_loader;

    // --- zoom controls ---

    m_toolbar_zoom = new Kwave::ZoomToolBar(this, TOOLBAR_ZOOM);
    Q_ASSERT(m_toolbar_zoom);
    if (!m_toolbar_zoom) return false;

    // -- create a new file context ---
    bool ok = newFileContext();
    Q_ASSERT(ok);
    if (!ok)
	return false;

    showInSplashSreen(i18n("Loading main menu..."));
    KMenuBar *menubar = menuBar();
    Q_ASSERT(menubar);
    if (!menubar) return false;
    m_menu_manager = new Kwave::MenuManager(this, *menubar);
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return false;

    // connect clicked menu entries with main communication channel of kwave
    connect(m_menu_manager, SIGNAL(sigMenuCommand(const QString &)),
	    this, SLOT(forwardCommand(const QString &)));
    connect(&Kwave::ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
	    this, SLOT(clipboardChanged(bool)));

    // load the menu from file
    QFile menufile(KStandardDirs::locate("data", _("kwave/menus.config")));
    menufile.open(QIODevice::ReadOnly);
    QTextStream stream(&menufile);
    Q_ASSERT(!stream.atEnd());
    if (!stream.atEnd()) parseCommands(stream);
    menufile.close();

    // ### GUI_MDI ###
    m_main_widget = m_current_context->mainWidget();

    // --- set up the toolbar ---

    showInSplashSreen(i18n("Initializing toolbar..."));
    KToolBar *toolbar_file = toolBar(TOOLBAR_FILE);
    Q_ASSERT(toolbar_file);
    if (!toolbar_file) return false;

    // --- file open and save ---

    toolbar_file->addAction(
	icon_loader.loadIcon(_("document-new"), KIconLoader::Toolbar),
	i18n("Create a new empty file"),
	this, SLOT(toolbarFileNew()));

    toolbar_file->addAction(
	icon_loader.loadIcon(_("document-open"), KIconLoader::Toolbar),
	i18n("Open an existing file"),
	this, SLOT(toolbarFileOpen()));

    m_action_save = toolbar_file->addAction(
	icon_loader.loadIcon(_("document-save"), KIconLoader::Toolbar),
	i18n("Save the current file"),
	this, SLOT(toolbarFileSave()));

    m_action_save_as = toolbar_file->addAction(
	icon_loader.loadIcon(_("document-save-as"), KIconLoader::Toolbar),
	i18n("Save the current file under a different name or file format..."),
	this, SLOT(toolbarFileSaveAs()));

    m_action_close = toolbar_file->addAction(
	icon_loader.loadIcon(_("document-close"), KIconLoader::Toolbar),
	i18n("Close the current file"),
	this, SLOT(toolbarFileClose()));

    // --- edit, cut&paste ---

    KToolBar *toolbar_edit = toolBar(TOOLBAR_EDIT);
    Q_ASSERT(toolbar_edit);
    if (!toolbar_edit) return false;

    m_action_undo = toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-undo"), KIconLoader::Toolbar),
	i18n("Undo"),
	this, SLOT(toolbarEditUndo()));

    m_action_redo = toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-redo"), KIconLoader::Toolbar),
	i18n("Redo"),
	this, SLOT(toolbarEditRedo()));

    toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-cut"), KIconLoader::Toolbar),
	i18n("Cut"),
	this, SLOT(toolbarEditCut()));

    toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-copy"), KIconLoader::Toolbar),
	i18n("Copy"),
	this, SLOT(toolbarEditCopy()));

    QAction *btPaste = toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-paste"), KIconLoader::Toolbar),
	i18n("Insert"),
	this, SLOT(toolbarEditPaste()));
    btPaste->setEnabled(!Kwave::ClipBoard::instance().isEmpty());
    connect(&Kwave::ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
            btPaste, SLOT(setEnabled(bool)));

    toolbar_edit->addAction(
	icon_loader.loadIcon(_("draw-eraser"), KIconLoader::Toolbar),
	i18n("Mute selection"),
	this, SLOT(toolbarEditErase()));

    toolbar_edit->addAction(
	icon_loader.loadIcon(_("edit-delete"), KIconLoader::Toolbar),
	i18n("Delete selection"),
	this, SLOT(toolbarEditDelete()));

    // --- record/playback controls ---

    // ### GUI_MDI ###
    m_toolbar_record_playback = new Kwave::PlayerToolBar(
	this, TOOLBAR_RECORD_PLAY,
	m_current_context->signalManager()->playbackController(),
	*m_menu_manager);
    Q_ASSERT(m_toolbar_record_playback);
    if (!m_toolbar_record_playback) return false;

    connect(m_toolbar_record_playback, SIGNAL(sigCommand(const QString &)),
            this,                    SLOT(forwardCommand(const QString &)));
    // ### GUI_MDI ###
    connect(m_main_widget, SIGNAL(sigVisibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)),
	    m_toolbar_record_playback, SLOT(visibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)) );

    // connect the signal manager
    // ### GUI_MDI ###
    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    connect(&(signal_manager->selection()),
            SIGNAL(changed(sample_index_t, sample_index_t)),
            this,
            SLOT(selectionChanged(sample_index_t, sample_index_t)));
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

    // set the MainWidget as the main view
    setCentralWidget(m_main_widget); // ### GUI_MDI ###

    // set a nice initial size
    int w = m_main_widget->minimumSize().width();
    w = qMax(w, m_main_widget->sizeHint().width());
    w = qMax(w, width());
    int h = qMax(m_main_widget->sizeHint().height(), (w * 6) / 10);
    h = qMax(h, height());
    resize(w, h);

    metaDataChanged(Kwave::MetaData());
    setUndoRedoInfo(QString(), QString());
    selectionChanged(0, 0);
    updateMenu();
    updateToolbar();
    updateRecentFiles();

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
    QString magic = _("3");
    if (cfg.readEntry("toolbars") != magic) {
	qDebug("toolbar layout changed => resetting toolbars to defaults");

	// unlock all toolbars
	KToolBar::setToolBarsLocked(false);

	// reset all toolbars to default layout
	resetToolbarToDefaults();

	cfg.writeEntry("toolbars", magic);
    }

    // make sure we have the focus, not the zoom combo box
    setFocus();

    // special handling: a null string tells the splash screen to hide
    showInSplashSreen(QString());
    return true;
}

//***************************************************************************
Kwave::TopWidget::~TopWidget()
{
    // close the current file (no matter what the user wants)
    closeFile();

    delete m_toolbar_zoom;
    m_toolbar_zoom = 0;

    delete m_toolbar_record_playback;
    m_toolbar_record_playback = 0;

    delete m_menu_manager;
    m_menu_manager = 0;

    m_main_widget = 0;

    m_current_context->close();
    delete m_current_context;
    m_current_context = 0;
}

//***************************************************************************
int Kwave::TopWidget::executeCommand(const QString &line)
{
    int result = 0;
    QString command = line;

//     qDebug("TopWidget::executeCommand(%s)", DBG(command));
    if (!command.length()) return 0; // empty line -> nothing to do

    // parse one single command
    Kwave::Parser parser(command);

    // playback commands are always possible
    if ( (parser.command() == _("playback")) && (m_toolbar_record_playback) )
	return m_toolbar_record_playback->executeCommand(parser.firstParam());

    if ((result = m_application.executeCommand(command)) != ENOSYS) {
	return result;
    CASE_COMMAND("about_kde")
	// Help / About KDE
	KHelpMenu *dlg = new KHelpMenu(this, _("Kwave"));
	if (dlg) dlg->aboutKDE();
    CASE_COMMAND("menu")
	Q_ASSERT(m_menu_manager);
	if (m_menu_manager) result = m_menu_manager->executeCommand(command);
    CASE_COMMAND("newsignal")
	sample_index_t samples = parser.toSampleIndex();
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
	result = closeFile() ? 0 : 1;
    CASE_COMMAND("revert")
	result = revert();
    CASE_COMMAND("saveas")
	result = saveFileAs(parser.nextParam(), false);
    CASE_COMMAND("loadbatch")
	result = loadBatch(parser.nextParam());
    CASE_COMMAND("saveselect")
	result = saveFileAs(QString(), true);
    CASE_COMMAND("quit")
	result = (close()) ? 0 : -1;
    CASE_COMMAND("reset_toolbars")
	if ((result = (Kwave::MessageBox::questionYesNo(this,
	    i18n("Reset the toolbar to default settings?"))
	    == KMessageBox::Yes) ? 1 : 0))
	{
	    resetToolbarToDefaults();
	}
    CASE_COMMAND("reenable_dna")
	if ((result = (Kwave::MessageBox::questionYesNo(this,
	    i18n("Re-enable all disabled notifications?\n"
		 "All messages that you previously turned off by activating "
		 "the \"Do not ask again\" checkbox will then be enabled again."
	    ))
	    == KMessageBox::Yes) ? 1 : 0))
	{
	    KMessageBox::enableAllMessages();
	}
    } else {
	return ENOSYS; // command not implemented (here)
    }

    return result;
}

//***************************************************************************
void Kwave::TopWidget::forwardCommand(const QString &command)
{
    if (m_current_context)
        m_current_context->executeCommand(command);
}

//***************************************************************************
int Kwave::TopWidget::loadBatch(const KUrl &url)
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

    return result;
}

//***************************************************************************
int Kwave::TopWidget::parseCommands(QTextStream &stream)
{
    int result = 0;
    QMap<QString, label_t> labels;

    // hold an own context pointer, just in case the context changes
    // during processing
    QPointer<Kwave::FileContext> context(m_current_context);
    if (!context) return ENOSYS;

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    while (!stream.atEnd() && !result) {
	QString line = stream.readLine().simplified();
	if (line.startsWith(_("#"))) continue; // skip comments
	if (!line.length()) continue;       // skip empty lines

	// remove stuff after the "#'" (comments)
	if (line.contains(QLatin1Char('#'))) {
	}

	if (line.endsWith(QLatin1Char(':'))) {
	    // this line seems to be a "label"
	    line = line.left(line.length() - 1).simplified();
	    if (!labels.contains(line)) {
		qDebug("new label '%s' at %llu", DBG(line), stream.pos());
		label_t label;
		label.pos  = stream.pos();
		label.hits = 0;
		labels[line] = label;
	    }
	    continue;
	}

	Kwave::Parser parser(line);

	// the "goto" command
	if (line.split(QLatin1Char(' ')).at(0) == _("goto")) {
	    qDebug(">>> detected 'goto'");
	    QString label = line.split(QLatin1Char(' ')).at(1).simplified();
	    if (labels.contains(label)) {
		labels[label].hits++;
		qDebug(">>> goto '%s' @ offset %llu (pass #%d)", DBG(label),
		       labels[label].pos,
		       labels[label].hits
  		    );
		stream.seek(labels[label].pos);
	    } else {
		qWarning("label '%s' not found", DBG(label));
		break;
	    }
	    continue;
	}

	// synchronize before the command
	if (context && context->pluginManager())
	    context->pluginManager()->sync();

	// the "msgbox" command (useful for debugging)
	if (parser.command() == _("msgbox")) {
	    QApplication::restoreOverrideCursor();
	    result = (Kwave::MessageBox::questionYesNo(this,
		parser.firstParam()) == KMessageBox::Yes) ? 0 : 1;
	    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	    continue;
	}

	// prevent this command from being re-added to the macro recorder
	if (!line.startsWith(_("nomacro:"), Qt::CaseInsensitive))
	    line.prepend(_("nomacro:"));

	// process the command in the current context
	// NOTE: this could theoretically also be a command that modifies
	//       or even deletes the current context!
	result = (context) ? context->executeCommand(line) : -EINTR;
	if (result)
	    qDebug(">>> '%s' - result=%d", DBG(line), result);

	// synchronize after the command
	if (context && context->pluginManager())
	    context->pluginManager()->sync();

	// special handling of the "quit" command
	if (parser.command() == _("quit")) {
	    break;
	}
    }

    // remove hourglass
    QApplication::restoreOverrideCursor();

    return result;
}

//***************************************************************************
int Kwave::TopWidget::revert()
{
    KUrl url(signalName());
    if (!url.isValid()) return -EINVAL;

    return loadFile(url);
}

//***************************************************************************
bool Kwave::TopWidget::closeFile()
{
    Q_ASSERT(m_current_context);
    if (!m_current_context) return true;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Kwave::PluginManager *plugin_manager = m_current_context->pluginManager();

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

    if (signal_manager) signal_manager->close();

    updateCaption(signalName(), false);
    emit sigSignalNameChanged(signalName());

    updateMenu();
    return true;
}

//***************************************************************************
int Kwave::TopWidget::loadFile(const KUrl &url)
{
    // ### TODO ### GUI_MDI ###
    if (!m_current_context) return -1;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Q_ASSERT(signal_manager);

    // abort if new file not valid and local
    if (!url.isLocalFile()) return -1;

    // detect whether it is a macro (batch) file
    QFileInfo file(url.fileName());
    QString suffix = file.suffix();
    if (suffix == _("kwave")) {
	return loadBatch(url);
    }

    // try to close the previous file
    if (!closeFile()) return -1;

    emit sigSignalNameChanged(url.path());

    int res = -ENOMEM;
    if (signal_manager && !(res = signal_manager->loadFile(url))) {
	// succeeded
	updateCaption(signalName(), false);

	// enable revert after successful load
	m_menu_manager->setItemEnabled(_("ID_FILE_REVERT"), true);
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
		reason = _("");
	}

	// show an error message box if the reason was known
	if (reason.length()) {
	    Kwave::MessageBox::error(this, reason);
	}

	// load failed
	closeFile();
    }

    m_application.addRecentFile(signalName());
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
int Kwave::TopWidget::openRecent(const QString &str)
{
    Kwave::Parser parser(str);
    return loadFile(parser.firstParam());
}

//***************************************************************************
int Kwave::TopWidget::openFile()
{
    QString filter = Kwave::CodecManager::decodingFilter();
    Kwave::FileDialog dlg(_("kfiledialog:///kwave_open_dir"),
                          filter, this, true);
    dlg.setMode(static_cast<KFile::Modes>(KFile::File | KFile::ExistingOnly));
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Open"));
    if (dlg.exec() == QDialog::Accepted)
	return loadFile(dlg.selectedUrl());
    else
	return -1;
}

//***************************************************************************
int Kwave::TopWidget::saveFile()
{
    int res = 0;

    Q_ASSERT(m_current_context);
    if (!m_current_context) return -EINVAL;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    if (!signal_manager) return -EINVAL;

    if (signalName() != NEW_FILENAME) {
	KUrl url;
	url = signalName();
	res = signal_manager->save(url, false);

	// if saving in current format is not possible (no encoder),
	// then try to "save/as" instead...
	if (res == -EINVAL) res = saveFileAs(QString(), false);
    } else res = saveFileAs(QString(), false);

    updateCaption(signalName(), signal_manager->isModified());
    updateMenu();

    // enable "revert" after successful "save"
    if (!res)
	m_menu_manager->setItemEnabled(_("ID_FILE_REVERT"), true);

    return res;
}

//***************************************************************************
int Kwave::TopWidget::saveFileAs(const QString &filename, bool selection)
{
    Q_ASSERT(m_current_context);
    if (!m_current_context) return false;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    int res = 0;
    Q_ASSERT(signal_manager);
    if (!signal_manager) return -EINVAL;

    QString name = filename;
    KUrl url;

    if (name.length()) {
	/* name given -> take it */
	url = KUrl(name);
    } else {
	/*
	 * no name given -> show the File/SaveAs dialog...
	 */
	KUrl current_url;
	current_url = signalName();

	QString what  = Kwave::CodecManager::whatContains(current_url);
	Kwave::Encoder *encoder = Kwave::CodecManager::encoder(what);
	QString extension; // = "*.wav";
	if (!encoder) {
	    // no extension selected yet, use mime type from file info
	    QString mime_type = Kwave::FileInfo(
		signal_manager->metaData()).get(Kwave::INF_MIMETYPE).toString();
	    encoder = Kwave::CodecManager::encoder(mime_type);
	    if (encoder) {
		QStringList extensions = encoder->extensions(mime_type);
		if (!extensions.isEmpty()) {
		    QString ext = extensions.first().split(_(" ")).first();
		    if (ext.length()) {
			extension = ext;
			QString new_filename = current_url.fileName();
			new_filename += extension.mid(1); // remove the "*"
			current_url.setFileName(new_filename);
		    }
		}
	    }
	}

	QString filter = Kwave::CodecManager::encodingFilter();
	Kwave::FileDialog dlg(_("kfiledialog:///kwave_save_as"),
	    filter, this, true, current_url.prettyUrl(), extension);
	dlg.setOperationMode(KFileDialog::Saving);
	dlg.setCaption(i18n("Save As"));
	if (dlg.exec() != QDialog::Accepted) return -1;

	url = dlg.selectedUrl();
	if (url.isEmpty()) return 0;

	QString new_name = url.path();
	QFileInfo path(new_name);

	// add the correct extension if necessary
	if (!path.suffix().length()) {
	    QString ext = dlg.selectedExtension();
	    QStringList extensions = ext.split(_(" "));
	    ext = extensions.first();
	    new_name += ext.mid(1);
	    path = new_name;
	    url.setPath(new_name);
	}
    }

    // check if the file exists and ask before overwriting it
    // if it is not the old filename
    name = url.path();
    if ((url.prettyUrl() != KUrl(signalName()).prettyUrl()) &&
	(QFileInfo(name).exists()))
    {
	if (Kwave::MessageBox::warningYesNo(this,
	    i18n("The file '%1' already exists.\n"
	         "Do you really want to overwrite it?", name)) !=
	         KMessageBox::Yes)
	{
	    return -1;
	}
    }

    // maybe we now have a new mime type
    QString previous_mimetype_name =
	Kwave::FileInfo(signal_manager->metaData()).get(
	    Kwave::INF_MIMETYPE).toString();

    QString new_mimetype_name = Kwave::CodecManager::whatContains(url);

    if (new_mimetype_name != previous_mimetype_name) {
	// saving to a different mime type
	// now we have to do as if the mime type and file name
	// has already been selected to satisfy the fileinfo
	// plugin
	qDebug("TopWidget::saveAs(%s) - [%s] (previous:'%s')",
	    DBG(url.prettyUrl()), DBG(new_mimetype_name),
	    DBG(previous_mimetype_name) );

	// set the new mimetype
	Kwave::FileInfo info(signal_manager->metaData());
	info.set(Kwave::INF_MIMETYPE, new_mimetype_name);

	// set the new filename
	info.set(Kwave::INF_FILENAME, url.prettyUrl());
	signal_manager->setFileInfo(info, false);

	// now call the fileinfo plugin with the new filename and
	// mimetype
	Q_ASSERT(m_context.pluginManager());
	res = (m_current_context->pluginManager()) ?
	       m_current_context->pluginManager()->setupPlugin(_("fileinfo"))
	       : -1;

	// restore the mime type and the filename
	info = Kwave::FileInfo(signal_manager->metaData());
	info.set(Kwave::INF_MIMETYPE, previous_mimetype_name);
	info.set(Kwave::INF_FILENAME, url.prettyUrl());
	signal_manager->setFileInfo(info, false);
    }

    if (!res) res = signal_manager->save(url, selection);

    updateCaption(signalName(), signal_manager->isModified());
    m_application.addRecentFile(signalName());
    updateMenu();

    if (!res && !selection) {
	// enable "revert" after successful "save as"
	// of the whole file (not only selection)
	m_menu_manager->setItemEnabled(_("ID_FILE_REVERT"), true);
    }

    return res;
}

//***************************************************************************
int Kwave::TopWidget::newSignal(sample_index_t samples, double rate,
                                unsigned int bits, unsigned int tracks)
{
    // ### TODO ### GUI_MDI ###
    Q_ASSERT(m_current_context);
    if (!m_current_context) return false;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return -1;

    // abort if the user pressed cancel
    if (!closeFile()) return -1;
    emit sigSignalNameChanged(signalName());

    signal_manager->newSignal(samples, rate, bits, tracks);

    updateCaption(signalName(), true);
    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
void Kwave::TopWidget::metaDataChanged(Kwave::MetaDataList meta_data)
{
    Q_ASSERT(statusBar());
    if (!statusBar() || !m_menu_manager) return;
    double ms;
    QString txt;

    const Kwave::FileInfo info(meta_data);
    sample_index_t length = info.length();
    unsigned int tracks   = info.tracks();
    double rate           = info.rate();
    unsigned int bits     = info.bits();

    // length in milliseconds
    if (length) {
	ms = (rate > 0) ? (static_cast<double>(length) /
	    static_cast<double>(rate) * 1E3) : 0;
	txt = _(" ") + i18nc(
	    "Length, as in total duration of loaded song",
	    "Length: %1 (%2 samples)",
	    Kwave::ms2string(ms),
	    KGlobal::locale()->formatLong(static_cast<long int>(length))
	) + _(" ");
    } else txt = _("");
    m_lbl_status_size->setText(txt);

    // sample rate and resolution
    if (bits) {
	QString khz = _("%0.3f");
	khz = khz.sprintf("%0.3f", static_cast<double>(rate) * 1E-3);
	txt = _(" ") + i18n("Mode: %1 kHz @ %2 Bit", khz, bits) +  _(" ");
    } else txt = _("");
    m_lbl_status_mode->setText(txt);

    // update the list of deletable tracks
    m_menu_manager->clearNumberedMenu(_("ID_EDIT_TRACK_DELETE"));
    QString buf;
    for (unsigned int i = 0; i < tracks; i++) {
	m_menu_manager->addNumberedMenuEntry(
	    _("ID_EDIT_TRACK_DELETE"), buf.setNum(i));
    }

    // enable/disable all items that depend on having a signal
    bool have_signal = (tracks != 0);
    m_menu_manager->setItemEnabled(_("@SIGNAL"), have_signal);

    // revert is not possible if no signal at all is present
    if (!have_signal) {
	m_menu_manager->setItemEnabled(_("ID_FILE_REVERT"), false);
    }

    // remove selection/position display on file close
    if (!have_signal) selectionChanged(0, 0);

    // update the menu
    updateMenu();

    // update the toolbar as well
    updateToolbar();
}

//***************************************************************************
void Kwave::TopWidget::selectionChanged(sample_index_t offset,
                                        sample_index_t length)
{
    Q_ASSERT(m_current_context);
    if (!m_current_context) return;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    Q_ASSERT(statusBar());
    if (!statusBar()) return;

    // force sample mode if rate==0
    const double rate        = signal_manager->rate();
    const bool   sample_mode = (qFuzzyIsNull(rate));

    if (length > 1) {
	// show offset and length
	// Selected: 2000...3000 (1000 samples)
	// Selected: 02:00...05:00 (3 min)
	sample_index_t last = offset + ((length) ? length-1 : 0);

	QString txt = _(" ");
	if (sample_mode) {
	    txt += i18nc(
	        "%1=first sample, %2=last sample, %3=number of samples, "\
	        "example: 'Selected: 2000...3000 (1000 samples)'",
	        "Selected: %1...%2 (%3 samples)",
	        KGlobal::locale()->formatLong(static_cast<long int>(offset)),
	        KGlobal::locale()->formatLong(static_cast<long int>(last)),
	        KGlobal::locale()->formatLong(static_cast<long int>(length))
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

	m_lbl_status_cursor->setText(_(""));
	statusBar()->showMessage(txt, 4000);
	m_menu_manager->setItemEnabled(_("@SELECTION"), true);
    } else {
	// show cursor position
	// Position: 02:00

	if (sample_mode || !signal_manager->tracks()) {
	    m_lbl_status_cursor->setText(_(""));
	} else {
	    double ms_first = static_cast<double>(offset) * 1E3 / rate;
	    QString txt = i18n("Position: %1",
		Kwave::ms2string(ms_first));
	    m_lbl_status_cursor->setText(txt);
	}

	m_menu_manager->setItemEnabled(_("@SELECTION"), false);
    }

    // update the zoom toolbar on selection change, maybe the
    // button for "zoom selection" has to be enabled/disabled
    if (m_toolbar_zoom)
	m_toolbar_zoom->updateToolbar();
}

//***************************************************************************
void Kwave::TopWidget::setUndoRedoInfo(const QString &undo,
                                       const QString &redo)
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

    if (!m_menu_manager) return;

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled(_("ID_EDIT_UNDO"), undo_enabled);
    txt = (undo_enabled) ?
	i18nc("menu entry for undo if undo enabled",  "Undo (%1)", undo) :
	i18nc("menu entry for undo if undo disabled", "Undo");
    m_menu_manager->setItemText(_("ID_EDIT_UNDO"), txt);

    // set new enable and text of the undo menu entry
    m_menu_manager->setItemEnabled(_("ID_EDIT_REDO"), redo_enabled);
    txt = (redo_enabled) ?
	i18nc("menu entry for redo if redo enabled",  "Redo (%1)", redo) :
	i18nc("menu entry for redo if redo disabled", "Redo");
    m_menu_manager->setItemText(_("ID_EDIT_REDO"), txt);
}

//***************************************************************************
void Kwave::TopWidget::mouseChanged(Kwave::MouseMark::Mode mode,
                                    sample_index_t offset,
                                    sample_index_t length)
{
    switch (mode) {
	case (Kwave::MouseMark::MouseAtSelectionBorder) :
	case (Kwave::MouseMark::MouseInSelection) :
	    selectionChanged(offset, length);
	    break;
	default:
	    ;
    }
}

//***************************************************************************
void Kwave::TopWidget::clipboardChanged(bool data_available)
{
    if (!m_menu_manager) return;
    m_menu_manager->setItemEnabled(_("@CLIPBOARD"), data_available);
}

//***************************************************************************
void Kwave::TopWidget::updateRecentFiles()
{
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    m_menu_manager->clearNumberedMenu(_("ID_FILE_OPEN_RECENT"));

    QStringList recent_files = m_application.recentFiles();
    QStringList::Iterator it;
    for (it = recent_files.begin(); it != recent_files.end(); ++it) {
	m_menu_manager->addNumberedMenuEntry(
	    _("ID_FILE_OPEN_RECENT"), *it);
    }
}

//***************************************************************************
void Kwave::TopWidget::updateMenu()
{
    if (!m_current_context) return;
    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    // enable/disable all items that depend on having a file
    bool have_file = (signalName().length() != 0);
    m_menu_manager->setItemEnabled(_("@NOT_CLOSED"), have_file);

    // enable/disable all items that depend on having a label
    Kwave::LabelList labels(signal_manager->metaData());
    bool have_labels = (!labels.isEmpty());
    m_menu_manager->setItemEnabled(_("@LABELS"), have_labels);

    // enable/disable all items that depend on having something in the
    // clipboard
    bool have_clipboard_data = !Kwave::ClipBoard::instance().isEmpty();
    clipboardChanged(have_clipboard_data);
}

//***************************************************************************
void Kwave::TopWidget::resetToolbarToDefaults()
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
    // -----------------------
    // file  |  edit  |
    // -----------------------
    // record/play  | zoom
    // -----------------------
    insertToolBar(toolbar_zoom,        toolbar_record_play);
    insertToolBar(toolbar_record_play, toolbar_edit);
    insertToolBar(toolbar_edit,        toolbar_file);

    // move record/playback into a separate line, below file/edit
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
void Kwave::TopWidget::updateToolbar()
{
    // ### TODO ### GUI_MDI
    if (!m_current_context) return;

    Kwave::SignalManager *signal_manager = m_current_context->signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    bool have_signal = signal_manager->tracks();

    if (m_action_save)
	m_action_save->setEnabled(have_signal);
    if (m_action_save_as)
	m_action_save_as->setEnabled(have_signal);
    if (m_action_close)
	m_action_close->setEnabled(have_signal);

    // update the zoom toolbar
    if (m_toolbar_zoom)
	m_toolbar_zoom->updateToolbar();
}

//***************************************************************************
void Kwave::TopWidget::modifiedChanged(bool modified)
{
    updateCaption(signalName(), modified);
}

//***************************************************************************
void Kwave::TopWidget::updateCaption(const QString &name, bool is_modified)
{
    // shortcut if no file loaded
    if (!name.length()) {
	setCaption(QString());
	return;
    }

    if (is_modified)
	setCaption(i18nc(
	    "%1 = Path to modified file",
	    "* %1 (modified)",
	    name)
	);
    else
	setCaption(name);
}

//***************************************************************************
void Kwave::TopWidget::closeEvent(QCloseEvent *e)
{
    (closeFile()) ? e->accept() : e->ignore();
}

//***************************************************************************
QString Kwave::TopWidget::signalName() const
{
    if (!m_current_context) return QString();
    if (!m_current_context->pluginManager()) return QString();
    return m_current_context->pluginManager()->signalManager().signalName();
}

//***************************************************************************
void Kwave::TopWidget::showInSplashSreen(const QString &message)
{
    Kwave::Splash::showMessage(message);
}

//***************************************************************************
void Kwave::TopWidget::showStatusBarMessage(const QString &msg,
                                            unsigned int ms)
{
    KStatusBar *status_bar = statusBar();
    Q_ASSERT(status_bar);
    if (status_bar) status_bar->showMessage(msg, ms);
}

//***************************************************************************
#include "TopWidget.moc"
//***************************************************************************
//***************************************************************************
