/*************************************************************************
        KwaveSplash.cpp  -  splash screen for Kwave
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

#include "config.h"

#include <QPixmap>
#include <QString>
#include <QTimer>

#include <kstandarddirs.h>

#include "KwaveSplash.h"

//***************************************************************************
KwaveSplash::KwaveSplash(const QString &PNGImageFileName)
    :QSplashScreen(0),
     m_pixmap(KStandardDirs::locate("appdata", PNGImageFileName))
{
    setPixmap(m_pixmap);

    // auto-close in 2 seconds...
    QTimer::singleShot(2000, this, SLOT(deleteLater()));
}

//***************************************************************************
KwaveSplash::~KwaveSplash()
{
}

//***************************************************************************
#include "KwaveSplash.moc"
//***************************************************************************
//***************************************************************************
