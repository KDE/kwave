/***************************************************************************
             Logger.cpp  -  Kwave log file handling
                             -------------------
    begin                : Sat May 17 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#include <new>

#include <QtCore/QtGlobal>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>

#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/String.h"

// static initializers
QFile *Kwave::Logger::m_logfile = 0;
static Kwave::Logger g_logger;

/** helper macro: returns the number of elements in an array */
#define ELEMENTS_OF(__array__) (sizeof(__array__) / sizeof(__array__[0]))

//***************************************************************************
Kwave::Logger::Logger()
{
}

//***************************************************************************
Kwave::Logger::~Logger()
{
    if (m_logfile) {
	log(0, Kwave::Logger::Info, _("--- CLOSED / APPLICATION SHUTDOWN ---"));
	m_logfile->flush();
	delete m_logfile;
    }
}

//***************************************************************************
bool Kwave::Logger::open(const QString& filename)
{
    if (m_logfile) {
	qWarning("reopening log file");
	log(0, Kwave::Logger::Info, _("--- CLOSED / REOPEN ---"));
	m_logfile->flush();
	delete m_logfile;
    }
    qDebug("logging to file: '%s'", DBG(filename));

    QString name(filename);
    m_logfile = new (std::nothrow) QFile(name);
    Q_ASSERT(m_logfile);

    if (m_logfile) m_logfile->open(
	QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (!m_logfile || (!m_logfile->isWritable())) {
	if (Kwave::MessageBox::warningContinueCancel(0,
	    i18n("Failed opening the log file '%1' for writing").arg(
	    filename)) != KMessageBox::Continue)
	{
	    return false;
	}
    }

    /*
     * provide the "header" of an extended log file
     * see http://www.w3.org/TR/WD-logfile.html
     */

    QTextStream out(m_logfile);
    const KAboutData *about_data = KGlobal::mainComponent().aboutData();

    out << "#Version: 1.0" << endl;
    out << "#Fields: x-status date time x-pid x-message" << endl;
    out << "#Software: " << about_data->programName() << " "
                         << about_data->version() << endl;
    QDateTime now = QDateTime::currentDateTime();
    out << "#Start-Date: " << now.toString(_("yyyy-MM-dd hh:mm:ss")) << endl;

    return true;
}

//***************************************************************************
void Kwave::Logger::log(const QObject *sender,
                        Kwave::Logger::LogLevel level,
                        const QString& msg)
{
    static const char *str_level[] = {
	"DBG", "INF", "WAR", "ERR", "FAT"
    };
    if (!m_logfile) return;

    // NOTE: it would be fine to have a way to find out the instance
    //       which this message belongs to (TopLevelWidget + MainWidget)
    Q_UNUSED(sender);

    // translate the log level into a text (syslog format)
    const char *x_status = str_level[qBound(
	Q_UINT64_C(0),
	static_cast<quint64>(level),
	static_cast<quint64>(ELEMENTS_OF(str_level)))
    ];

    // get the time stamp
    QDateTime now = QDateTime::currentDateTime();
    QString date_time = now.toString(_("yyyy-MM-dd hh:mm:ss.zzz"));

    // get the PID of the application
    long int x_pid = static_cast<long int>(qApp->applicationPid());

    // format the log log message
    // x-status date time x-pid x-message
    QString line;
    line.sprintf("<%s> %s %ld %s\n",
	x_status,
	__(date_time),
	x_pid,
	__(msg)
    );

    m_logfile->write(line.toUtf8().constData());
    m_logfile->flush();
}

//***************************************************************************
//***************************************************************************
