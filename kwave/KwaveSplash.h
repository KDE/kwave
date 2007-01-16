/*************************************************************************
          KwaveSplash.h  -  splash screen for Kwave
                             -------------------
    begin                : Tue Jun 24 2003
    copyright            : Copyright (C) 2003 Gilles CAULIER
    email                : Gilles CAULIER <caulier.gilles@free.fr>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KWAVE_SPLASH_H_
#define _KWAVE_SPLASH_H_

#include <qobject.h>
#include <qwidget.h>
#include <qstring.h>
#include <qtimer.h>

class KwaveSplash : public QWidget
{
Q_OBJECT

public:
    /**
     * Constructor
     * @param PNGImageFileName name of a file to be shown as splashscreen,
     *        should be found in one of the "appdata" directories.
     */
    KwaveSplash(const QString &PNGImageFileName);

    /** Destructor */
    virtual ~KwaveSplash();

private:

    /** timer for auto-destruction of the splashscreen */
    QTimer m_timer;
    
};

#endif /* _KWAVE_SPLASH_H_ */
