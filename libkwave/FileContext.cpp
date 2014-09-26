/***************************************************************************
    libkwave/FileContext.cpp  -  Context of a Loaded File
			     -------------------
    begin                : 2009-12-31
    copyright            : (C) 2009 by Thomas.Eschenbacher
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

#include "libkwave/FileContext.h"
#include "libkwave/Logger.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"

#include "kwave/MainWidget.h"
#include "kwave/TopWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == _(x)) {

//***************************************************************************
Kwave::FileContext::FileContext(Kwave::App &app)
    :QObject(),
     m_application(app),
     m_top_widget(0),
     m_main_widget(0),
     m_signal_manager(0),
     m_plugin_manager(0)
{
}

//***************************************************************************
Kwave::FileContext::~FileContext()
{
    m_top_widget     = 0;
    m_main_widget    = 0;
    m_signal_manager = 0;
    m_plugin_manager = 0;
}

//***************************************************************************
Kwave::App &Kwave::FileContext::application() const
{
    return m_application;
}

//***************************************************************************
Kwave::TopWidget *Kwave::FileContext::topWidget() const
{
    Q_ASSERT(m_top_widget);
    return m_top_widget;
}

//***************************************************************************
Kwave::MainWidget *Kwave::FileContext::mainWidget() const
{
    Q_ASSERT(m_main_widget);
    return m_main_widget;
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
int Kwave::FileContext::executeCommand(const QString &line)
{
    int result = 0;
    Q_UNUSED(line);
    bool use_recorder = true;
    QString command = line;

    Q_ASSERT(m_plugin_manager);
    Q_ASSERT(m_top_widget);
    if (!m_plugin_manager || !m_top_widget) return -ENOMEM;

//     qDebug("FileContext::executeCommand(%s)", DBG(command));
    if (!command.length()) return 0; // empty line -> nothing to do
    if (command.trimmed().startsWith(_("#")))
	return 0; // only a comment

    // log all commands to the log file if enabled
    Kwave::Logger::log(this, Kwave::Logger::Info, _("CMD: ") + line);

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

    // parse one single command
    Kwave::Parser parser(command);

    // exclude menu commands from the recorder
    if (parser.command() == _("menu")) use_recorder = false;

    // only record plugin:execute, not plugin without parameters
    if (parser.command() == _("plugin")) use_recorder = false;

    // let through all commands that handle zoom/view or playback like fwd/rew
    bool allow_always =
        (parser.command() == _("playback")) ||
	parser.command().startsWith(_("view:")) ||
	parser.command().startsWith(_("playback:")) ||
	parser.command().startsWith(_("select_track:")) ||
	(parser.command() == _("close")) ||
	(parser.command() == _("quit"))
	;

    // all others only if no plugin is currently running
    if (!allow_always && m_plugin_manager->onePluginRunning())
    {
	qWarning("FileContext::executeCommand('%s') - currently not possible, "
		 "a plugin is running :-(",
		 DBG(parser.command()));
	return -1;
    }

    if (use_recorder) {
	// append the command to the macro recorder
	// @TODO macro recording...
	qDebug("# %s ", DBG(command));
    }

    if ((result = m_top_widget->executeCommand(command)) != ENOSYS) {
	return result;
    CASE_COMMAND("plugin")
	QString name = parser.firstParam();
	QStringList params;
	int cnt = Kwave::toInt(parser.count());
	while (--cnt > 0) params.append(parser.nextParam());
	qDebug("FileContext::executeCommand(): loading plugin '%s'", DBG(name));
	qDebug("FileContext::executeCommand(): with %d parameter(s)",
		params.count());
	result = m_plugin_manager->executePlugin(
		    name, params.count() ? &params : 0);
    CASE_COMMAND("plugin:execute")
	QStringList params;
	int cnt = Kwave::toInt(parser.count());
	QString name(parser.firstParam());
	while (--cnt > 0) params.append(parser.nextParam());
	result = m_plugin_manager->executePlugin(name, &params);
    CASE_COMMAND("plugin:setup")
	QString name(parser.firstParam());
	result = m_plugin_manager->setupPlugin(name);
    } else {
	// pass the command to the layer below (main widget)
	Kwave::CommandHandler *layer_below = m_main_widget;
	Q_ASSERT(layer_below);
	result = (layer_below) ? layer_below->executeCommand(command) : -1;
    }

    return result;
}

//***************************************************************************
void Kwave::FileContext::forwardZoomChanged(double zoom)
{
    emit sigZoomChanged(this, zoom);
}

//***************************************************************************
#include "FileContext.moc"
//***************************************************************************
//***************************************************************************
