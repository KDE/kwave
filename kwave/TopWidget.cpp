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

#include <new>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QLatin1Char>
#include <QMap>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMutableMapIterator>
#include <QPixmap>
#include <QSizePolicy>
#include <QStatusBar>
#include <QStringList>
#include <QtGlobal>

#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KToolBar>

#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/FileDrag.h"
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Parser.h"
#include "libkwave/Plugin.h" // for some helper functions
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"
#include "libgui/MenuManager.h"

#include "App.h"
#include "FileContext.h"
#include "MainWidget.h"
#include "PlayerToolBar.h"
#include "Splash.h"
#include "TopWidget.h"
#include "ZoomToolBar.h"

using namespace Qt::StringLiterals;

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
//***************************************************************************
Kwave::TopWidget::TopWidget(Kwave::App &app)
    :KMainWindow(),
     m_application(app),
     m_context_map()
{
    // status bar items
    QStatusBar *status_bar = statusBar();
    Q_ASSERT(status_bar);
    if (!status_bar) return;

    QLabel *spacer = new(std::nothrow) QLabel(this);
    const int frame_style = static_cast<int>(QFrame::StyledPanel) |
                            static_cast<int>(QFrame::Sunken);
    status_bar->addWidget(spacer);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setFrameStyle(frame_style);
    QSizePolicy policy = spacer->sizePolicy();
    policy.setHorizontalStretch(100);
    spacer->setSizePolicy(policy);
    policy.setHorizontalStretch(0);

    m_lbl_status_cursor = new(std::nothrow) QLabel(this);
    status_bar->addWidget(m_lbl_status_cursor);
    m_lbl_status_cursor->setSizePolicy(policy);
    m_lbl_status_cursor->setFrameStyle(frame_style);

    m_lbl_status_mode = new(std::nothrow) QLabel(this);
    status_bar->addWidget(m_lbl_status_mode);
    m_lbl_status_mode->setSizePolicy(policy);
    m_lbl_status_mode->setFrameStyle(frame_style);

    m_lbl_status_size = new(std::nothrow) QLabel(this);
    status_bar->addWidget(m_lbl_status_size);
    m_lbl_status_size->setSizePolicy(policy);
    m_lbl_status_size->setFrameStyle(frame_style);

    // start up iconified if requested
    const QCommandLineParser *args = m_application.cmdline();
    bool iconic = (args && args->isSet(_("iconic")));
    if (iconic) {
        showMinimized();
    }

    setAcceptDrops(true); // enable drag&drop

    // direct all kind of focus to this window per default
    setFocusPolicy(Qt::WheelFocus);
}

//***************************************************************************
void Kwave::TopWidget::connectContext(Kwave::FileContext *context)
{
    // connect the context to the top widget
    connect(context, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            this,         SLOT(metaDataChanged(Kwave::MetaDataList)));
    connect(context,
            SIGNAL(sigSelectionChanged(sample_index_t,sample_index_t)),
            this,
            SLOT(selectionChanged(sample_index_t,sample_index_t)));
    connect(context, SIGNAL(sigUndoRedoInfo(QString,QString)),
            this,      SLOT(setUndoRedoInfo(QString,QString)));
    connect(context, SIGNAL(sigModified()),
            this,      SLOT(modifiedChanged()));

    // connect the zoom toolbar
    connect(context,   SIGNAL(sigZoomChanged(Kwave::FileContext*,double)),
            m_toolbar_zoom, SLOT(setZoomInfo(Kwave::FileContext*,double)));
    connect(context,             SIGNAL(destroyed(Kwave::FileContext*)),
            m_toolbar_zoom, SLOT(contextDestroyed(Kwave::FileContext*)));

    // connect the playback/record toolbar
    connect(context,
            SIGNAL(destroyed(Kwave::FileContext*)),
            m_toolbar_record_playback,
            SLOT(contextDestroyed(Kwave::FileContext*))
    );
    connect(context, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            m_toolbar_record_playback,
            SLOT(metaDataChanged(Kwave::MetaDataList)));

    // connect the status bar
    connect(context, SIGNAL(sigStatusBarMessage(QString,uint)),
            this,    SLOT(showStatusBarMessage(QString,uint)));
}

//***************************************************************************
Kwave::FileContext *Kwave::TopWidget::newFileContext()
{
    Q_ASSERT(m_toolbar_zoom);
    if (!m_toolbar_zoom) return nullptr;

    Kwave::FileContext *context =
        new(std::nothrow) Kwave::FileContext(m_application);
    if (!context) return nullptr;
    if (!context->init(this)) {
        delete context;
        return nullptr;
    }

    // if we are in SDI mode, there is a context but no MDI sub window
    // and in MDI/TAB mode we use this special entry for a context that
    // has just been created but has no sub window yet
    m_context_map[nullptr] = context;

    // do all signal/slot connections
    connectContext(context);

    // if we reach this point everything was ok, now we can safely switch
    // to the new context
    emit sigFileContextSwitched(context);

    return context;
}

//***************************************************************************
Kwave::FileContext *Kwave::TopWidget::currentContext() const
{
    if (m_context_map.isEmpty()) return nullptr;

    if (m_context_map.contains(nullptr)) {
        // the "null" entry is special, it has precedence over all
        // other entries. In SDI mode it is the only one ever and in
        // MDI/TAB mode it is used for a context that has no sub
        // window yet.
        return m_context_map[nullptr];
    }

    if (m_mdi_area) {
        // MDI or TAB mode
        QMdiSubWindow *current_sub = m_mdi_area->currentSubWindow();
        if (!m_context_map.contains(current_sub)) {
            qWarning("WARNING: unassociated MDI sub window %p?",
                     static_cast<void *>(current_sub));
            QMapIterator<QMdiSubWindow*, Kwave::FileContext*> it(m_context_map);
            Kwave::FileContext *context = nullptr;
            while (it.hasNext()) {
                it.next();
                context = it.value();
            }
            return (m_context_map.count() == 1) ? context : nullptr;
        }
        return m_context_map[current_sub];
    } else {
        // SDI mode
        Q_ASSERT(0 && "SDI mode but no context?");
        return nullptr;
    }
}

//***************************************************************************
bool Kwave::TopWidget::init()
{
    showInSplashSreen(i18n("Loading main menu..."));
    QMenuBar *menubar = menuBar();
    Q_ASSERT(menubar);
    if (!menubar) return false;
    m_menu_manager = new(std::nothrow) Kwave::MenuManager(this, *menubar);
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return false;

    // connect clicked menu entries with main communication channel of kwave
    connect(m_menu_manager, SIGNAL(sigMenuCommand(QString)),
            this, SLOT(forwardCommand(QString)));
    connect(&Kwave::ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
            this, SLOT(clipboardChanged(bool)));

    // --- zoom control toolbar ---

    m_toolbar_zoom = new(std::nothrow) Kwave::ZoomToolBar(this, TOOLBAR_ZOOM);
    Q_ASSERT(m_toolbar_zoom);
    if (!m_toolbar_zoom) return false;

    connect(this,  SIGNAL(sigFileContextSwitched(Kwave::FileContext*)),
            m_toolbar_zoom, SLOT(contextSwitched(Kwave::FileContext*)));

    // --- playback control toolbar ---
    m_toolbar_record_playback = new(std::nothrow) Kwave::PlayerToolBar(
        this, TOOLBAR_RECORD_PLAY, *m_menu_manager);
    Q_ASSERT(m_toolbar_record_playback);
    if (!m_toolbar_record_playback) return false;

    connect(this,  SIGNAL(sigFileContextSwitched(Kwave::FileContext*)),
            m_toolbar_record_playback,
            SLOT(contextSwitched(Kwave::FileContext*)));
    connect(m_toolbar_record_playback, SIGNAL(sigCommand(QString)),
            this,                    SLOT(forwardCommand(QString)));

    // -- create a new file context ---
    Kwave::FileContext *context = newFileContext();
    if (!context) return false;

    QWidget *central_widget = nullptr;
    switch (m_application.guiType()) {
        case Kwave::App::GUI_SDI:
            // create a main widget
            if (!context->createMainWidget(geometry().size() * 0.85))
                return false;
            central_widget = context->mainWidget();
            break;
        case Kwave::App::GUI_MDI:
            // create a MDI area if required, MDI mode
            m_mdi_area = new(std::nothrow) QMdiArea(this);
            Q_ASSERT(m_mdi_area);
            if (!m_mdi_area) return false;
            m_mdi_area->setViewMode(QMdiArea::SubWindowView);
            break;
        case Kwave::App::GUI_TAB:
            // create a MDI area if required, TAB mode
            m_mdi_area = new(std::nothrow) QMdiArea(this);
            Q_ASSERT(m_mdi_area);
            if (!m_mdi_area) return false;
            m_mdi_area->setViewMode(QMdiArea::TabbedView);
            m_mdi_area->setTabsClosable(true);
            m_mdi_area->setTabsMovable(true);
            break;
        DEFAULT_IMPOSSIBLE;
    }

    if (m_mdi_area) {
        connect(m_mdi_area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
                this,       SLOT(subWindowActivated(QMdiSubWindow*)) );
        central_widget = m_mdi_area;
    }

    // --- set up the toolbar ---

    showInSplashSreen(i18n("Initializing toolbar..."));
    KToolBar *toolbar_file = toolBar(TOOLBAR_FILE);
    Q_ASSERT(toolbar_file);
    if (!toolbar_file) return false;

    // --- file open and save ---

    toolbar_file->addAction(
        QIcon::fromTheme(_("document-new")),
        i18n("Create a new empty file"),
        this, SLOT(toolbarFileNew()));

    toolbar_file->addAction(
        QIcon::fromTheme(_("document-open")),
        i18n("Open an existing file"),
        this, SLOT(toolbarFileOpen()));

    m_action_save = toolbar_file->addAction(
        QIcon::fromTheme(_("document-save")),
        i18n("Save the current file"),
        this, SLOT(toolbarFileSave()));

    m_action_save_as = toolbar_file->addAction(
        QIcon::fromTheme(_("document-save-as")),
        i18n("Save the current file under a different name or file format..."),
        this, SLOT(toolbarFileSaveAs()));

    m_action_close = toolbar_file->addAction(
        QIcon::fromTheme(_("document-close")),
        i18n("Close the current file"),
        this, SLOT(toolbarFileClose()));

    // --- edit, cut&paste ---

    KToolBar *toolbar_edit = toolBar(TOOLBAR_EDIT);
    Q_ASSERT(toolbar_edit);
    if (!toolbar_edit) return false;

    m_action_undo = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-undo")),
        i18n("Undo"),
        this, SLOT(toolbarEditUndo()));

    m_action_redo = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-redo")),
        i18n("Redo"),
        this, SLOT(toolbarEditRedo()));

    m_action_cut = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-cut")),
        i18n("Cut"),
        this, SLOT(toolbarEditCut()));

    m_action_copy = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-copy")),
        i18n("Copy"),
        this, SLOT(toolbarEditCopy()));

    QAction *btPaste = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-paste")),
        i18n("Insert"),
        this, SLOT(toolbarEditPaste()));
    btPaste->setEnabled(!Kwave::ClipBoard::instance().isEmpty());
    connect(&Kwave::ClipBoard::instance(), SIGNAL(clipboardChanged(bool)),
            btPaste, SLOT(setEnabled(bool)));

    m_action_erase = toolbar_edit->addAction(
        QIcon::fromTheme(_("draw-eraser")),
        i18n("Mute selection"),
        this, SLOT(toolbarEditErase()));

    m_action_delete = toolbar_edit->addAction(
        QIcon::fromTheme(_("edit-delete")),
        i18n("Delete selection"),
        this, SLOT(toolbarEditDelete()));

    // set up the relationship between top widget / mdi area / main widget
    Q_ASSERT(central_widget);
    if (!central_widget)
        return false;

    setCentralWidget(central_widget);

    // set a nice initial size of the toplevel window
    int w = central_widget->minimumSize().width();
    w = qMax(w, central_widget->sizeHint().width());
    w = qMax(w, width());
    int h = qMax(central_widget->sizeHint().height(), (w * 6) / 10);
    h = qMax(h, height());
    resize(w, h);

    metaDataChanged(Kwave::MetaDataList());
    setUndoRedoInfo(QString(), QString());
    selectionChanged(0, 0);
    updateMenu();
    updateToolbar();
    updateRecentFiles();

    // make sure that everything of our window is visible
    QRect desk = qApp->primaryScreen()->availableGeometry();
    QRect g    = this->geometry();
    if (!desk.contains(g)) {
        // KDE's stupid geometry management has failed ?
        // this happens when one passes "-geometry <WIDTH>x<HEIGHT>" without
        // specifying a target position!?
        // passing "-geometry <WIDTH>x<HEIGHT>-<LEFT>-<TOP>" works...
        g = desk.intersected(g);
        setGeometry(g);
    }

    // enable saving of window size and position for next startup
    setAutoSaveSettings();

    // workaround for KDE4: detect first startup and set all toolbars
    // to "only symbols" mode
    KConfigGroup cfg = KSharedConfig::openConfig()->group(u"MainWindow"_s);
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
    closeAllSubWindows();

    delete m_toolbar_zoom;
    m_toolbar_zoom = nullptr;

    delete m_toolbar_record_playback;
    m_toolbar_record_playback = nullptr;

    delete m_menu_manager;
    m_menu_manager = nullptr;

    while (!m_context_map.isEmpty())
        delete m_context_map.take(m_context_map.lastKey());

    m_application.toplevelWindowHasClosed(this);
}

//***************************************************************************
QList<Kwave::App::FileAndInstance> Kwave::TopWidget::openFiles() const
{
    QList<Kwave::App::FileAndInstance> all_files;

    for (QMap<QMdiSubWindow *, Kwave::FileContext *>::const_iterator
         it(m_context_map.constBegin()); it != m_context_map.constEnd();
         ++it)
    {
        const Kwave::FileContext *context = it.value();
        if (!context) continue;
        QString name = context->signalName();
        if (!name.length()) continue;
        int instance_nr = context->instanceNr();
        all_files.append(Kwave::App::FileAndInstance(name, instance_nr));
    }

    return all_files;
}

//***************************************************************************
QList<Kwave::FileContext *> Kwave::TopWidget::detachAllContexts()
{
    QList<Kwave::FileContext *> list;

    QMutableMapIterator<QMdiSubWindow *, Kwave::FileContext *> i(m_context_map);
    while (i.hasNext()) {
        i.next();
        QMdiSubWindow      *sub     = i.key();
        Kwave::FileContext *context = i.value();

        // remove the entry from the map to prevent damage
        i.remove();

        if (sub) {
            // leave the "minimized" state before migration
            Qt::WindowStates state = sub->windowState();
            if (state & Qt::WindowMinimized)
            {
                state &= ~Qt::WindowMinimized;
                sub->setWindowState(state);
                sub->showNormal();
            }

            // detach the main widget from the MDI sub window
            sub->setWidget(nullptr);
            delete sub;
        }

        // detach the context from this parent widget
        if (context) {
            context->disconnect();
            context->setParent(nullptr);
            if (context->isInUse())
                list += context;    // in use -> keep
            else
                context->release(); // empty -> release
        }
    }

    // get rid of the MDI area, it should be empty now
    setCentralWidget(nullptr);
    if (m_mdi_area) {
        delete m_mdi_area;
        m_mdi_area = nullptr;
    }

    emit sigFileContextSwitched(nullptr);

    return list;
}

//***************************************************************************
void Kwave::TopWidget::insertContext(Kwave::FileContext *context)
{
    Kwave::FileContext *old_default_context = nullptr;

    // if no context was given: create a new empty default context
    if (!context) {
        context = newFileContext();
        Q_ASSERT(context);
        if (!context) return;
        if ( (m_application.guiType() == Kwave::App::GUI_SDI) &&
             !context->mainWidget() )
        {
            context->createMainWidget(geometry().size() * 0.85);
        }
        // prevent it from getting removed again
        m_context_map.remove(nullptr);
    }

    switch (m_application.guiType()) {
        case Kwave::App::GUI_SDI:
            // we may have an empty default context -> get rid of it
            Q_ASSERT(m_context_map.count() <= 1);
            if (!m_context_map.isEmpty()) {
                Kwave::FileContext *ctx = m_context_map[nullptr];
                m_context_map.remove(nullptr);
                Q_ASSERT(ctx != context);
                old_default_context = ctx;
            }
            // take over the new context
            m_context_map[nullptr] = context;
            connectContext(context);
            context->setParent(this);

            // set the central widget to the new main widget
            setCentralWidget(context->mainWidget());
            break;
        case Kwave::App::GUI_MDI:
            if (!m_mdi_area) {
                // create a MDI area if required, MDI mode
                m_mdi_area = new(std::nothrow) QMdiArea(this);
                Q_ASSERT(m_mdi_area);
                if (!m_mdi_area) return;
                m_mdi_area->setViewMode(QMdiArea::SubWindowView);
            }
            /* FALLTHROUGH */
        case Kwave::App::GUI_TAB: {
            // create a MDI area if required, TAB mode
            if (!m_mdi_area) {
                m_mdi_area = new(std::nothrow) QMdiArea(this);
                Q_ASSERT(m_mdi_area);
                if (!m_mdi_area) return;
                m_mdi_area->setViewMode(QMdiArea::TabbedView);
                m_mdi_area->setTabsClosable(true);
                m_mdi_area->setTabsMovable(true);
            }

            context->setParent(this);
            setCentralWidget(m_mdi_area);

            connect(m_mdi_area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
                    this,       SLOT(subWindowActivated(QMdiSubWindow*)) );

            QWidget *main_widget = context->mainWidget();
            if (main_widget) {
                QMdiSubWindow *sub = m_mdi_area->addSubWindow(
                    main_widget, Qt::SubWindow);
                Q_ASSERT(sub);
                if (!sub) return;
                sub->adjustSize();

                if (m_context_map.contains(nullptr))
                    m_context_map.remove(nullptr);
                m_context_map[sub] = context;

                connect(context->mainWidget(), SIGNAL(destroyed(QObject*)),
                        sub,                   SLOT(close()));
                connect(sub,  SIGNAL(destroyed(QObject*)),
                        this, SLOT(subWindowDeleted(QObject*)));
                connectContext(context);

                if (m_application.guiType() != Kwave::App::GUI_SDI) {
                    // this really sucks...
                    // Qt adds a "Close" entry to the MDI subwindow's system
                    // menu, with the shortcut "Ctrl+W". This collides with
                    // our own shortcut, produces a warning and makes the
                    // shortcut key not work:
                    // QAction::eventFilter: Ambiguous shortcut overload: Ctrl+W
                    QMenu *m = sub->systemMenu();
                    if (m) {
                        foreach (QAction *act, m->actions())
                            if (act) act->setShortcut(0); // remove shortcut
                    }
                }

                sub->setAttribute(Qt::WA_DeleteOnClose);
                main_widget->setWindowTitle(context->windowCaption(true));

                // workaround for stupid bug in Qt: when having only one
                // single sub window, switching to tab mode shows a tab +
                // a sub window with frame and title (not maximized within
                // the mdi area)
                Qt::WindowStates state = sub->windowState();
                if (m_application.guiType() == Kwave::App::GUI_TAB)
                {
                    state |=  Qt::WindowMaximized;
                    sub->setWindowState(state);
                    sub->show();
                }
                else
                {
                    state &= ~Qt::WindowMaximized;
                    sub->setWindowState(state);
                    sub->showNormal();
                }

                // NOTE: we have to mark the sub window as "not hidden",
                // otherwise currentSubWindow() would return a null pointer!
                sub->setHidden(false);
                m_mdi_area->setActiveSubWindow(sub);
            } else {
                m_context_map[nullptr] = context; // set empty default context
            }

            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    // update the menu bar, toolbar etc.
    emit sigFileContextSwitched(nullptr);
    emit sigFileContextSwitched(context);

    updateMenu();
    updateToolbar();
    updateCaption();

    if (old_default_context)
        old_default_context->release();
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

    if ((result = m_application.executeCommand(command)) != ENOSYS)
        return result;
    result = 0;
    if (false) {
    CASE_COMMAND("about_kde")
        // Help / About KDE
        KHelpMenu *dlg = new(std::nothrow) KHelpMenu(this);
        if (dlg) dlg->aboutKDE();
        result = 0;
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
            result = loadFile(Kwave::URLfromUserInput(filename));
        } else {
            // show file open dialog
            result = openFile();
        }
    CASE_COMMAND("openrecent")
        result = openRecent(command);
    CASE_COMMAND("quit")
        qApp->setQuitOnLastWindowClosed(false);
        result = (close()) ? EBUSY : -1;
    CASE_COMMAND("reset_toolbars")
        if ((result = (Kwave::MessageBox::questionYesNo(this,
            i18n("Reset the toolbar to default settings?"))
            == KMessageBox::PrimaryAction) ? 1 : 0))
        {
            resetToolbarToDefaults();
        }
        result = 0;
    CASE_COMMAND("select_gui_type")
        QString gui_type = parser.nextParam();
        Kwave::App::GuiType new_type = Kwave::App::GUI_SDI;

        if (gui_type == _("SDI"))
            new_type = Kwave::App::GUI_SDI;
        else if (gui_type == _("MDI"))
            new_type = Kwave::App::GUI_MDI;
        else if (gui_type == _("TAB"))
            new_type = Kwave::App::GUI_TAB;
        else
            return -1;

        KConfigGroup cfg = KSharedConfig::openConfig()->group(u"Global"_s);
        cfg.writeEntry(_("UI Type"), gui_type);
        m_application.switchGuiType(this, new_type);
        result = 0;
    CASE_COMMAND("reenable_dna")
        if ((result = (Kwave::MessageBox::questionYesNo(this,
            i18n("Re-enable all disabled notifications?\n"
                 "All messages that you previously turned off by activating "
                 "the \"Do not ask again\" checkbox will then be enabled again."
            ))
            == KMessageBox::PrimaryAction) ? 1 : 0))
        {
            KMessageBox::enableAllMessages();
        }
        result = 0;
    CASE_COMMAND("window:minimize")
        if (m_application.guiType() == Kwave::App::GUI_MDI) {
            // in case of MDI mode: minimize the current sub window
            if (m_mdi_area) {
                QMdiSubWindow *sub = m_mdi_area->activeSubWindow();
                if (!sub) return -1;
                sub->setWindowState(windowState() | Qt::WindowMinimized);
            }
        } else {
            // in case of TAB or SDI mode: minimize the toplevel window
            setWindowState(windowState() | Qt::WindowMinimized);
        }
        result = 0;
    CASE_COMMAND("window:next_sub")
        if (m_mdi_area) m_mdi_area->activateNextSubWindow();
        result = 0;
    CASE_COMMAND("window:prev_sub")
        if (m_mdi_area) m_mdi_area->activatePreviousSubWindow();
        result = 0;
    CASE_COMMAND("window:cascade")
        if (m_mdi_area) m_mdi_area->cascadeSubWindows();
        result = 0;
    CASE_COMMAND("window:tile")
        if (m_mdi_area) m_mdi_area->tileSubWindows();
        result = 0;
    CASE_COMMAND("window:tile_vertical")
        if (!m_mdi_area) return 0;

        // determine the number of not minimized sub windows
        int count = 0;
        foreach (QMdiSubWindow *sub, m_mdi_area->subWindowList()) {
            if (sub && !(sub->windowState() & Qt::WindowMinimized))
                ++count;
        }
        if (!count) return 0;

        int total_height = m_mdi_area->height();
        int width = m_mdi_area->width();
        int height = total_height / count;
        int increment = height;
        int y = 0;
        foreach (QMdiSubWindow *sub, m_mdi_area->subWindowList()) {
            if (!sub || (sub->windowState() & Qt::WindowMinimized))
                continue;
            // resize/move the sub window
            sub->resize(width, height);
            sub->move(0, y);
            y += increment;
        }
        result = 0;

    CASE_COMMAND("window:activate")
        if (m_mdi_area) {
            QString title = parser.nextParam();
            for (QMap<QMdiSubWindow *, Kwave::FileContext *>::const_iterator
                it(m_context_map.constBegin()); it != m_context_map.constEnd();
                ++it)
            {
                QMdiSubWindow            *sub     = it.key();
                const Kwave::FileContext *context = it.value();
                if (!sub  || !context) continue;

                // identify the window by it's title
                if (context->windowCaption(false) == title) {
                    // activate the sub window if it is not the active one
                    if (m_mdi_area->activeSubWindow() != sub)
                        m_mdi_area->setActiveSubWindow(sub);

                    // leave the "minimized" state if necessary
                    Qt::WindowStates state = sub->windowState();
                    if (state & Qt::WindowMinimized)
                        sub->setWindowState(state & ~(Qt::WindowMinimized));
                    sub->raise();
                    return 0;
                }
            }
        } else return ENOSYS;
        return EINVAL;
    } else {
        return ENOSYS; // command not implemented (here)
    }

    return result;
}

//***************************************************************************
int Kwave::TopWidget::forwardCommand(const QString &command)
{
    Kwave::FileContext *context = currentContext();
    if (!context) return EAGAIN;

    // execute the command in the current context
    int retval = context->executeCommand(command);

    // special handling for application shutdown, e.g. "quit()"
    if (retval == EBUSY)
        return -1; // do not continue and do a "sync"

    // synchronize after the command
    context = currentContext();
    if (context != nullptr) {
        Kwave::PluginManager *plugin_manager = context->pluginManager();
        if (plugin_manager != nullptr) plugin_manager->sync();
    }

    return retval;
}

//***************************************************************************
bool Kwave::TopWidget::closeAllSubWindows()
{
    bool allowed = true;

    QMutableMapIterator<QMdiSubWindow *, Kwave::FileContext *>
        it(m_context_map);
    it.toBack();
    while (it.hasPrevious()) {
        it.previous();
        QMdiSubWindow      *sub     = it.key();
        Kwave::FileContext *context = it.value();

        if (!sub) {
            // reached the default context (without sub windows)
            // or SDI mode with only one context
            if (context) allowed &= context->closeFile();
            break;
        }

        if (!context) {
            // invalid entry?
            it.remove();
            continue;
        }

        // try to close the sub window / context
        if (!sub->close()) {
            allowed = false;
            break;
        } else {
            it.remove();
            if (m_context_map.isEmpty()) {
                // keep the default context, without a window
                m_context_map[nullptr] = context;
                break;
            }
        }
    }

    return allowed;
}

//***************************************************************************
int Kwave::TopWidget::newWindow(Kwave::FileContext *&context, const QUrl &url)
{
    switch (m_application.guiType()) {
        case Kwave::App::GUI_SDI: {
            // SDI mode and already something loaded
            // -> open a new toplevel window
            //    (except for processing commands per kwave: URL
            Kwave::SignalManager *signal_manager = (context) ?
                context->signalManager() : nullptr;
            if ( signal_manager && !signal_manager->isEmpty() &&
                (url.scheme().toLower() != Kwave::urlScheme()) )
                return m_application.newWindow(url);

            // try to close the previous file
            if (context && !context->closeFile()) return -1;

            break;
        }
        case Kwave::App::GUI_MDI: /* FALLTHROUGH */
        case Kwave::App::GUI_TAB:
        {
            // MDI or TAB mode: open a new sub window
            Q_ASSERT(m_mdi_area);
            if (!m_mdi_area) return -1;

            if (context && context->isEmpty()) {
                // current context is empty, no main widget etc -> discard it
                if (m_context_map.contains(nullptr)) {
                    // must have been the default context
                    Q_ASSERT(m_context_map[nullptr] == context);
                    m_context_map.remove(nullptr);
                }

                // NOTE: do not handle the following context switch
                //       notification, it might be handled with a
                //       refcount that has already been set to zero
                disconnect(
                    this,
                    SIGNAL(sigFileContextSwitched(Kwave::FileContext*)),
                    context,
                    SLOT(contextSwitched(Kwave::FileContext*))
                );
                context->release();
            }

            // create a new file context
            context = newFileContext();
            if (!context) return -1;

            // create a main widget
            if (!context->createMainWidget(
                m_mdi_area->geometry().size() * 0.85)) return -1;

            // insert the context into this instance
            insertContext(context);
            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    return 1;
}

//***************************************************************************
int Kwave::TopWidget::loadFile(const QUrl &url)
{
    Kwave::FileContext *context = currentContext();

    // special handling for kwave: URLs
    if (url.scheme().toLower() == Kwave::urlScheme()) {
        QString cmd = Kwave::Parser::fromUrl(url);
        Kwave::Logger::log(this, Kwave::Logger::Info,
            _("CMD: from command line: '") + cmd + _("'"));
        Kwave::Splash::showMessage(i18n("Executing command '%1'...", cmd));
        return (context) ? context->executeCommand(cmd) : executeCommand(cmd);
    }

    if (!context) return -1;
    Kwave::SignalManager *signal_manager = context->signalManager();
    Q_ASSERT(signal_manager);

    // add an entry to the list of recent files
    m_application.addRecentFile(url.isLocalFile() ?
        url.toLocalFile() : url.toDisplayString());

    // abort if new file not valid and local
    if (!url.isLocalFile()) return -1;

    // detect whether it is a macro (batch) file
    QFileInfo file(url.fileName());
    QString suffix = file.suffix();
    if (suffix == _("kwave")) {
        Kwave::Splash::showMessage(
            i18n("Executing Kwave script file '%1'...", url.toDisplayString())
        );
        return context->loadBatch(url);
    }

    // open a new window (empty in case of MDI/TAB)
    int retval = newWindow(context, url);
    if ((retval <= 0) || !context) return retval;

    Kwave::Splash::showMessage(
        i18n("Loading file '%1'...", url.toDisplayString())
    );

    // NOTE: the context may have changed, now we may have a different
    //       signal manager
    signal_manager = context->signalManager();
    int res = (signal_manager) ? signal_manager->loadFile(url) : -ENOMEM;
    if (res != 0) {
        qWarning("TopWidget::loadFile() failed: result=%d", res);
        QString reason;
        switch (res) {
            case -ENOMEM:
                reason = i18n("Out of memory");
                break;
            case -EIO:
                reason = i18nc("error message after opening a file failed",
                               "Unable to open '%1'", url.toDisplayString());
                break;
            case -EINVAL:
                reason = i18nc("error message after opening a file failed",
                               "Invalid or unknown file type: '%1'",
                               url.toDisplayString());
                break;
            default:
                reason = _("");
        }

        // show an error message box if the reason was known
        if (reason.length()) {
            Kwave::MessageBox::error(this, reason);
        }

        // load failed
        context->closeFile();
    }

    updateMenu();
    updateToolbar();

    return 0;
}

//***************************************************************************
int Kwave::TopWidget::openRecent(const QString &str)
{
    Kwave::Parser parser(str);
    return loadFile(Kwave::URLfromUserInput(parser.firstParam()));
}

//***************************************************************************
int Kwave::TopWidget::openFile()
{
    QString filter = Kwave::CodecManager::decodingFilter();
    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        _("kfiledialog:///kwave_open_dir"),
        Kwave::FileDialog::OpenFile, filter, this
    );
    if (!dlg) return -1;
    dlg->setWindowTitle(i18n("Open"));
    if (dlg->exec() == QDialog::Accepted) {
        QUrl url = dlg->selectedUrl();
        delete dlg;
        return loadFile(url);
    } else {
        delete dlg;
        return -1;
    }
}

//***************************************************************************
int Kwave::TopWidget::newSignal(sample_index_t samples, double rate,
                                unsigned int bits, unsigned int tracks)
{
    Kwave::FileContext *context = currentContext();
    if (!context) return -1;

    Kwave::SignalManager *signal_manager = context->signalManager();
    if (!signal_manager) return -1;

    QUrl url = Kwave::Parser::toUrl(
        _("newsignal(%1,%2,%3,%4)"
        ).arg(samples).arg(rate).arg(bits).arg(tracks));
    int retval = newWindow(context, url);
    if (retval <= 0) return retval;

    signal_manager = context->signalManager();
    if (!signal_manager) return -1;

    signal_manager->newSignal(samples, rate, bits, tracks);
    return 0;
}

//***************************************************************************
void Kwave::TopWidget::metaDataChanged(Kwave::MetaDataList meta_data)
{
    Q_ASSERT(statusBar());
    if (!statusBar() || !m_menu_manager) return;
    QString txt;

    const Kwave::FileInfo info(meta_data);
    sample_index_t length = info.length();
    unsigned int tracks   = info.tracks();
    double rate           = info.rate();
    unsigned int bits     = info.bits();

    // length in milliseconds
    if (length) {
        double ms = (rate > 0) ? (static_cast<double>(length) /
            static_cast<double>(rate) * 1E3) : 0;
        txt = _(" ") + i18nc(
            "Length, as in total duration of loaded song",
            "Length: %1 (%2 samples)",
            Kwave::ms2string(ms),
            Kwave::samples2string(length)
        ) + _(" ");
    } else txt = _("");
    m_lbl_status_size->setText(txt);

    // sample rate and resolution
    if (bits) {
        QString khz;
        khz.setNum(static_cast<double>(rate) * 1E-3, 'f', 3);
        txt = _(" ") + i18n("Mode: %1 kHz @ %2 Bit", khz, bits) +  _(" ");
    } else txt = _("");
    m_lbl_status_mode->setText(txt);

    // remove selection/position display on file close
    bool have_signal = (tracks != 0);
    if (!have_signal) selectionChanged(0, 0);

    // update the menu
    updateMenu();

    // update the toolbar as well
    updateToolbar();

    // update the window caption
    updateCaption();
}

//***************************************************************************
void Kwave::TopWidget::selectionChanged(sample_index_t offset,
                                        sample_index_t length)
{
    const Kwave::FileContext *context = currentContext();
    if (!context) return;

    Kwave::SignalManager *signal_manager = context->signalManager();
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
        sample_index_t last = offset + length - 1;

        QString txt = _(" ");
        if (sample_mode) {
            txt += i18nc(
                "%1=first sample, %2=last sample, %3=number of samples, "\
                "example: 'Selected: 2000...3000 (1000 samples)'",
                "Selected: %1...%2 (%3 samples)",
                Kwave::samples2string(offset),
                Kwave::samples2string(last),
                Kwave::samples2string(length)
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
        if (m_menu_manager)
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

        if (m_menu_manager)
            m_menu_manager->setItemEnabled(_("@SELECTION"), false);
    }

    // update the toolbar on selection change, maybe the
    // button for "zoom selection" has to be enabled/disabled
    updateToolbar();
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

    foreach (const QString &file, m_application.recentFiles())
        m_menu_manager->addNumberedMenuEntry(
            _("ID_FILE_OPEN_RECENT"), file, QString());

    // enable/disable the "clear" menu entry in Files / Open Recent
    m_menu_manager->setItemEnabled(_("ID_FILE_OPEN_RECENT_CLEAR"),
        !m_application.recentFiles().isEmpty());
}

//***************************************************************************
void Kwave::TopWidget::updateMenu()
{
    const Kwave::FileContext *context = currentContext();
    Kwave::SignalManager *signal_manager =
        (context) ? context->signalManager() : nullptr;
    Q_ASSERT(m_menu_manager);
    if (!m_menu_manager) return;

    bool have_window_menu = false;
    switch (m_application.guiType()) {
        case Kwave::App::GUI_SDI:
            m_menu_manager->selectItem(_("@GUI_TYPE"), _("ID_GUI_SDI"));
            m_menu_manager->setItemVisible(_("ID_WINDOW"),               false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_NEXT"),          false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_PREV"),          false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_CASCADE"),       false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE"),          false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE_VERTICAL"), false);
            m_menu_manager->setItemVisible(_("ID_FILE_NEW_WINDOW"),      true);
            break;
        case Kwave::App::GUI_MDI:
            m_menu_manager->selectItem(_("@GUI_TYPE"), _("ID_GUI_MDI"));
            m_menu_manager->setItemVisible(_("ID_WINDOW"),               true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_NEXT"),          true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_PREV"),          true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_CASCADE"),       true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE"),          true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE_VERTICAL"), true);
            m_menu_manager->setItemVisible(_("ID_FILE_NEW_WINDOW"),      false);
            have_window_menu = true;
            break;
        case Kwave::App::GUI_TAB:
            m_menu_manager->selectItem(_("@GUI_TYPE"), _("ID_GUI_TAB"));
            m_menu_manager->setItemVisible(_("ID_WINDOW"),               true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_NEXT"),          true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_PREV"),          true);
            m_menu_manager->setItemVisible(_("ID_WINDOW_CASCADE"),       false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE"),          false);
            m_menu_manager->setItemVisible(_("ID_WINDOW_TILE_VERTICAL"), false);
            m_menu_manager->setItemVisible(_("ID_FILE_NEW_WINDOW"),      false);
            have_window_menu = true;
            break;
        DEFAULT_IMPOSSIBLE;
    }

    if (have_window_menu) {
        // update the "Windows" menu
        m_menu_manager->clearNumberedMenu(_("ID_WINDOW_LIST"));
        unsigned int win_count = 0;
        for (QMap<QMdiSubWindow *, Kwave::FileContext *>::const_iterator
             it(m_context_map.constBegin()); it != m_context_map.constEnd();
             ++it)
        {
            const Kwave::FileContext *ctx = it.value();
            if (!ctx) continue;
            QString caption = ctx->windowCaption(false);
            if (!caption.length()) continue;
            m_menu_manager->addNumberedMenuEntry(_("ID_WINDOW_LIST"),
                caption, QString());
            ++win_count;
        }

        bool on = (win_count > 1);
        m_menu_manager->setItemEnabled(_("ID_WINDOW_NEXT"),          on);
        m_menu_manager->setItemEnabled(_("ID_WINDOW_PREV"),          on);
        m_menu_manager->setItemEnabled(_("ID_WINDOW_CASCADE"),       on);
        m_menu_manager->setItemEnabled(_("ID_WINDOW_TILE"),          on);
        m_menu_manager->setItemEnabled(_("ID_WINDOW_TILE_VERTICAL"), on);
    }

    // enable/disable all items that depend on having a file
    bool have_file = (context && context->signalName().length());
    m_menu_manager->setItemEnabled(_("@NOT_CLOSED"), have_file);

    // enable/disable all items that depend on having a label
    // and update the label menu
    bool have_labels = false;
    if (signal_manager) {
        Kwave::LabelList labels(signal_manager->metaData());
        have_labels = !labels.isEmpty();

        m_menu_manager->clearNumberedMenu(_("ID_LABEL_DELETE"));
        if (labels.count()) {

            // add special entry to delete all labels
            m_menu_manager->addNumberedMenuEntry(
                _("ID_LABEL_DELETE"),
                i18nc("special entry in the list of labels to delete all",
                      "(All)"), _("-1"));

            // iterate over the list of labels
            unsigned int index = 0;
            foreach (const Kwave::Label &label, labels) {
                QString name = label.name();
                QString desc = (name.length()) ?
                    i18nc(
                    "list menu entry of a label, %1=index, %2=description/name",
                    "#%1 (%2)", index, name) :
                    i18nc("list menue entry of a label, "
                            "without description, %1=index",
                            "#%1", index);
                m_menu_manager->addNumberedMenuEntry(
                    _("ID_LABEL_DELETE"), desc, name.setNum(index));
                index++;
            }
        }
    }
    m_menu_manager->setItemEnabled(_("@LABELS"), have_labels);

    // update the list of deletable tracks
    unsigned int tracks = (signal_manager) ? signal_manager->tracks() : 0;
    m_menu_manager->clearNumberedMenu(_("ID_EDIT_TRACK_DELETE"));
    QString buf;
    for (unsigned int i = 0; i < tracks; i++) {
        m_menu_manager->addNumberedMenuEntry(
            _("ID_EDIT_TRACK_DELETE"), buf.setNum(i), buf.setNum(i));
    }

    // enable/disable all items that depend on having a signal
    bool have_signal = (tracks != 0);
    m_menu_manager->setItemEnabled(_("@SIGNAL"), have_signal);

    // revert is only possible if the signal exists, is modified and there
    // is a file behind from which we could reload (which is not the case
    // after File/New...).
    bool enable_revert = false;
    if (signal_manager) {
        bool have_filename = Kwave::FileInfo(
            signal_manager->metaData()
        ).contains(Kwave::INF_FILENAME);
        bool is_modified = signal_manager->isModified();
        enable_revert = have_filename && is_modified;
    }
    m_menu_manager->setItemEnabled(_("ID_FILE_REVERT"), enable_revert);

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
    const Kwave::FileContext *context = currentContext();
    if (!context) return;

    Kwave::SignalManager *signal_manager = context->signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    bool have_signal = signal_manager->tracks();
    if (m_action_save)
        m_action_save->setEnabled(have_signal);
    if (m_action_save_as)
        m_action_save_as->setEnabled(have_signal);
    if (m_action_close)
        m_action_close->setEnabled(have_signal);

    bool have_selection = (signal_manager->selection().length() > 1);
    if (m_action_cut)
        m_action_cut->setEnabled(have_selection);
    if (m_action_copy)
        m_action_copy->setEnabled(have_selection);
    if (m_action_erase)
        m_action_erase->setEnabled(have_selection);
    if (m_action_delete)
        m_action_delete->setEnabled(have_selection);

    // update the zoom toolbar
    if (m_toolbar_zoom)
        m_toolbar_zoom->updateToolbar();
}

//***************************************************************************
void Kwave::TopWidget::modifiedChanged()
{
    updateCaption();
    updateMenu();
}

//***************************************************************************
void Kwave::TopWidget::updateCaption()
{
    const Kwave::FileContext *context = currentContext();
    QString caption = (context) ? context->windowCaption(true) : QString();
    setCaption(caption);
}

//***************************************************************************
void Kwave::TopWidget::closeEvent(QCloseEvent *e)
{
    (closeAllSubWindows()) ? e->accept() : e->ignore();
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
    QStatusBar *status_bar = statusBar();
    if (!status_bar) return;

    if (msg.length())
        status_bar->showMessage(msg, ms);
    else
        status_bar->clearMessage();
}

//***************************************************************************
void Kwave::TopWidget::subWindowActivated(QMdiSubWindow *sub)
{
    if (!sub || !m_context_map.contains(sub)) return;
    emit sigFileContextSwitched(currentContext());
}

//***************************************************************************
void Kwave::TopWidget::subWindowDeleted(QObject *obj)
{
    QMdiSubWindow *sub = static_cast<QMdiSubWindow *>(obj);
    if (!sub || !m_context_map.contains(sub)) {
        // sub window is not in the map, maybe it has already been detached
        return;
    }

    Kwave::FileContext *context = m_context_map[sub];
    Q_ASSERT(context);
    if (!context) return;

    m_context_map.remove(sub);

    if (m_context_map.isEmpty()) {
        // keep the default context, without a window
        m_context_map[nullptr] = context;
    } else {
        // remove the context
        context->release();
    }
}

//***************************************************************************
void Kwave::TopWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) return;
    if ((event->proposedAction() != Qt::MoveAction) &&
        (event->proposedAction() != Qt::CopyAction))
        return; /* unsupported action */

    if (Kwave::FileDrag::canDecode(event->mimeData()))
        event->acceptProposedAction();
}

//***************************************************************************
void Kwave::TopWidget::dropEvent(QDropEvent *event)
{
    if (!event) return;
    if (!event->mimeData()) return;

    if (event->mimeData()->hasUrls()) {
        bool first = true;
        foreach (const QUrl &url, event->mimeData()->urls()) {
            QString filename = url.toLocalFile();
            QString mimetype = Kwave::CodecManager::mimeTypeOf(url);
            if (Kwave::CodecManager::canDecode(mimetype)) {
                if (first) {
                    // first dropped URL -> open in this window
                    forwardCommand(_("open(%1)").arg(
                        Kwave::Parser::escape(filename)));
                    first = false;
                } else {
                    // all others -> open a new window
                    forwardCommand(_("newwindow(%1)").arg(
                        Kwave::Parser::escape(filename)));
                }
            }
        }
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_TopWidget.cpp"
