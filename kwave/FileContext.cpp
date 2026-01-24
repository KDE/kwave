/***************************************************************************
    kwave/FileContext.cpp  -  Context of a Loaded File
                             -------------------
    begin                : 2010-01-02
    copyright            : (C) 2010 by Thomas.Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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
#include <new>

#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QPointer>
#include <QTextStream>
#include <QMdiSubWindow>
#include <QStandardPaths>

#include <KLocalizedString>

#include "libkwave/CodecManager.h"
#include "libkwave/Encoder.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/Parser.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"

#include "App.h"
#include "FileContext.h"
#include "MainWidget.h"
#include "Splash.h"
#include "TopWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == _(x)) {

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
Kwave::FileContext::FileContext(Kwave::App &app)
    :QObject(),
     m_use_count(1),
     m_application(app),
     m_top_widget(nullptr),
     m_main_widget(nullptr),
     m_signal_manager(nullptr),
     m_plugin_manager(nullptr),
     m_active(true),
     m_last_zoom(0),
     m_last_playback_pos(0),
     m_last_status_message_text(),
     m_last_status_message_timer(),
     m_last_status_message_ms(0),
     m_last_undo(QString()),
     m_last_redo(QString()),
     m_instance_nr(-1),
     m_delayed_command_timer(),
     m_delayed_command_queue()
{
    m_delayed_command_timer.setSingleShot(true);
    connect(&m_delayed_command_timer, SIGNAL(timeout()),
            this, SLOT(processDelayedCommand()));
}

//***************************************************************************
Kwave::FileContext::~FileContext()
{
    delete m_main_widget;
    m_main_widget = nullptr;

    m_top_widget = nullptr;

    delete m_plugin_manager;
    m_plugin_manager = nullptr;

    delete m_signal_manager;
    m_signal_manager = nullptr;
}

//***************************************************************************
void Kwave::FileContext::use()
{
    Q_ASSERT(int(m_use_count) > 0);
    m_use_count.ref();
}

//***************************************************************************
void Kwave::FileContext::release()
{
    Q_ASSERT(int(m_use_count) > 0);
    if (m_use_count.deref() == false) {
        disconnect();
        deleteLater();
    }
}

//***************************************************************************
bool Kwave::FileContext::createMainWidget(const QSize &preferred_size)
{
    Q_ASSERT(!m_main_widget);

    // create the main widget
    m_main_widget = new(std::nothrow) Kwave::MainWidget(
        m_top_widget, *this, preferred_size
    );
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return false;
    if (!(m_main_widget->isOK())) {
        delete m_main_widget;
        m_main_widget = nullptr;
        return false;
    }

    // connect the main widget
    connect(&(m_signal_manager->playbackController()),
            SIGNAL(sigSeekDone(sample_index_t)),
            m_main_widget, SLOT(scrollTo(sample_index_t)));
    connect(m_main_widget, SIGNAL(sigCommand(QString)),
            this,          SLOT(executeCommand(QString)));
    connect(m_main_widget, SIGNAL(sigZoomChanged(double)),
            this,          SLOT(forwardZoomChanged(double)));
    connect(m_main_widget, SIGNAL(sigVisibleRangeChanged(sample_index_t,
            sample_index_t, sample_index_t)),
            this, SLOT(visibleRangeChanged(sample_index_t,
            sample_index_t, sample_index_t)) );

    return true;
}

//***************************************************************************
bool Kwave::FileContext::init(Kwave::TopWidget *top_widget)
{
    Kwave::FileContext::UsageGuard _keep(this);

    m_top_widget = top_widget;
    Q_ASSERT(m_top_widget);
    if (!m_top_widget) return false;

    m_signal_manager = new(std::nothrow)
        Kwave::SignalManager(m_top_widget);
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return false;

    m_plugin_manager = new(std::nothrow)
        Kwave::PluginManager(m_top_widget, *m_signal_manager);
    Q_ASSERT(m_plugin_manager);
    if (!m_plugin_manager) return false;

    // connect the signal manager
    connect(m_signal_manager, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            this,             SLOT(metaDataChanged(Kwave::MetaDataList)));
    connect(&(m_signal_manager->selection()),
            SIGNAL(changed(sample_index_t,sample_index_t)),
            this,
            SLOT(selectionChanged(sample_index_t,sample_index_t)));
    connect(m_signal_manager, SIGNAL(sigUndoRedoInfo(const QString&,
                                                     const QString&)),
            this, SLOT(setUndoRedoInfo(QString,QString)));
    connect(m_signal_manager, SIGNAL(sigModified()),
            this,             SLOT(modifiedChanged()));

    // connect the plugin manager
    connect(m_plugin_manager, SIGNAL(sigCommand(QString)),
            this,             SLOT(executeCommand(QString)));

    // connect the playback controller
    connect(&(m_signal_manager->playbackController()),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this, SLOT(updatePlaybackPos(sample_index_t)));

    setParent(top_widget);

    Kwave::Splash::showMessage(i18n("Scanning plugins..."));
    m_plugin_manager->searchPluginModules();

    // load the menu from file
    QFile menufile(QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        _("kwave/menus.config")
    ));
    if (!menufile.open(QIODevice::ReadOnly)) {
        qWarning("menu file not found in:");
        const QStringList locations = QStandardPaths::standardLocations(
            QStandardPaths::GenericDataLocation);
        for (const QString &location : locations)
        {
            qWarning("    '%s'", DBG(location));
        }
        return false;
    }
    QTextStream stream(&menufile);
    if (stream.atEnd()) {
        qWarning("menu file not found in:");
        QStringList locations = QStandardPaths::standardLocations(
            QStandardPaths::GenericDataLocation);
        foreach (const QString &location, locations)
        {
            qWarning("    '%s'", DBG(location));
        }
    }
    Q_ASSERT(!stream.atEnd());
    if (!stream.atEnd()) parseCommands(stream);
    menufile.close();

    // now we are initialized, load all plugins
    Kwave::Splash::showMessage(i18n("Loading plugins..."));
    statusBarMessage(i18n("Loading plugins..."), 0);
    if (!m_plugin_manager->loadAllPlugins()) {
        statusBarMessage(i18n("Failed"), 1000);
        QApplication::restoreOverrideCursor();
        Kwave::MessageBox::error(top_widget,
            i18n("Kwave has not been properly installed. "\
                 "No plugins found!")
        );
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        return false;
    }
    statusBarMessage(i18n("Ready"), 1000);

    return true;
}

//***************************************************************************
void Kwave::FileContext::setParent(Kwave::TopWidget *top_widget)
{
    if (m_top_widget) {
        Kwave::TopWidget *old = m_top_widget;

        // disconnect all old signal/slot relationships
        if (m_plugin_manager)
            disconnect(m_plugin_manager, SIGNAL(sigProgress(QString)),
                       old,              SLOT(showInSplashSreen(QString)));
        disconnect(old,  SIGNAL(sigFileContextSwitched(Kwave::FileContext*)),
                   this, SLOT(contextSwitched(Kwave::FileContext*)));

        if (m_signal_manager) m_signal_manager->setParentWidget(nullptr);
        if (m_plugin_manager) m_plugin_manager->setParentWidget(nullptr);
        if (m_main_widget)    m_main_widget->setParent(nullptr);

        m_active = false;
    }

    // set the new top widget
    m_top_widget = top_widget;

    if (m_top_widget) {
        QWidget *top = m_top_widget;

        connect(top,  SIGNAL(sigFileContextSwitched(Kwave::FileContext*)),
                this, SLOT(contextSwitched(Kwave::FileContext*)));
        if (m_plugin_manager)
            connect(m_plugin_manager, SIGNAL(sigProgress(QString)),
                    top,              SLOT(showInSplashSreen(QString)));

        if (m_signal_manager) m_signal_manager->setParentWidget(m_top_widget);
        if (m_plugin_manager) m_plugin_manager->setParentWidget(m_top_widget);
        if (m_main_widget)    m_main_widget->setParent(m_top_widget);
    }
}

//***************************************************************************
QWidget *Kwave::FileContext::mainWidget() const
{
    return static_cast<QWidget *>(m_main_widget);
}

//***************************************************************************
Kwave::SignalManager *Kwave::FileContext::signalManager() const
{
    Q_ASSERT(m_signal_manager);
    return m_signal_manager;
}

//***************************************************************************
Kwave::PluginManager *Kwave::FileContext::pluginManager() const
{
    return m_plugin_manager;
}

//***************************************************************************
Kwave::Zoomable* Kwave::FileContext::zoomable() const
{
    return m_main_widget;
}

//***************************************************************************
int Kwave::FileContext::delegateCommand(const char *plugin,
                                        Kwave::Parser &parser,
                                        unsigned int param_count)
{
    if (!m_plugin_manager) return -1;
    if (parser.count() != param_count) return -EINVAL;

    QStringList params;
    params.append(parser.command());
    params.append(parser.remainingParams());
    int result = m_plugin_manager->setupPlugin(_(plugin), params);
    if (result > 0) result = 0;
    return result;
}

//***************************************************************************
int Kwave::FileContext::executeCommand(const QString &line)
{
    Kwave::FileContext::UsageGuard _keep(this);

    int result = 0;
    bool use_recorder = true;
    QString command = line;

//     qDebug("Kwave::FileContext[%p]::executeCommand(%s)", this, DBG(command));

    Q_ASSERT(m_plugin_manager);
    Q_ASSERT(m_top_widget);
    if (!m_plugin_manager || !m_top_widget) return -ENOMEM;

    if (!command.length()) return 0; // empty line -> nothing to do
    if (command.trimmed().startsWith(_("#")))
        return 0; // only a comment

    // special case: if the command contains ";" it is a list of
    // commands -> macro !
    Kwave::Parser parse_list(command);
    if (parse_list.hasMultipleCommands()) {
        QStringList macro = parse_list.commandList();
        foreach (const QString &it, macro) {
            result = executeCommand(_("nomacro:") + it);
            Q_ASSERT(!result);
            if (result) {
                qWarning("macro execution of '%s' failed: %d",
                         DBG(it), result);
                return result; // macro failed :-(
            }

            // wait until the command has completed !
            m_plugin_manager->sync();
        }
        return result;
    }

    // check if the macro recorder has to be disabled for this command
    if (command.startsWith(_("nomacro:"))) {
        use_recorder = false;
        command = command.mid(QString(_("nomacro:")).length());
    }

    // expand variables
    if (command.contains(_("${"))) {
        // current language
        if (command.contains(_("${LANG}"))) {
            QLocale locale;
            if (!m_main_widget.isNull()) locale = m_main_widget->locale();
            QString lang = locale.name().split(_("-")).at(0);
            command.replace(_("${LANG}"), lang);
        }
    }

    // log all commands to the log file if enabled
    Kwave::Logger::log(this, Kwave::Logger::Info, _("CMD: ") + line);

    // parse one single command
    Kwave::Parser parser(command);
    QString cmd = parser.command();

    // exclude menu commands from the recorder
    if (cmd == _("menu")) use_recorder = false;

    // only record plugin:execute, not plugin without parameters
    if (cmd == _("plugin")) use_recorder = false;

    // let through all commands that handle zoom/view or playback like fwd/rew
    bool allow_always =
        (cmd == _("playback")) ||
        cmd.startsWith(_("view:")) ||
        cmd.startsWith(_("playback:")) ||
        cmd.startsWith(_("select_track:")) ||
        (cmd == _("close")) ||
        (cmd == _("quit")) ||
        (cmd == _("window:screenshot")) ||
        (cmd == _("window:sendkey"))
        ;

    // all others only if no plugin is currently running
    if (!allow_always && m_plugin_manager->onePluginRunning())
    {
        qWarning("FileContext::executeCommand('%s') - currently not possible, "
                 "a plugin is running :-(",
                 DBG(cmd));
        return -1;
    }

    if (use_recorder) {
        // append the command to the macro recorder
        // @TODO macro recording...
        qDebug("# %s ", DBG(command));
    }

    if ((result = m_top_widget->executeCommand(command)) != ENOSYS)
        return result;

    if (false) {
    CASE_COMMAND("close")
        result = closeFile() ? 0 : 1;
    CASE_COMMAND("delayed")
        if (parser.count() != 2)
            return -EINVAL;
        unsigned int delay         = parser.firstParam().toUInt();
        QString      delayed_cmd   = parser.nextParam();
        enqueueCommand(delay, delayed_cmd);
        result = 0;
    CASE_COMMAND("loadbatch")
        result = loadBatch(QUrl(parser.nextParam()));
    CASE_COMMAND("plugin")
        QString name(parser.firstParam());
        QStringList params(parser.remainingParams());
        qDebug("FileContext::executeCommand(): loading plugin '%s'", DBG(name));
        qDebug("FileContext::executeCommand(): with %lld parameter(s)",
                params.count());
        result = m_plugin_manager->executePlugin(
            name, params.count() ? &params : nullptr);
    CASE_COMMAND("plugin:execute")
        QString name(parser.firstParam());
        QStringList params(parser.remainingParams());
        result = m_plugin_manager->executePlugin(name, &params);
    CASE_COMMAND("plugin:setup")
        QString name(parser.firstParam());
        QStringList params(parser.remainingParams());
        result = m_plugin_manager->setupPlugin(name, params);
        if (result > 0) result = 0;
    CASE_COMMAND("revert")
        result = revert();
    CASE_COMMAND("save")
        result = saveFile();
    CASE_COMMAND("saveas")
        result = saveFileAs(parser.nextParam(), false);
    CASE_COMMAND("saveselect")
        result = saveFileAs(QString(), true);
    CASE_COMMAND("sync")
        while (!m_delayed_command_queue.isEmpty()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        result = 0;
    CASE_COMMAND("window:click")
        result = delegateCommand("debug", parser, 3);
    CASE_COMMAND("window:close")
        result = delegateCommand("debug", parser, 1);
    CASE_COMMAND("window:mousemove")
        result = delegateCommand("debug", parser, 3);
    CASE_COMMAND("window:resize")
        result = delegateCommand("debug", parser, 3);
    CASE_COMMAND("window:sendkey")
        result = delegateCommand("debug", parser, 2);
    CASE_COMMAND("window:screenshot")
        result = delegateCommand("debug", parser, 2);
    } else {
        // pass the command to the layer below (main widget)
        Kwave::CommandHandler *layer_below = m_main_widget;
        result = (layer_below) ? layer_below->executeCommand(command) : -ENOSYS;
    }

    return result;
}

//***************************************************************************
void Kwave::FileContext::forwardZoomChanged(double zoom)
{
    m_last_zoom = zoom;
    emit sigZoomChanged(this, zoom);
}

//***************************************************************************
void Kwave::FileContext::statusBarMessage(const QString &msg, unsigned int ms)
{
    m_last_status_message_text = msg;
    m_last_status_message_ms   = ms;
    if (ms)
        m_last_status_message_timer.start();
    else
        m_last_status_message_timer.invalidate();

    if (isActive())
        emit sigStatusBarMessage(msg, ms);
}

//***************************************************************************
void Kwave::FileContext::updatePlaybackPos(sample_index_t offset)
{
    if (!m_plugin_manager) return;
    if (!m_main_widget) return;

    bool playing = m_signal_manager->playbackController().running();
    if (!playing) return;

    QString txt;
    double rate = m_plugin_manager->signalRate();
    if (rate > 0) {
        double ms = static_cast<double>(offset) * 1E3 / rate;
        txt = i18n("Playback: %1", Kwave::ms2string(ms));
    } else {
        txt = i18n("Playback: %1 samples", Kwave::samples2string(offset));
    }

    statusBarMessage(txt, 2000);

    // make sure that the current playback position is visible
    m_last_playback_pos = offset;
    Kwave::Zoomable *z = zoomable();
    if (z) z->scrollTo(offset);
}

//***************************************************************************
void Kwave::FileContext::metaDataChanged(Kwave::MetaDataList meta_data)
{
    // find out the instance ID
    if (m_instance_nr == -1) {
        // build a list of all currently open files/instances (including this)
        QList<Kwave::App::FileAndInstance> files = m_application.openFiles();

        // filter out all instances of our file name
        QString our_name = signalName();
        QList<int> existing_instances;
        foreach (const Kwave::App::FileAndInstance &it, files) {
            const QString &name = it.first;
            int            inst = it.second;
            if (name == our_name) existing_instances.append(inst);
        }

        // remove our own entry
        if (existing_instances.contains(m_instance_nr))
            existing_instances.removeOne(m_instance_nr);

        // find an empty slot
        if (!existing_instances.isEmpty())
            while (existing_instances.contains(m_instance_nr))
                m_instance_nr = (m_instance_nr != -1) ? (m_instance_nr + 1) : 2;
    }

    if (isActive()) {
        // we are active -> emit the meta data immediately
        emit sigMetaDataChanged(meta_data);
    } // else: we are inactive -> emit the meta data later, when activated

    // update the caption of the sub window
    if (m_main_widget && (m_application.guiType() != Kwave::App::GUI_SDI))
        m_main_widget->setWindowTitle(windowCaption(true));
}

//***************************************************************************
void Kwave::FileContext::selectionChanged(sample_index_t offset,
                                          sample_index_t length)
{
    if (isActive()) {
        // we are active -> emit the selection change immediately
        emit sigSelectionChanged(offset, length);
    } // else: we are inactive -> not of interest / ignore
}

//***************************************************************************
void Kwave::FileContext::setUndoRedoInfo(const QString &undo,
                                         const QString &redo)
{
    m_last_undo = undo;
    m_last_redo = redo;

    if (isActive()) {
        // we are active -> emit the undo/redo info immediately
        emit sigUndoRedoInfo(undo, redo);
    } // else: we are inactive -> emit the undo/redo info later, when activated
}

//***************************************************************************
void Kwave::FileContext::visibleRangeChanged(sample_index_t offset,
                                             sample_index_t visible,
                                             sample_index_t total)
{
    if (isActive()) {
        // we are active -> emit the view info immediately
        emit sigVisibleRangeChanged(offset, visible, total);
    } // else: we are inactive -> emit the view info later, when activated
}

//***************************************************************************
void Kwave::FileContext::modifiedChanged()
{
    if (isActive()) {
        // we are active -> emit the modified state immediately
        emit sigModified();
    } // else: we are inactive -> emit the modified state later, when activated

    // update the caption of our main widget
    if (m_main_widget && (m_application.guiType() != Kwave::App::GUI_SDI))
        m_main_widget->setWindowTitle(windowCaption(true));
}

//***************************************************************************
void Kwave::FileContext::contextSwitched(Kwave::FileContext *context)
{
    Kwave::FileContext::UsageGuard _keep(this);

    if (context == this) {
        if (!m_active) {
            m_active = true;
            activated();
        }
    } else
        m_active = false;
}

//***************************************************************************
void Kwave::FileContext::activated()
{
    // let our plugin manager be the active one
    if (m_plugin_manager) m_plugin_manager->setActive();

    // emit last playback position if playback is running
    if (m_signal_manager && m_signal_manager->playbackController().running())
        updatePlaybackPos(m_last_playback_pos);

    // emit last zoom factor
    forwardZoomChanged(m_last_zoom);

    // erase the status message of the previous context
    emit sigStatusBarMessage(QString(), 0);

    // emit our last status bar message if it has not expired
    if (m_last_status_message_timer.isValid()) {
        quint64 elapsed = m_last_status_message_timer.elapsed();
        if (elapsed < m_last_status_message_ms) {
            unsigned int remaining =
                Kwave::toUint(m_last_status_message_ms - elapsed);
            emit sigStatusBarMessage(m_last_status_message_text, remaining);
        } else
            m_last_status_message_timer.invalidate();
    } else if (m_last_status_message_ms == 0) {
        // static message without expiration
        if (m_last_status_message_text.length()) {
            emit sigStatusBarMessage(m_last_status_message_text, 0);
        }
    }

    // emit latest meta data
    if (m_signal_manager)
        emit sigMetaDataChanged(m_signal_manager->metaData());

    // emit latest view range change
    if (m_main_widget && m_signal_manager) {
        sample_index_t offset  = m_main_widget->visibleOffset();
        sample_index_t visible = m_main_widget->visibleSamples();
        sample_index_t total   = m_signal_manager->length();
        emit sigVisibleRangeChanged(offset, visible, total);
    }

    // force update of the "modified" state
    emit sigModified();

    // emit last undo/redo info
    emit sigUndoRedoInfo(m_last_undo, m_last_redo);

}

//***************************************************************************
int Kwave::FileContext::parseCommands(QTextStream &stream)
{
    Kwave::FileContext::UsageGuard _keep(this);

    int result = 0;
    QMap<QString, label_t> labels;

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString label; // label, jump target of a "GOTO"
    while (!stream.atEnd() && !result) {
        QString line = stream.readLine().simplified();
        if (line.startsWith(_("#"))) continue; // skip comments
        if (!line.length()) continue;          // skip empty lines

        if (line.endsWith(QLatin1Char(':'))) {
            // this line seems to be a "label"
            QString name = line.left(line.length() - 1).simplified();
            if (!labels.contains(name)) {
                // qDebug("new label '%s' at %llu", DBG(name), stream.pos());
                label_t label_pos;
                label_pos.pos  = stream.pos();
                label_pos.hits = 0;
                labels[name] = label_pos;
            }

            // special handling for a label at the end of the file
            if (label.length() && (label == name)) {
                // label found
                label = QString();
            }
            continue;
        }

        Kwave::Parser parser(line);

        // the "GOTO" command
        if ( !label.length() &&
            (line.split(QLatin1Char(' ')).at(0) == _("GOTO")) ) {
            label = line.split(QLatin1Char(' ')).at(1).simplified();
        }

        // jump to a label, scan/seek mode
        if (label.length()) {
            if (labels.contains(label)) {
                labels[label].hits++;
                qDebug(">>> GOTO '%s' @ offset %lld (pass #%u)", DBG(label),
                       labels[label].pos,
                       labels[label].hits
                    );
                stream.seek(labels[label].pos);

                // reset the label to search for
                label = QString();
            }
            // else: maybe the label will follow somewhere below,
            //       scan forward...
            continue;
        }

        // synchronize before the command
        if (m_plugin_manager)
            m_plugin_manager->sync();

        // the "msgbox" command (useful for debugging)
        if (false) {
        CASE_COMMAND("msgbox")
            QApplication::restoreOverrideCursor();
            result = (Kwave::MessageBox::questionYesNo(mainWidget(),
                parser.firstParam()) == KMessageBox::PrimaryAction) ? 0 : 1;
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            continue;
        }

        // prevent this command from being re-added to the macro recorder
        if (!line.startsWith(_("nomacro:"), Qt::CaseInsensitive))
            line.prepend(_("nomacro:"));

        // process the command in the current context
        // NOTE: this could theoretically also be a command that modifies
        //       or even deletes the current context!
        result = EAGAIN;
        const Kwave::FileContext *current_ctx = (m_top_widget) ?
            m_top_widget->currentContext() : nullptr;
        if (current_ctx && (current_ctx != this))
            result = m_top_widget->forwardCommand(line);

        // If the call returned with EAGAIN, then the context in duty is
        // different from this instance but not yet completely set up.
        // In that case this context is still responsible for executing the
        // current command.
        if (result == EAGAIN)
            result = executeCommand(line);

        if (result)
            qDebug(">>> '%s' - result=%d", DBG(line), result);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        // synchronize after the command
        if (m_plugin_manager)
            m_plugin_manager->sync();

        // special handling of the "quit" command
        if (parser.command() == _("quit")) {
            result = ECANCELED;
            break;
        }
    }

    if (label.length()) {
        // oops, if we get here then we have searched for a non-exising label
        qWarning("label '%s' not found", DBG(label));
        result = -ENOENT;
    }

    // remove hourglass
    QApplication::restoreOverrideCursor();

    return result;
}

//***************************************************************************
void Kwave::FileContext::enqueueCommand(unsigned int delay,
                                       const QString &command)
{
    use();

    m_delayed_command_queue.append(
        QPair<unsigned int, QString>(delay, command)
    );
    if (!m_delayed_command_timer.isActive())
        m_delayed_command_timer.start(delay);
}

//***************************************************************************
void Kwave::FileContext::processDelayedCommand()
{
    if (m_delayed_command_queue.isEmpty()) return;

    QPair<unsigned int, QString> current = m_delayed_command_queue.takeFirst();
    executeCommand(current.second);
    if (m_delayed_command_queue.isEmpty()) return;

    QPair<unsigned int, QString> next = m_delayed_command_queue.first();
    m_delayed_command_timer.start(next.first);

    release();
}

//***************************************************************************
bool Kwave::FileContext::isInUse() const
{
    if (m_use_count >= 2) return true;
    return (m_signal_manager && !m_signal_manager->isEmpty());
}

//***************************************************************************
QString Kwave::FileContext::signalName() const
{
    return (m_signal_manager) ? m_signal_manager->signalName() : QString();
}

//***************************************************************************
QString Kwave::FileContext::windowCaption(bool with_modified) const
{
    QString name = signalName();

    // shortcut if no file loaded
    if (!name.length()) return QString();

    // if not in SDI mode we have to take care of multiple instances on our
    // own and append a " <n>" manually !
    if (m_application.guiType() != Kwave::App::GUI_SDI)
        if (m_instance_nr != -1)
            name = i18nc(
                "for window title: "
                "%1 = Name of the file, "
                "%2 = Instance number when opened multiple times",
                "%1 <%2>", name, m_instance_nr);

    if (with_modified) {
        bool modified = (m_signal_manager) ?
            m_signal_manager->isModified() : false;
        if (modified)
            return i18nc("%1 = Path to modified file", "* %1 (modified)", name);
    }
        return name;
}

//***************************************************************************
int Kwave::FileContext::loadBatch(const QUrl &url)
{
    Kwave::FileContext::UsageGuard _keep(this);

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
int Kwave::FileContext::revert()
{
    Kwave::FileContext::UsageGuard _keep(this);

    QUrl url(signalName());
    if (!url.isValid() || !m_signal_manager) return -EINVAL;

    if (m_signal_manager->isModified()) {
        int res =  Kwave::MessageBox::warningContinueCancel(m_top_widget,
            i18n("This file has been modified, changes will be lost.\n"
                 "Do you want to continue?"));
        if (res == KMessageBox::Cancel) return ECANCELED;
    }

    return m_signal_manager->loadFile(url);
}

//***************************************************************************
int Kwave::FileContext::saveFile()
{
    if (!m_signal_manager) return -EINVAL;

    int res = 0;

    if (signalName() != NEW_FILENAME) {
        QUrl url;
        url = QUrl(signalName());
        res = m_signal_manager->save(url, false);

        // if saving in current format is not possible (no encoder),
        // then try to "save/as" instead...
        if (res == -EINVAL) res = saveFileAs(QString(), false);
    } else res = saveFileAs(QString(), false);

    return res;
}

//***************************************************************************
int Kwave::FileContext::saveFileAs(const QString &filename, bool selection)
{
    if (!m_signal_manager) return -EINVAL;

    QString name = filename;
    QUrl url;
    int res = 0;

    if (name.length()) {
        /* name given -> take it */
        url = QUrl(name);
    } else {
        /*
         * no name given -> show the File/SaveAs dialog...
         */
        QUrl current_url;
        current_url = QUrl(signalName());

        QString what  = Kwave::CodecManager::mimeTypeOf(current_url);
        Kwave::Encoder *encoder = Kwave::CodecManager::encoder(what);
        QString extension; // = "*.wav";
        if (!encoder) {
            // no extension selected yet, use mime type from file info
            QString mime_type = Kwave::FileInfo(
                m_signal_manager->metaData()).get(
                    Kwave::INF_MIMETYPE).toString();
            encoder = Kwave::CodecManager::encoder(mime_type);
            if (encoder) {
                QStringList extensions = encoder->extensions(mime_type);
                if (!extensions.isEmpty()) {
                    QString ext = extensions.first().split(_(" ")).first();
                    if (ext.length()) {
                        extension = ext;
                        QString new_filename = current_url.fileName();
                        new_filename += extension.mid(1); // remove the "*"
                        current_url = current_url.adjusted(
                            QUrl::RemoveFilename);
                        current_url.setPath(current_url.path() +
                            new_filename);
                    }
                }
            }
        }

        QString filter = Kwave::CodecManager::encodingFilter();
        QPointer<Kwave::FileDialog> dlg = new(std::nothrow)Kwave::FileDialog(
            _("kfiledialog:///kwave_save_as"),
            Kwave::FileDialog::SaveFile,
            filter, m_top_widget, current_url, extension
        );
        if (!dlg) return 0;
        dlg->setWindowTitle(i18n("Save As"));
        if (dlg->exec() != QDialog::Accepted) {
            delete dlg;
            return -1;
        }

        url = dlg->selectedUrl();
        if (url.isEmpty()) {
            delete dlg;
            return 0;
        }

        QString new_name = url.path();
        QFileInfo path(new_name);

        // add the correct extension if necessary
        if (!path.suffix().length()) {
            QString ext = dlg->selectedExtension();
            QStringList extensions = ext.split(_(" "));
            ext = extensions.first();
            new_name += ext.mid(1);
            path.setFile(new_name);
            url.setPath(new_name);
        }

        delete dlg;
    }

    name = url.path();

    // NOTE: When we get here from the context of a script we already
    //       have a file name and must check overwrites on our own.
    //       If we used the standard file dialog above, this check has
    //       already been done there!
    if (filename.length()) {
        // check if the file exists and ask before overwriting it
        // if it is not the old filename
        if ((url.toDisplayString() != QUrl(signalName()).toDisplayString()) &&
            QFileInfo::exists(name))
        {
            if (Kwave::MessageBox::warningYesNo(m_top_widget,
                i18n("The file '%1' already exists.\n"
                    "Do you really want to overwrite it?", name)) !=
                    KMessageBox::PrimaryAction)
            {
                return -1;
            }
        }
    }

    // maybe we now have a new mime type
    QString previous_mimetype_name =
        Kwave::FileInfo(m_signal_manager->metaData()).get(
            Kwave::INF_MIMETYPE).toString();

    QString new_mimetype_name = Kwave::CodecManager::mimeTypeOf(url);

    if (new_mimetype_name != previous_mimetype_name) {
        // saving to a different mime type
        // now we have to do as if the mime type and file name
        // has already been selected to satisfy the fileinfo
        // plugin
        qDebug("TopWidget::saveAs(%s) - [%s] (previous:'%s')",
            DBG(url.toDisplayString()), DBG(new_mimetype_name),
            DBG(previous_mimetype_name) );

        // set the new mimetype
        Kwave::FileInfo info(m_signal_manager->metaData());
        info.set(Kwave::INF_MIMETYPE, new_mimetype_name);

        // set the new filename
        info.set(Kwave::INF_FILENAME, url.toDisplayString());
        m_signal_manager->setFileInfo(info, false);

        // now call the fileinfo plugin with the new filename and
        // mimetype
        res = (m_plugin_manager) ?
               m_plugin_manager->setupPlugin(_("fileinfo"), QStringList())
               : -1;

        // restore the mime type and the filename
        info = Kwave::FileInfo(m_signal_manager->metaData());
        info.set(Kwave::INF_MIMETYPE, previous_mimetype_name);
        info.set(Kwave::INF_FILENAME, url.toDisplayString());
        m_signal_manager->setFileInfo(info, false);
    }

    // now we have a file name -> do the "save" operation
    if (!res) res = m_signal_manager->save(url, selection);

    // if saving was successful, add the file to the list of recent files
    if (!res) m_application.addRecentFile(signalName());

    return res;
}

//***************************************************************************
bool Kwave::FileContext::closeFile()
{
    Kwave::FileContext::UsageGuard _keep(this);

    if (m_plugin_manager && !m_plugin_manager->canClose())
    {
        qWarning("FileContext::closeFile() - currently not possible, "\
                 "a plugin is running :-(");
        return false;
    }

    if (m_signal_manager && m_signal_manager->isModified()) {
        int res =  Kwave::MessageBox::warningYesNoCancel(m_top_widget,
            i18n("This file has been modified.\nDo you want to save it?"));
        if (res == KMessageBox::Cancel) return false;
        if (res == KMessageBox::PrimaryAction) {
            // user decided to save
            res = saveFile();
            qDebug("FileContext::closeFile()::saveFile, res=%d",res);
            if (res) return false;
        }
    }

    // close all plugins that still might use the current signal
    if (m_plugin_manager) {
        m_plugin_manager->stopAllPlugins();
        m_plugin_manager->signalClosed();
    }

    if (m_signal_manager) m_signal_manager->close();

    switch (m_application.guiType()) {
        case Kwave::App::GUI_MDI: /* FALLTHROUGH */
        case Kwave::App::GUI_TAB:
            // close the main widget
            delete m_main_widget;
            m_main_widget = nullptr;
            break;
        case Kwave::App::GUI_SDI:
            break;
        DEFAULT_IMPOSSIBLE;
    }

    return true;
}

//***************************************************************************
//***************************************************************************

#include "moc_FileContext.cpp"
