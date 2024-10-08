/***************************************************************************
                App.cpp  -  The Kwave main application
                             -------------------
    begin                : Wed Feb 28 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <QCommandLineParser>
#include <QFile>
#include <QMetaType>
#include <QMutableListIterator>
#include <QString>

#include <KConfig>
#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KSharedConfig>

#include "libkwave/ClipBoard.h"
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "App.h"
#include "FileContext.h"
#include "Splash.h"
#include "TopWidget.h"

using namespace Qt::StringLiterals;

/** maximum number of recent files */
#define MAX_RECENT_FILES 20

//***************************************************************************
Kwave::App::App(int &argc, char **argv)
   :QApplication(argc, argv),
    m_cmdline(nullptr),
    m_recent_files(),
    m_top_widgets(),
    m_gui_type(Kwave::App::GUI_TAB)
{
    qRegisterMetaType<Kwave::SampleArray>("Kwave::SampleArray");
    qRegisterMetaType<Kwave::LabelList>("Kwave::LabelList");
    qRegisterMetaType<sample_index_t>("sample_index_t");
    qRegisterMetaType<Kwave::MetaDataList>("Kwave::MetaDataList");

    // connect the clipboard
    connect(QApplication::clipboard(),
            SIGNAL(changed(QClipboard::Mode)),
            &(Kwave::ClipBoard::instance()),
            SLOT(slotChanged(QClipboard::Mode)));
}

//***************************************************************************
void Kwave::App::processCmdline(QCommandLineParser *cmdline)
{
    Q_ASSERT(cmdline);
    if (!cmdline) return;

    m_cmdline = cmdline;
    m_cmdline->parse(arguments());

    // read the configured user interface type
    QString result;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(u"Global"_s);
    result = cfg.readEntry("UI Type");
    if (result == _("SDI")) {
        m_gui_type = Kwave::App::GUI_SDI;
    } else if (result == _("MDI")) {
        m_gui_type = Kwave::App::GUI_MDI;
    } else if (result == _("TAB")) {
        m_gui_type = Kwave::App::GUI_TAB;
    }
    // else: use default

    // if user interface type is given as cmdline parameter: use that one
    if (m_cmdline->isSet(_("gui"))) {
        QString arg = m_cmdline->value(_("gui")).toUpper();
        bool valid = false;
        if (arg == _("SDI")) {
            m_gui_type = Kwave::App::GUI_SDI;
            valid = true;
        } else if (arg == _("MDI")) {
            m_gui_type = Kwave::App::GUI_MDI;
            valid = true;
        } else if (arg == _("TAB")) {
            m_gui_type = Kwave::App::GUI_TAB;
            valid = true;
        }
        // else: use previous setting

        // save this setting
        if (valid && (arg != result))
            cfg.writeEntry(_("UI Type"), arg);
    }
}

//***************************************************************************
Kwave::App::~App()
{
    saveRecentFiles();
    m_recent_files.clear();

    // let remaining cleanup handlers run (deferred delete)
    processEvents(QEventLoop::ExcludeUserInputEvents);
}

//***************************************************************************
int Kwave::App::newInstance(const QStringList &args, const QString &dir)
{
    int retval = 0;
    Q_UNUSED(dir)

    Q_ASSERT(m_cmdline);
    if (!m_cmdline) return -EINVAL;

    m_cmdline->parse(args);

    static bool first_time = true;
    if (first_time) {
        first_time = false;

        // open the log file if given on the command line
        if (m_cmdline->isSet(_("logfile"))) {
            if (!Kwave::Logger::open(m_cmdline->value(_("logfile"))))
                exit(-1);
        }

        Kwave::Splash::showMessage(i18n("Reading configuration..."));
        readConfig();

        // close when the last window closed
        connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
    }

    QStringList params = m_cmdline->positionalArguments();

    // only one parameter -> open with empty window
    if (params.isEmpty()) {
        retval = newWindow(QUrl());
    } else {
        // open a window for each file specified in the
        // command line an load it
        foreach (const QString &name, params) {
            retval = newWindow(Kwave::URLfromUserInput(name));
        }
    }
    return retval;
}

//***************************************************************************
bool Kwave::App::isOK() const
{
    return (!m_top_widgets.isEmpty());
}

//***************************************************************************
int Kwave::App::executeCommand(const QString &command)
{
    Kwave::Parser parser(command);
    if (parser.command() == _("newwindow")) {
        int retval = 0;
        if (parser.hasParams()) {
            retval = newWindow(QUrl(parser.params().at(0)));
        } else {
            retval = newWindow(QUrl(QString()));
        }
        return (retval == 0) ? 0 : -EIO;
    } else if (parser.command() == _("openrecent:clear")) {
        m_recent_files.clear();
        saveRecentFiles();
        emit recentFilesChanged();
    } else if (parser.command() == _("help")) {
        KHelpClient::invokeHelp();
    } else {
        return ENOSYS; // command not implemented (here)
    }
    return 0;
}

//***************************************************************************
void Kwave::App::addRecentFile(const QString &newfile)
{
    if (!newfile.length()) return;

    // remove old entries if present
    m_recent_files.removeAll(newfile);

    // shorten the list down to (MAX_RECENT_FILES - 1) entries
    while (m_recent_files.count() >= MAX_RECENT_FILES)
        m_recent_files.removeLast();

    // insert the new entry at top
    m_recent_files.prepend(newfile);

    // save the list of recent files
    saveRecentFiles();

    // update all toplevel widgets
    emit recentFilesChanged();
}

//***************************************************************************
int Kwave::App::newWindow(const QUrl &url)
{
    int retval = 0;
    Kwave::TopWidget *new_top_widget = nullptr;

    Kwave::Splash::showMessage(i18n("Opening main window..."));

    switch (m_gui_type) {
        case Kwave::App::GUI_TAB: /* FALLTHROUGH */
        case Kwave::App::GUI_MDI:
            // re-use the last top widget to open the file
            if (!m_top_widgets.isEmpty())
                new_top_widget = m_top_widgets.last();
            break;
        case Kwave::App::GUI_SDI:
            // always create a new top widget, except when handling commands
            if ( (url.scheme().toLower() == Kwave::urlScheme()) &&
                 !m_top_widgets.isEmpty() )
                new_top_widget = m_top_widgets.last();
            break;
        DEFAULT_IMPOSSIBLE;
    }

    if (!new_top_widget) {
        new_top_widget = new(std::nothrow) Kwave::TopWidget(*this);
        if (!new_top_widget || !new_top_widget->init()) {
            // init failed
            qWarning("ERROR: initialization of TopWidget failed");
            delete new_top_widget;
            return ECANCELED;
        }

        if (!m_top_widgets.isEmpty()) {
            // create a new widget with the same geometry as
            // the last created one
            const QRect &geom = m_top_widgets.last()->geometry();
            // calling setGeometry(geom) would overlap :-(
            new_top_widget->resize(geom.width(), geom.height());
        }

        m_top_widgets.append(new_top_widget);
        new_top_widget->show();

        // inform the widget about changes in the list of recent files
        connect(this, SIGNAL(recentFilesChanged()),
                new_top_widget, SLOT(updateRecentFiles()));
    }

    retval = (!url.isEmpty()) ? new_top_widget->loadFile(url) : 0;
    if (retval == ECANCELED)
        delete new_top_widget;

    Kwave::Splash::showMessage(i18n("Startup done"));
    return retval;
}

//***************************************************************************
bool Kwave::App::toplevelWindowHasClosed(Kwave::TopWidget *todel)
{
    // save the list of recent files
    saveRecentFiles();

    // remove the toplevel widget from our list
    if (m_top_widgets.contains(todel))
        m_top_widgets.removeAll(todel);

    // if list is empty -> no more windows there -> exit application
    return (m_top_widgets.isEmpty());
}

//***************************************************************************
QList<Kwave::App::FileAndInstance> Kwave::App::openFiles() const
{
    QList<Kwave::App::FileAndInstance> all_files;
    foreach (const Kwave::TopWidget *topwidget, m_top_widgets) {
        if (!topwidget) continue;
        QList<Kwave::App::FileAndInstance> files = topwidget->openFiles();
        if (!files.isEmpty())
            all_files += files;
    }
    return all_files;
}

//***************************************************************************
void Kwave::App::switchGuiType(Kwave::TopWidget *top, GuiType new_type)
{
    Q_ASSERT(top);
    if (!top) return;
    if (new_type == m_gui_type) return;

    // collect all contexts of all toplevel widgets, and delete all except
    // the new top widget that is calling us
    QList<Kwave::FileContext *> all_contexts;
    QMutableListIterator<Kwave::TopWidget *> it(m_top_widgets);
    while (it.hasNext()) {
        Kwave::TopWidget *topwidget = it.next();
        if (!topwidget) { it.remove(); continue; }
        QList<Kwave::FileContext *> contexts = topwidget->detachAllContexts();
        if (!contexts.isEmpty()) all_contexts += contexts;
        if (topwidget != top) {
            it.remove();
            delete topwidget;
        }
    }

    // from now on use the new GUI type
    m_gui_type = new_type;

    // at this point we should have exactly one toplevel widget without
    // context and a list of contexts (which may be empty)
    if (!all_contexts.isEmpty()) {
        bool first = true;
        foreach (Kwave::FileContext *context, all_contexts) {
            Kwave::TopWidget *top_widget = nullptr;

            switch (m_gui_type) {
                case GUI_SDI:
                    if (!context->isEmpty() || (all_contexts.count() == 1)) {
                        // either the context contains some signal and is worth
                        // assigning it to a toplevel widget, or it is the one
                        // and only context
                        if (first) {
                            // the context reuses the calling toplevel widget
                            top_widget = top;
                            first = false;
                        } else {
                            // for all other contexts we have to create a new
                            // toplevel widget
                            top_widget = new(std::nothrow)
                                Kwave::TopWidget(*this);
                            if (!top_widget || !top_widget->init()) {
                                // init failed
                                qWarning("ERROR: initialization of "
                                         "TopWidget failed");
                                delete top_widget;
                                delete context;
                                break;
                            }
                            m_top_widgets.append(top_widget);
                            top_widget->show();

                            // inform the widget about changes in the list of
                            // recent files
                            connect(this, SIGNAL(recentFilesChanged()),
                                    top_widget, SLOT(updateRecentFiles()));
                        }
                    } else {
                        // probably this context is only executing a script and
                        // has (no longer) any valid signal. We need to assign
                        // it to a toplevel widget for processing further
                        // commands but reduce the reference count to make it
                        // vanish as soon as the script terminates.
                        context->setParent(top);
                        context->release();
                    }
                    break;
                case GUI_MDI: /* FALLTHROUGH */
                case GUI_TAB:
                    // all contexts go into the same toplevel widget
                    top_widget = top;
                    break;
                DEFAULT_IMPOSSIBLE;
            }

            if (top_widget) top_widget->insertContext(context);
        }
    } else {
        // give our one and only toplevel widget a default context
        top->insertContext(nullptr);
    }
}

//***************************************************************************
void Kwave::App::saveRecentFiles()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(u"Recent Files"_s);

    QString num;
    for (int i = 0 ; i < MAX_RECENT_FILES; i++) {
        num.setNum(i);
        if (i < m_recent_files.count())
            cfg.writeEntry(num, m_recent_files[i]);
        else
            cfg.deleteEntry(num);
    }

    cfg.sync();
}

//***************************************************************************
void Kwave::App::readConfig()
{
    const KConfigGroup cfg = KSharedConfig::openConfig()->group(u"Recent Files"_s);

    for (unsigned int i = 0 ; i < MAX_RECENT_FILES; i++) {
        QString key = QString::number(i);        // generate number

        // read corresponding entry, which is stored in UTF-8
        QString result = cfg.readEntry(key);
        if (result.length()) {
            QFile file(result);

            //check if file exists and insert it if not already present
            if (file.exists() && (m_recent_files.contains(result) == 0))
                m_recent_files.append(result);
        }
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_App.cpp"
