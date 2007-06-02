/***************************************************************************
KwaveAboutContainer.cpp  -  Authors and thanks field in the about dialog
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

#include <qnamespace.h>
#include <kapp.h>

#include "KwaveAboutContainer.h"

//***************************************************************************
KwaveAboutContainer::KwaveAboutContainer(QWidget* parent, const char* name)
    :KAboutContainer(parent, name, 0, 0, (Qt::AlignTop | Qt::AlignLeft),
     (Qt::AlignTop | Qt::AlignLeft))
{
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
    kapp->invokeBrowser(url);
}

//***************************************************************************
void KwaveAboutContainer::sendMail(const QString &,const QString &address)
{
    kapp->invokeMailer(address, QString::null);
}

//***************************************************************************
#include "KwaveAboutContainer.moc"
//***************************************************************************
//***************************************************************************
