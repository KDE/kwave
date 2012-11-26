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
#include <stdio.h>
#include <string.h>

#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QThread>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>

#include "Utils.h"

//***************************************************************************
void Kwave::yield()
{
    pthread_testcancel();
    QThread::yieldCurrentThread();
}

//***************************************************************************
QString Kwave::zoom2string(double percent)
{
    QString result = "";

    if (percent < 1.0) {
	int digits = static_cast<int>(ceil(1.0 - log10(percent)));
	QString format;
	format = "%0."+format.setNum(digits)+"f %%";
	result = format.sprintf(format.toUtf8(), percent);
    } else if (percent < 10.0) {
	result = result.sprintf("%0.1f %%", percent);
    } else if (percent < 1000.0) {
	result = result.sprintf("%0.0f %%", percent);
    } else {
	result = result.sprintf("x %d",
	    static_cast<int>(rint(percent / 100.0)));
    }
    return result;
}

//***************************************************************************
QString Kwave::ms2string(double ms, int precision)
{
    char buf[128];
    int bufsize = 128;

    if (ms < 1.0) {
	char format[128];
	// limit precision, use 0.0 for exact zero
	int digits = (ms != 0.0) ? static_cast<int>(ceil(1.0 - log10(ms))) : 1;
	if ( (digits < 0) || (digits > precision)) digits = precision;

	snprintf(format, sizeof(format), "%%0.%df ms", digits);
	snprintf(buf, bufsize, format, ms);
    } else if (ms < 1000.0) {
	snprintf(buf, bufsize, "%0.1f ms", ms);
    } else {
	int s = static_cast<int>(round(ms / 1000.0));
	int m = static_cast<int>(floor(s / 60.0));

	if (m < 1) {
	    char format[128];
	    int digits = static_cast<int>(
		ceil(static_cast<double>(precision+1) - log10(ms)));
	    snprintf(format, sizeof(format), "%%0.%df s", digits);
	    snprintf(buf, bufsize, format, ms / 1000.0);
	} else {
	    snprintf(buf, bufsize, "%02d:%02d min", m, s % 60);
	}
    }

    QString result(buf);
    return result;
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

    QString hms_format = i18nc(
	"time of label tooltip, %1=hour, %2=minute, %3=second, %4=milliseconds",
	"%02u:%02u:%02u.%04u");
    QString hms;
    hms.sprintf(hms_format.toUtf8().data(), h, m, s, tms);

    return hms;
}

//***************************************************************************
QString Kwave::dottedNumber(unsigned int number)
{
    const QString num = QString::number(number);
    QString dotted = "";
    const QString dot = KGlobal::locale()->thousandsSeparator();
    const int len = num.length();
    for (int i=len-1; i >= 0; i--) {
	if ((i != len-1) && !((len-i-1) % 3)) dotted = dot + dotted;
	dotted = num.at(i) + dotted;
    }
    return dotted;
}

//***************************************************************************
QString Kwave::string2date(const QString &str)
{
    const Qt::DateFormat formats[] = {
	Qt::ISODate,
	Qt::TextDate,
	Qt::SystemLocaleShortDate,
	Qt::SystemLocaleLongDate,
	Qt::DefaultLocaleShortDate,
	Qt::DefaultLocaleLongDate
    };
    QString s;
    const unsigned int fmt_count =
	sizeof(formats) / sizeof(formats[0]);
    QDateTime dt;

    // try ID3 full date/time
    dt = QDateTime::fromString(str, "yyyy-MM-ddThh:mm:ss");
    if (dt.isValid())
	return str; // already in complete date/time format

    // type ID3 date without time
    dt = QDateTime::fromString(str, "yyyy-MM-dd");
    if (dt.isValid())
	return str; // already a valid date

    // try all date/time formats supported by Qt
    for (unsigned int i = 0; i < fmt_count; i++) {
	Qt::DateFormat fmt = formats[i];
	s = QString();

	dt = QDateTime::fromString(str, fmt);

	if (dt.isValid()) {
	    // full timestamp, including time
	    s = dt.toString("yyyy-MM-ddThh:mm:ss");
	}
	if (!s.length()) {
	    // date only, without time
	    dt = QDateTime(QDate::fromString(str), QTime(0,0));
	    s = dt.toString("yyyy-MM-dd");
	}

	if (s.length()) {
	    return s;
	}
    }

    return QString();
}

//***************************************************************************
//***************************************************************************
