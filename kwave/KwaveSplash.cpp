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

#include <qpixmap.h>
#include <qapplication.h>
#include <kstandarddirs.h>

#include "KwaveSplash.h"

/***************************************************************************/
KwaveSplash::KwaveSplash(const QString &PNGImageFileName)
    :QWidget(0, "Kwave Splash", WStyle_NoBorder | WStyle_StaysOnTop |
    WStyle_Customize), m_timer()
{
    QString file = locate("appdata", PNGImageFileName);
    QPixmap pixmap(file);

    // the size of the splashscreen image
    int h = pixmap.width();
    int l = pixmap.height();

    // center the image on the desktop
    setGeometry(QApplication::desktop()->width ()/2-(h/2),
                QApplication::desktop()->height()/2-(l/2), h, l);
    setFixedSize(h, l);

    setPaletteBackgroundPixmap(pixmap);

    // auto-close in 2 seconds...
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(deleteLater()));
    m_timer.start(2000, true);
}

/***************************************************************************/
KwaveSplash::~KwaveSplash()
{
}

/***************************************************************************/
/***************************************************************************/
