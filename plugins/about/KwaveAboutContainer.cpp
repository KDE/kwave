/***************************************************************************
    KwaveAboutContainer.cpp  -  Base class for the authors and thanks field in
            the kwave about dialog
                              -------------------
    begin                : Sat Mar 9 2002
    copyright            : (C) 2002 by Ralf Waspe
    email                : rwaspe@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapp.h>

#include "KwaveAboutContainer.h"

//***************************************************************************

KwaveAboutContainer::KwaveAboutContainer(QWidget* parent, const char* name)
    :KAboutContainer(parent,name,10,10,0,0)
{
    setFrameStyle(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    QSize tabsize = parent->size();
    setGeometry(QRect(0,0,tabsize.width(),tabsize.height()));

    connect(this, SIGNAL(urlClick(const QString &)),
            this, SLOT(openURL(const QString &)));
    connect(this, SIGNAL(mailClick(const QString &,const QString &)),
            this, SLOT(sendMail(const QString &,const QString &)));
}

//***************************************************************************

KwaveAboutContainer::~KwaveAboutContainer()
{
}

//***************************************************************************

void KwaveAboutContainer::openURL(const QString &url)
{
    warning("open url %s",url.data());
    kapp->invokeBrowser(url);
}

//***************************************************************************

void KwaveAboutContainer::sendMail(const QString &name,const QString &address)
{
    warning("mailto %s",address.data());
    kapp->invokeMailer(address,QString::null);
}

//***************************************************************************
//***************************************************************************
