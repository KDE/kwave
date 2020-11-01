/*************************************************************************
        DebugPlugin.cpp  -  various debug aids
                             -------------------
    begin                : Mon Feb 02 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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
#include <math.h>
#include <string.h>
#include <new>

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QStringList>
#include <QWindow>

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPixmap>
#include <QtEvents>

#include <KLocalizedString> // for the i18n macro

#include "libkwave/Logger.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "DebugPlugin.h"

KWAVE_PLUGIN(debug, DebugPlugin)

/** size of the internal buffer */
#define BUFFER_SIZE (64 * 1024)

/** helper for generating menu entries */
#define MENU_ENTRY(cmd,txt) \
    emitCommand(entry.arg(_(cmd)).arg(txt));

//***************************************************************************
Kwave::DebugPlugin::DebugPlugin(QObject *parent,
                                const QVariantList &args)
    :Kwave::Plugin(parent, args), m_buffer()
{
}

//***************************************************************************
Kwave::DebugPlugin::~DebugPlugin()
{
}

//***************************************************************************
void Kwave::DebugPlugin::load(QStringList &params)
{
    Q_UNUSED(params)
#ifdef HAVE_DEBUG_PLUGIN
    QString entry = _("menu(plugin:execute(debug,%1),Calculate/Debug/%2)");

    MENU_ENTRY("dc_50",             _(I18N_NOOP("Generate 50% DC Level")))
    MENU_ENTRY("dc_100",            _(I18N_NOOP("Generate 100% DC Level")))
    MENU_ENTRY("min_max",           _(I18N_NOOP("MinMax Pattern")))
    MENU_ENTRY("sawtooth",          _(I18N_NOOP("Generate Sawtooth Pattern")))
    MENU_ENTRY("sawtooth_verify",   _(I18N_NOOP("Verify Sawtooth Pattern")))
    MENU_ENTRY("fm_sweep",          _(I18N_NOOP("FM Sweep")))
//  MENU_ENTRY("stripe_index",      _(I18N_NOOP("Stripe Index")))
//  MENU_ENTRY("hull_curve",        _(I18N_NOOP("Hull Curve")))
//  MENU_ENTRY("offset_in_stripe",  _(I18N_NOOP("Offset in Stripe")))
//  MENU_ENTRY("stripe_borders",    _(I18N_NOOP("Show Stripe Borders")))
    MENU_ENTRY("labels_at_stripes", _(I18N_NOOP("Labels at Stripe borders")))

    entry = _("menu(plugin:setup(debug,%1),Help/%2)");
    MENU_ENTRY("dump_windows",      _(I18N_NOOP("Dump Window Hierarchy")))

    entry = _("menu(%1,Help/%2)");
    MENU_ENTRY("dump_metadata()",   _(I18N_NOOP("Dump Meta Data")))
#endif /* HAVE_DEBUG_PLUGIN */
}

//***************************************************************************
QStringList *Kwave::DebugPlugin::setup(QStringList &params)
{
    if (params.count() < 1) return Q_NULLPTR;

    QString command = params.first();
    QString action = i18n("Debug (%1)", command);
    Kwave::UndoTransactionGuard undo_guard(*this, action);

//     for (int i = 0; i < params.count()d; ++i)
// 	qDebug("param[%d] = '%s'", i, DBG(params[i]));

    if (command == _("dump_windows")) {
	dump_children(parentWidget(), _(""));
    } else if (command == _("window:click")) {
        if (params.count() != 4) return Q_NULLPTR;
	QString    class_name = params[1];
	QWidget   *widget     = findWidget(class_name.toUtf8().constData());
	unsigned int x        = params[2].toUInt();
	unsigned int y        = params[3].toUInt();
        if (!widget) return Q_NULLPTR;

	QMouseEvent *press_event = new(std::nothrow)
	    QMouseEvent(QEvent::MouseButtonPress,
		QPoint(x, y),
		widget->mapToGlobal(QPoint(x,y)),
		Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
	    );
	QCoreApplication::postEvent(widget, press_event);

	QMouseEvent *release_event = new(std::nothrow)
	    QMouseEvent(QEvent::MouseButtonRelease,
		QPoint(x, y),
		widget->mapToGlobal(QPoint(x,y)),
		Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
	    );
	QCoreApplication::postEvent(widget, release_event);
    } else if (command == _("window:close")) {
        if (params.count() != 2) return Q_NULLPTR;
	QString    class_name = params[1];
	QWidget   *widget     = findWidget(class_name.toUtf8().constData());

	qDebug("close window '%s' [%p]",
	    DBG(class_name), static_cast<void *>(widget));
        if (!widget) return Q_NULLPTR;
	widget->close();
    } else if (command == _("window:mousemove")) {
        if (params.count() != 4) return Q_NULLPTR;
	QString    class_name = params[1];
	QWidget   *widget     = findWidget(class_name.toUtf8().constData());
	unsigned int x        = params[2].toUInt();
	unsigned int y        = params[3].toUInt();
        if (!widget) return Q_NULLPTR;

	QMouseEvent *move_event = new(std::nothrow)
	    QMouseEvent(QEvent::MouseMove,
		QPoint(x, y),
		widget->mapToGlobal(QPoint(x,y)),
		Qt::NoButton, Qt::NoButton, Qt::NoModifier
	    );
	QCoreApplication::postEvent(widget, move_event);
    } else if (command == _("window:resize")) {
        if (params.count() != 4) return Q_NULLPTR;
	QString    class_name = params[1];
	QWidget   *widget     = findWidget(class_name.toUtf8().constData());
	unsigned int width    = params[2].toUInt();
	unsigned int height   = params[3].toUInt();
        if (!widget) return Q_NULLPTR;
	widget->resize(width, height);
    } else if (command == _("window:screenshot")) {
        if (params.count() != 3) return Q_NULLPTR;
	screenshot(params[1].toUtf8(), params[2]);
    } else if (command == _("window:sendkey")) {
        if (params.count() != 3) return Q_NULLPTR;
	QString    class_name = params[1];
	QString    key_name   = params[2];
	QWidget   *widget     = findWidget(class_name.toUtf8().constData());

	unsigned int          shortcut = QKeySequence::fromString(key_name)[0];
	int                   key_code = shortcut & Qt::Key_unknown;
	Qt::KeyboardModifiers key_modifiers(shortcut & ~Qt::Key_unknown);

	qDebug("send key '%s' [0x%08X:0x%08X] to '%s' [%p]",
	       DBG(key_name), static_cast<int>(key_modifiers), key_code,
	       DBG(class_name), static_cast<void *>(widget));
        if (!widget) return Q_NULLPTR;

	// make sure that the widget gets the focus
	widget->activateWindow();
	widget->raise();
	widget->setFocus(Qt::OtherFocusReason);

	QKeyEvent *press_event = new(std::nothrow)
	    QKeyEvent(QEvent::KeyPress, key_code, key_modifiers);
	QCoreApplication::postEvent(widget, press_event);

	QKeyEvent *release_event = new(std::nothrow)
	    QKeyEvent(QEvent::KeyRelease, key_code, key_modifiers);
	QCoreApplication::postEvent(widget, release_event);

    }

    return new(std::nothrow) QStringList;
}

//***************************************************************************
void Kwave::DebugPlugin::run(QStringList params)
{
    sample_index_t first = 0;
    sample_index_t last  = 0;
    Kwave::SignalManager &sig    = signalManager();
    const double          rate   = signalRate();

    if (params.count() < 1) return;

    QString command = params.first();
    QString action = i18n("Debug (%1)", command);
    Kwave::UndoTransactionGuard undo_guard(*this, action);

    // get the buffer for faster processing
    {
	bool ok = m_buffer.resize(BUFFER_SIZE);
	Q_ASSERT(ok);
	if (!ok) return;
	m_buffer.fill(0);
    }

    bool make_new_track = (
	(command == _("stripe_index")) ||
	(command == _("offset_in_stripe")) ||
	(command == _("stripe_borders"))
    );

    if (command == _("min_max")) {
	// toggle between minimum and maximum possible sample value
	for (unsigned int i = 0; i < BUFFER_SIZE; i++)
	    m_buffer[i] = (i & 1) ? SAMPLE_MIN : SAMPLE_MAX;
    }

    if (command == _("labels_at_stripes")) {
	QList<Kwave::Stripe::List> all_stripes = sig.stripes(sig.allTracks());
	if (all_stripes.isEmpty()) return;

	const Kwave::Stripe::List &stripes = all_stripes.first();
	unsigned int index = 0;
	foreach (const Kwave::Stripe &stripe, stripes) {
	    QString text;
	    text = _("stripe #%1 [%2 .. %3]").
		arg(index++).
		arg(stripe.start()).
		arg(stripe.end());
	    sig.addLabel(stripe.start(), text);
	}
	return;
    } else if (command == _("dump_metadata")) {
	sig.metaData().dump();
	return;
    } else if (command == _("sawtooth_verify")) {
	Kwave::MultiTrackReader *readers = new(std::nothrow)
	    Kwave::MultiTrackReader(
		Kwave::SinglePassForward, sig, sig.selectedTracks(),
		first, last
	    );
	Q_ASSERT(readers);
	if (!readers) return;

	sample_index_t pos = first;
	bool ok = true;
	while (ok && (first <= last) && (!shouldStop())) {
	    sample_index_t rest = last - first + 1;
	    if (rest < m_buffer.size()) {
		ok = m_buffer.resize(Kwave::toUint(rest));
		Q_ASSERT(ok);
		if (!ok) break;
	    }

	    unsigned int count = readers->tracks();
	    for (unsigned int r = 0; ok && (r < count); ++r) {
		*((*readers)[r]) >> m_buffer;
		for (unsigned int ofs = 0; ofs < m_buffer.size(); ++ofs) {
		    sample_t value_is = m_buffer[ofs];
		    sample_t value_should = SAMPLE_MIN + static_cast<sample_t>(
			((pos + ofs) % (SAMPLE_MAX - SAMPLE_MIN)));
		    if (value_is != value_should) {
			qWarning("ERROR: mismatch detected at offset %llu: "
			         "value=%d, expected=%d", pos + ofs,
			         value_is, value_should);
			ok = false;
			break;
		    }
		}
		if (!ok) break;
	    }

	    pos   += m_buffer.size();
	    first += m_buffer.size();
	}
	delete readers;
	if (ok) {
	    qDebug("test pattern successfully detected, no errors :-)");
	}
	return;
    }

    Kwave::MultiTrackWriter *writers = Q_NULLPTR;

    if (make_new_track) {
	// append a new track
	sig.appendTrack();

	// and use only the new track as target
	last = signalLength() - 1;
	QVector<unsigned int> track_list;
	track_list.append(sig.tracks() - 1);
	writers = new(std::nothrow) Kwave::MultiTrackWriter(
	    sig, track_list, Kwave::Overwrite, 0, last);
    } else {
	// use all currently selected tracks
	writers = new(std::nothrow)
	    Kwave::MultiTrackWriter(sig, Kwave::Overwrite);
    }

    Q_ASSERT(writers);
    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // connect the progress dialog
    connect(writers, SIGNAL(progress(qreal)),
	    this,  SLOT(updateProgress(qreal)),
	     Qt::BlockingQueuedConnection);

    // loop over the sample range
    const sample_index_t left   = first;
    const sample_index_t right  = last;
    const sample_index_t length = right - left + 1;
    sample_index_t pos = first;
    while ((first <= last) && (!shouldStop())) {
	sample_index_t rest = last - first + 1;
	if (rest < m_buffer.size()) {
	    bool ok = m_buffer.resize(Kwave::toUint(rest));
	    Q_ASSERT(ok);
	    if (!ok) break;
	}

	// sawtooth pattern from min to max
	if (command == _("fm_sweep")) {
	    const double f_max = rate / 2.0;
	    const double f_min = 1;
	    for (unsigned int i = 0; i < m_buffer.size(); ++i, ++pos) {
		double t = static_cast<double>(pos - left) / rate;
		double f = f_min + (((f_max - f_min) *
		    static_cast<double>(pos - left)) /
		    static_cast<double>(length));
		double y = 0.707 * sin(M_PI * f * t);
		m_buffer[i] = double2sample(y);
	    }
	} else if (command == _("sawtooth")) {
	    for (unsigned int i = 0; i < m_buffer.size(); ++i, ++pos) {
		m_buffer[i] = SAMPLE_MIN +
		    static_cast<sample_t>(pos % (SAMPLE_MAX - SAMPLE_MIN));
	    }
	} else if (command == _("dc_50")) {
	    const sample_t s = float2sample(0.5);
	    for (unsigned int i = 0; i < m_buffer.size(); ++i) {
		m_buffer[i] = s;
	    }
	} else if (command == _("dc_100")) {
	    const sample_t s = SAMPLE_MAX;
	    for (unsigned int i = 0; i < m_buffer.size(); ++i) {
		m_buffer[i] = s;
	    }
	}

	// loop over all writers
	for (unsigned int w = 0; w < count; ++w) {
	    *((*writers)[w]) << m_buffer;
	}

	first += m_buffer.size();
    }

    delete writers;
}

//***************************************************************************
void Kwave::DebugPlugin::dump_children(const QObject *obj,
                                       const QString &indent) const
{
    if (!obj) return;
    const char *classname = obj->metaObject()->className();
    qDebug("%s - %p [%s]",
	DBG(indent),
	static_cast<const void *>(obj),
	classname
    );

    foreach (QObject *o, obj->children()) {
	dump_children(o, indent + _("|   "));
    }
}

//***************************************************************************
QWidget *Kwave::DebugPlugin::findWidget(const char *class_name) const
{
    QObject *obj = findObject(parentWidget(), class_name);
    if (!obj) return Q_NULLPTR;
    return qobject_cast<QWidget *>(obj);
}

//***************************************************************************
QObject *Kwave::DebugPlugin::findObject(QObject *obj,
                                        const char *class_name) const
{
    if (!obj) return Q_NULLPTR;
    const char *obj_class_name = obj->metaObject()->className();
    if (strcmp(class_name, obj_class_name) == 0)
	return obj;

    foreach (QObject *o, obj->children()) {
	QObject *result = findObject(o, class_name);
	if (result) return result; // first match -> found
    }

    return Q_NULLPTR; // nothing found
}

//***************************************************************************
void Kwave::DebugPlugin::screenshot(const QByteArray &class_name,
                                    const QString &filename)
{
    // find the first widget/window with the given class name
    QWidget *widget = findWidget(class_name.constData());
    qDebug("screenshot of '%s' [%p] -> '%s'",
	class_name.constData(),
	static_cast<void *>(widget),
	DBG(filename)
    );
    if (!widget) return;

    // get the outer frame geometry, absolute coordinates
    const QRect rect = widget->windowHandle()->frameGeometry();
    QScreen *screen = QGuiApplication::primaryScreen();
    Q_ASSERT(screen);
    if (!screen) return;
    QPixmap pixmap = screen->grabWindow(
	QApplication::desktop()->winId(),
	rect.x(), rect.y(),
	rect.width(), rect.height()
    );

    QString str;
    str = str.asprintf("screenshot of %s - [%p] %d/%d %dx%d",
	DBG(filename), static_cast<void*>(widget),
	rect.x(), rect.y(), rect.width(), rect.height()
    );
    Kwave::Logger::log(this, Logger::Info, str);

    // make sure the directory exists
    QFileInfo file(filename);
    QDir dir = file.absoluteDir();
    if (!dir.exists()) dir.mkpath(dir.absolutePath());

    // save the file
    pixmap.save(filename, "PNG", 90);
}

//***************************************************************************
#include "DebugPlugin.moc"
//***************************************************************************
//***************************************************************************
