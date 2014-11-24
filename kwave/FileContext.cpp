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

#include <QtGui/QApplication>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <kglobal.h>
#include <kstandarddirs.h>

#include "libkwave/CodecManager.h"
#include "libkwave/Encoder.h"
#include "libkwave/FileContext.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/Parser.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"

#include "App.h"
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
     m_application(app),
     m_top_widget(0),
     m_main_widget(0),
     m_signal_manager(0),
     m_plugin_manager(0),
     m_active(true),
     m_last_zoom(0),
     m_last_playback_pos(0),
     m_last_status_message_text(),
     m_last_status_message_timer(),
     m_last_status_message_ms(0),
     m_last_meta_data(),
     m_last_selection_offset(SAMPLE_INDEX_MAX),
     m_last_selection_length(SAMPLE_INDEX_MAX),
     m_last_undo(QString()),
     m_last_redo(QString()),
     m_last_modified(false),
     m_last_visible_offset(SAMPLE_INDEX_MAX),
     m_last_visible_samples(SAMPLE_INDEX_MAX),
     m_last_visible_total(SAMPLE_INDEX_MAX)
{
}

//***************************************************************************
Kwave::FileContext::~FileContext()
{
    emit destroyed(this);

    if (m_main_widget) delete m_main_widget;
    m_main_widget = 0;

    m_application.closeWindow(m_top_widget);
    m_top_widget = 0;

    if (m_plugin_manager) delete m_plugin_manager;
    m_plugin_manager = 0;

    if (m_signal_manager) delete m_signal_manager;
    m_signal_manager = 0;
}

//***************************************************************************
bool Kwave::FileContext::createMainWidget()
{
    // create the main widget
    m_main_widget = new(std::nothrow) Kwave::MainWidget(m_top_widget, *this);
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return false;
    if (!(m_main_widget->isOK())) {
	delete m_main_widget;
	m_main_widget = 0;
	return false;
    }

    // connect the main widget
    connect(&(m_signal_manager->playbackController()),
            SIGNAL(sigSeekDone(sample_index_t)),
            m_main_widget, SLOT(scrollTo(sample_index_t)));
    connect(m_main_widget, SIGNAL(sigCommand(const QString &)),
            this,          SLOT(executeCommand(const QString &)));
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

    if (m_application.guiType() == Kwave::App::GUI_SDI) {
	// SDI mode: create a main widget right now
	if (!createMainWidget())
	    return false;
    }

    // connect the signal manager
    connect(m_signal_manager, SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            this,             SLOT(metaDataChanged(Kwave::MetaDataList)));
    connect(&(m_signal_manager->selection()),
            SIGNAL(changed(sample_index_t, sample_index_t)),
            this,
            SLOT(selectionChanged(sample_index_t, sample_index_t)));
    connect(m_signal_manager, SIGNAL(sigUndoRedoInfo(const QString&,
                                                     const QString&)),
            this, SLOT(setUndoRedoInfo(const QString&, const QString&)));
    connect(m_signal_manager, SIGNAL(sigModified(bool)),
            this,             SLOT(modifiedChanged(bool)));

    // connect the plugin manager
    connect(m_plugin_manager, SIGNAL(sigProgress(const QString &)),
            m_top_widget,     SLOT(showInSplashSreen(const QString &)));
    connect(m_plugin_manager, SIGNAL(sigCommand(const QString &)),
            this,             SLOT(executeCommand(const QString &)));

    // connect the playback controller
    connect(&(m_signal_manager->playbackController()),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this, SLOT(updatePlaybackPos(sample_index_t)));

    connect(top_widget, SIGNAL(sigFileContextSwitched(Kwave::FileContext *)),
            this,       SLOT(contextSwitched(Kwave::FileContext *)));

    Kwave::Splash::showMessage(i18n("Scanning plugins..."));
    m_plugin_manager->searchPluginModules();

    // load the menu from file
    QFile menufile(KStandardDirs::locate("data", _("kwave/menus.config")));
    menufile.open(QIODevice::ReadOnly);
    QTextStream stream(&menufile);
    Q_ASSERT(!stream.atEnd());
    if (!stream.atEnd()) parseCommands(stream);
    menufile.close();

    // now we are initialized, load all plugins
    Kwave::Splash::showMessage(i18n("Loading plugins..."));
    statusBarMessage(i18n("Loading plugins..."), 0);
    m_plugin_manager->loadAllPlugins();
    statusBarMessage(i18n("Ready"), 1000);

    return true;
}

//***************************************************************************
QWidget *Kwave::FileContext::topWidget() const
{
    Q_ASSERT(m_top_widget);
    return m_top_widget;
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
    CASE_COMMAND("close")
	result = closeFile() ? 0 : 1;
    CASE_COMMAND("loadbatch")
	result = loadBatch(parser.nextParam());
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
    CASE_COMMAND("revert")
	result = revert();
    CASE_COMMAND("save")
	result = saveFile();
    CASE_COMMAND("saveas")
	result = saveFileAs(parser.nextParam(), false);
    CASE_COMMAND("saveselect")
	result = saveFileAs(QString(), true);
    } else {
	// pass the command to the layer below (main widget)
	Kwave::CommandHandler *layer_below = m_main_widget;
	result = (layer_below) ? layer_below->executeCommand(command) : -1;
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
	txt = i18n("Playback: %1 samples", KGlobal::locale()->formatLong(
	           static_cast<long int>(offset)));
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
    if (isActive()) {
	// we are active -> emit the meta data immediately
	m_last_meta_data = Kwave::MetaDataList();
	emit sigMetaDataChanged(meta_data);
    } else {
	// we are inactive -> emit the meta data later, when activated
	m_last_meta_data = meta_data;
    }

    // update the caption of the sub window
    if (m_application.guiType() != Kwave::App::GUI_SDI) {
	QString caption = windowCaption();
	if (m_main_widget) m_main_widget->setWindowTitle(caption);
    }
}

//***************************************************************************
void Kwave::FileContext::selectionChanged(sample_index_t offset,
                                          sample_index_t length)
{
    m_last_selection_offset = offset;
    m_last_selection_length = length;

    if (isActive()) {
	// we are active -> emit the selection change immediately
	emit sigSelectionChanged(offset, length);
    } else {
	// we are inactive -> emit the selection change later, when activated
    }
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
    } else {
	// we are inactive -> emit the undo/redo info later, when activated
    }
}

//***************************************************************************
void Kwave::FileContext::visibleRangeChanged(sample_index_t offset,
                                             sample_index_t visible,
                                             sample_index_t total)
{
    if (isActive()) {
	// we are active -> emit the view info immediately
	emit sigVisibleRangeChanged(offset, visible, total);

	m_last_visible_offset  = SAMPLE_INDEX_MAX;
	m_last_visible_samples = SAMPLE_INDEX_MAX;
	m_last_visible_total   = SAMPLE_INDEX_MAX;
    } else {
	// we are inactive -> emit the view info later, when activated
	m_last_visible_offset  = offset;
	m_last_visible_samples = visible;
	m_last_visible_total   = total;
    }
}

//***************************************************************************
void Kwave::FileContext::modifiedChanged(bool modified)
{
    m_last_modified = modified;

    if (isActive()) {
	// we are active -> emit the modified state immediately
	emit sigModified(modified);
    } else {
	// we are inactive -> emit the modified state later, when activated
    }
}

//***************************************************************************
void Kwave::FileContext::contextSwitched(Kwave::FileContext *context)
{
    if (context == this) {
	if (!m_active) {
	    m_active = true;
	    activated();
	}
    } else {
	if (m_active) {
	    m_active = false;
	    deactivated();
	}
    }
}

//***************************************************************************
void Kwave::FileContext::activated()
{
    // emit last playback position if playback is running
    if (m_signal_manager && m_signal_manager->playbackController().running())
	updatePlaybackPos(m_last_playback_pos);

    // emit last zoom factor
    forwardZoomChanged(m_last_zoom);

    // emit last status bar message if it has not expired
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

    // emit last meta data change
    if (!m_last_meta_data.isEmpty()) {
	emit sigMetaDataChanged(m_last_meta_data);
	m_last_meta_data = Kwave::MetaDataList();
    }

    // emit last view range change (always, if available)
    if (!( (m_last_visible_offset  == SAMPLE_INDEX_MAX) &&
           (m_last_visible_samples == SAMPLE_INDEX_MAX) &&
           (m_last_visible_total   == SAMPLE_INDEX_MAX)))
    {
	emit sigVisibleRangeChanged(m_last_visible_offset,
	                            m_last_visible_samples,
	                            m_last_visible_total);
    }

    // emit last selection change (always, if available)
    if ( (m_last_selection_length != SAMPLE_INDEX_MAX) &&
         (m_last_selection_offset != SAMPLE_INDEX_MAX) )
    {
	emit sigSelectionChanged(m_last_selection_offset,
	                         m_last_selection_length);
    }

    // emit the last "modified" state (always)
    emit sigModified(m_last_modified);

}

//***************************************************************************
void Kwave::FileContext::deactivated()
{
}

//***************************************************************************
int Kwave::FileContext::parseCommands(QTextStream &stream)
{
    int result = 0;
    QMap<QString, label_t> labels;

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
	if (m_plugin_manager)
	    m_plugin_manager->sync();

	// the "msgbox" command (useful for debugging)
	if (parser.command() == _("msgbox")) {
	    QApplication::restoreOverrideCursor();
	    result = (Kwave::MessageBox::questionYesNo(mainWidget(),
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
	result = executeCommand(line);
	if (result)
	    qDebug(">>> '%s' - result=%d", DBG(line), result);

	// synchronize after the command
	if (m_plugin_manager)
	    m_plugin_manager->sync();

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
QString Kwave::FileContext::signalName() const
{
    return (m_signal_manager) ? m_signal_manager->signalName() : QString();
}

//***************************************************************************
QString Kwave::FileContext::windowCaption() const
{
    const QString name = signalName();

    // shortcut if no file loaded
    if (!name.length()) return QString();

    bool modified = (m_signal_manager) ? m_signal_manager->isModified() :false;
    if (modified)
	return i18nc("%1 = Path to modified file", "* %1 (modified)", name);
    else
	return name;
}

//***************************************************************************
int Kwave::FileContext::loadBatch(const KUrl &url)
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
int Kwave::FileContext::revert()
{
    KUrl url(signalName());
    if (!url.isValid()) return -EINVAL;

    return (m_signal_manager) ? m_signal_manager->loadFile(url) : -EINVAL;
}

//***************************************************************************
int Kwave::FileContext::saveFile()
{
    if (!m_signal_manager) return -EINVAL;

    int res = 0;

    if (signalName() != NEW_FILENAME) {
	KUrl url;
	url = signalName();
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
    KUrl url;
    int res = 0;

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
			current_url.setFileName(new_filename);
		    }
		}
	    }
	}

	QString filter = Kwave::CodecManager::encodingFilter();
	Kwave::FileDialog dlg(_("kfiledialog:///kwave_save_as"),
	    filter, m_top_widget, true, current_url.prettyUrl(), extension);
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
	if (Kwave::MessageBox::warningYesNo(m_top_widget,
	    i18n("The file '%1' already exists.\n"
	         "Do you really want to overwrite it?", name)) !=
	         KMessageBox::Yes)
	{
	    return -1;
	}
    }

    // maybe we now have a new mime type
    QString previous_mimetype_name =
	Kwave::FileInfo(m_signal_manager->metaData()).get(
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
	Kwave::FileInfo info(m_signal_manager->metaData());
	info.set(Kwave::INF_MIMETYPE, new_mimetype_name);

	// set the new filename
	info.set(Kwave::INF_FILENAME, url.prettyUrl());
	m_signal_manager->setFileInfo(info, false);

	// now call the fileinfo plugin with the new filename and
	// mimetype
	Q_ASSERT(m_context.pluginManager());
	res = (m_plugin_manager) ?
	       m_plugin_manager->setupPlugin(_("fileinfo"))
	       : -1;

	// restore the mime type and the filename
	info = Kwave::FileInfo(m_signal_manager->metaData());
	info.set(Kwave::INF_MIMETYPE, previous_mimetype_name);
	info.set(Kwave::INF_FILENAME, url.prettyUrl());
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
	if (res == KMessageBox::Yes) {
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
    return true;
}

//***************************************************************************
//***************************************************************************
