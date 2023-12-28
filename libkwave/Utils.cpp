/***************************************************************************
              Utils.cpp  -  some commonly used utility functions
                             -------------------
    begin                : Sun Mar 27 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#include <math.h>
#include <pthread.h>
#include <string.h>

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QLatin1Char>
#include <QLocale>
#include <QString>
#include <QThread>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

//***************************************************************************
void Kwave::yield()
{
    pthread_testcancel();
    QThread::yieldCurrentThread();
}

//***************************************************************************
QString Kwave::zoom2string(double percent)
{
    QString result;

    if (percent < 1000.0) {
	int digits;

	if (percent < 1.0)
	    digits = Kwave::toInt(ceil(1.0 - log10(percent)));
	else if (percent < 10.0)
	    digits = 1;
	else
	    digits = 0;
	result.setNum(percent, 'f', digits);
	result += _(" %");
    } else {
	result.setNum(Kwave::toInt(rint(percent / 100.0)));
	result.prepend(_("x "));
    }
    return result;
}

//***************************************************************************
QString Kwave::ms2string(double ms, int precision)
{
    QString result;

    if (ms < 1.0) {
	// limit precision, use 0.0 for exact zero
	int digits = (ms != 0.0) ? Kwave::toInt(ceil(1.0 - log10(ms))) : 1;
	if ( (digits < 0) || (digits > precision)) digits = precision;

	result = _("%1 ms").arg(ms, 0, 'f', digits);
    } else if (ms < 1000.0) {
	result = _("%1 ms").arg(ms, 0, 'f', 1);
    } else {
	int s = Kwave::toInt(round(ms / 1000.0));
	int m = Kwave::toInt(floor(s / 60.0));

	if (m < 1) {
	    int digits = Kwave::toInt(
		ceil(static_cast<double>(precision + 1) - log10(ms)));
	    result = _("%1 s").arg(
		static_cast<double>(ms) / 1000.0, 0, 'f', digits);
	} else {
	    result = _("%1:%2 min").arg(
		m,      2, 10, QLatin1Char('0')).arg(
		s % 60, 2, 10, QLatin1Char('0'));
	}
    }

    return result;
}

//***************************************************************************
QString Kwave::samples2string(sample_index_t samples)
{
    return QLocale().toString(Kwave::toUint(samples));
}

//***************************************************************************
QString Kwave::ms2hms(double ms)
{
    unsigned int t, h, m, s, tms;
    t = static_cast<unsigned int>(rint(ms * 10.0));
    tms = t % 10000;
    t /= 10000;
    s = t % 60;
    t /= 60;
    m = t % 60;
    t /= 60;
    h = t;

    return i18nc(
	"time of label tooltip, %1=hour, %2=minute, %3=second, %4=milliseconds",
	"%1:%2:%3.%4",
	QString::asprintf("%02u", h),
	QString::asprintf("%02u", m),
	QString::asprintf("%02u", s),
	QString::asprintf("%04u", tms)
    );
}

//***************************************************************************
QString Kwave::string2date(const QString &str)
{
    const Qt::DateFormat formats[] = {
	Qt::ISODate,
	Qt::TextDate,
    };
    const unsigned int fmt_count =
	sizeof(formats) / sizeof(formats[0]);
    QDateTime dt;

    // try ID3 full date/time
    dt = QDateTime::fromString(str, _("yyyy-MM-ddThh:mm:ss"));
    if (dt.isValid())
	return str; // already in complete date/time format

    // type ID3 date without time
    dt = QDateTime::fromString(str, _("yyyy-MM-dd"));
    if (dt.isValid())
	return str; // already a valid date

    // try all date/time formats supported by Qt
    dt = QLocale().toDateTime(str, QLocale::ShortFormat);
    if (dt.isValid()) {
	return dt.toString(_("yyyy-MM-ddThh:mm:ss"));
    }
    dt = QLocale().toDateTime(str, QLocale::LongFormat);
    if (dt.isValid()) {
	return dt.toString(_("yyyy-MM-ddThh:mm:ss"));
    }
    dt = QLocale::system().toDateTime(str, QLocale::ShortFormat);
    if (dt.isValid()) {
	return dt.toString(_("yyyy-MM-ddThh:mm:ss"));
    }
    dt = QLocale::system().toDateTime(str, QLocale::LongFormat);
    if (dt.isValid()) {
	return dt.toString(_("yyyy-MM-ddThh:mm:ss"));
    }
    for (unsigned int i = 0; i < fmt_count; i++) {
	Qt::DateFormat fmt = formats[i];
	QString s;

	dt = QDateTime::fromString(str, fmt);

	if (dt.isValid()) {
	    // full timestamp, including time
	    s = dt.toString(_("yyyy-MM-ddThh:mm:ss"));
	}
	if (!s.length()) {
	    // date only, without time
	    dt = QDateTime(QDate::fromString(str), QTime(0,0));
	    s = dt.toString(_("yyyy-MM-dd"));
	}

	if (s.length()) {
	    return s;
	}
    }

    return QString();
}

//***************************************************************************
QUrl Kwave::URLfromUserInput(const QString &path)
{
    return QUrl::fromUserInput(path, QDir::currentPath(),
                               QUrl::AssumeLocalFile);
}

//***************************************************************************
QString Kwave::urlScheme()
{
    return _("kwave");
}

//***************************************************************************
quint64 Kwave::undoLimit()
{
    return 2048;
}

//***************************************************************************
//***************************************************************************
