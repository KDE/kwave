/***************************************************************************
                Utils.h  -  some commonly used utility functions
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

#ifndef KWAVE_UTILS_H
#define KWAVE_UTILS_H

#include "config.h"
#include "libkwave_export.h"

#include <limits>

#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "libkwave/Sample.h" // for sample_index_t

/** helper macro for switch/case where the default is not relevant */
#define DEFAULT_IGNORE     default: break

/** helper macro for switch/case where reaching the default is impossible */
#define DEFAULT_IMPOSSIBLE default: Q_ASSERT(false); break

namespace Kwave
{

    /**
     * Gives the control to the next thread. This can be called from
     * within the run() function.
     */
    void LIBKWAVE_EXPORT yield();

    /**
     * Converts a zoom factor into a string. The number of decimals
     * is automatically adjusted in order to give a nice formated
     * percent value. If the zoom factor gets too high for a reasonable
     * display in percent, the factor is displayed as a numeric
     * multiplier.
     * examples: "0.1 %", "12.3 %", "468 %", "11x"
     * @param percent the zoom factor to be formated, a value of "100.0"
     *             means "100%", "0.1" means "0.1%" and so on.
     */
    QString LIBKWAVE_EXPORT zoom2string(double percent);

    /**
     * Converts a time in milliseconds into a string. Times below one
     * millisecond are formated with an automatically adjusted number
     * of decimals. Times below one second are formated like "9.9 ms".
     * Times above one second and below one minute are rounded up
     * to full seconds and shown as "12.3 s". From one full minute
     * upwards time is shown as "12:34" (like most CD players do).
     * @param ms time in milliseconds
     * @param precision number of digits after the comma, for limiting
     *                  the length. optional, default = 6 digits,
     *                  must be >= 3 !
     * @return time formatted as user-readable string
     */
    QString LIBKWAVE_EXPORT ms2string(double ms, int precision = 6);

    /**
     * Converts a number of samples (aka sample_index_t) into a string,
     * according the current locale settings.
     * @param samples number of sample
     * @return number formatted with thousands separators
     */
    QString LIBKWAVE_EXPORT samples2string(sample_index_t samples);

    /**
     * Converts a time in milliseconds into a string with hours,
     * minutes, seconds and milliseconds.
     * @param ms time in milliseconds
     * @return time formatted as HH:MM:SS:mmmm
     */
    QString LIBKWAVE_EXPORT ms2hms(double ms);

    /**
     * Tries to convert a string into a QDate
     * @param s string to convert
     * @return a ISO 8601 timestamp: "yyyy-MM-ddTHH:mm:ss"
     *         or shortened as date "yyyy-MM-dd"
     */
    QString LIBKWAVE_EXPORT string2date(const QString &s);

    /**
     * Round up a numeric value
     * @param x value to round up
     * @param s unit size
     * @return x rounded up to the next unit
     */
    template<class T> T round_up(T x, const T s)
    {
        T modulo = (x % s);
        if (modulo) x += (s - modulo);
        return x;
    }

    /**
     * Convert a numeric value into an unsigned int, with range / overflow
     * protection
     * @param x some numeric value, e.g. a sample_index_t or qint64
     * @return some unsigned int [0 ... UINT_MAX]
     */
    template <typename T> unsigned int toUint(T x)
    {
        const unsigned int max = std::numeric_limits<unsigned int>::max();
        Q_ASSERT(x >= 0);
        Q_ASSERT(static_cast<quint64>(x) <= static_cast<quint64>(max));

        if (x <= 0) return 0;
        if (static_cast<quint64>(x) > static_cast<quint64>(max)) return max;

        return static_cast<unsigned int>(x);
    }

    /**
     * Convert a numeric value into an int, with range / overflow
     * protection
     * @param x some numeric value, e.g. a sample_index_t or qint64
     * @return some (signed) int [INT_MIN ... INT_MAX]
     */
    template <typename T> int toInt(T x)
    {
        const int min = std::numeric_limits<int>::min();
        const int max = std::numeric_limits<int>::max();
        Q_ASSERT(static_cast<qint64>(x) >= static_cast<qint64>(min));
        Q_ASSERT(static_cast<qint64>(x) <= static_cast<qint64>(max));

        if (static_cast<qint64>(x) < static_cast<qint64>(min)) return min;
        if (static_cast<qint64>(x) > static_cast<qint64>(max)) return max;

        return static_cast<int>(x);
    }

    /**
     * Wrapper around Kwave::URLfromUserInput that handles relative paths and
     * prefers local file names
     * @param path a file name, path, or other kind of string encoded URL
     * @return a QUrl object
     */
    QUrl URLfromUserInput(const QString &path) LIBKWAVE_EXPORT;

    /** returns the URL scheme for encoding/decoding kwave:<*> URLs */
    QString urlScheme() LIBKWAVE_EXPORT;

    /**
     * Returns the limit of memory that can be used for undo/redo
     * in units of whole megabytes
     */
    quint64 undoLimit() LIBKWAVE_EXPORT;

}

#endif /* KWAVE_UTILS_H */

//***************************************************************************
//***************************************************************************
