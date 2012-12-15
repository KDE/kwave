/***************************************************************************
               String.h  -  some commonly used string conversion
                             -------------------
    begin                : Sat Dec 08 2011
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef _KWAVE_STRING_H_
#define _KWAVE_STRING_H_

#include "config.h"

#include <QtCore/QString>

/*
 * This is a workaround for compiling Kwave with QT_NO_CAST_FROM_ASCII,
 * which is really pain in the a**.
 * Instead of spilling the whole source code with useless thousands
 * of QLatin1String(...) conversions, we use a small conversion
 * function in that case, which does not blow up the code so much...
 */
#ifdef QT_NO_CAST_FROM_ASCII
    #include <QtCore/QLatin1String>
    static inline QString _(const char *s) { return QLatin1String(s); }
#else
    static inline QString _(const char *s) { return QString(s); }
#endif

/**
 * helper for converting QString to const char *, useful for debug output
 * @param s a QString
 * @return a const char * in UTF8 representation
 */
#define __(__s__) ((__s__).toUtf8().data())

/**
 * helper for converting QString to const char *, useful for debug output
 * @param s a QString
 * @return a const char * in local 8 bit representation
 */
#define DBG(__s__) ((__s__).toLocal8Bit().data())

#endif /* _KWAVE_STRING_H_ */

//***************************************************************************
//***************************************************************************
