/***************************************************************************
   AboutKwaveDialog.cpp  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Feb 10 2002
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

#include <qlabel.h>
#include <qframe.h>
#include <qstring.h>
#include <qtextview.h>

#include <kaboutdialog.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurllabel.h>

#include "AboutKwaveDialog.h"
#include "KwaveAboutContainer.h"
#include "LogoWidget.h"

//***************************************************************************
AboutKwaveDialog::AboutKwaveDialog(QWidget *parent)
    :KwaveAboutDialogBase(parent,"kwaveabout",true)
{

    /* get the about data defined in main() */
    const KAboutData *about_data = KGlobal::instance()->aboutData();

    /* display version information in the header */
    QString kde_version = QString::fromLatin1(KDE_VERSION_STRING);
    QString kwave_version = about_data->programName()+
        " "+about_data->version();
    QString header_text = "<h2>"+kwave_version+
        i18n(" (Using KDE %1)").arg(kde_version)+"</h2>";
    header->setText(header_text);

    /* set the url of the kwave homepage */
    kwave_url_label->setText(about_data->homepage());

    /* the frame containing the developer information */
    QValueList<KAboutPerson>::ConstIterator it;
    KAboutContainer* about = new KwaveAboutContainer(authorframe);
    for (it = about_data->authors().begin();
        it != about_data->authors().end();++it){
            about->addPerson((*it).name(),(*it).emailAddress(),
            (*it).webAddress(),(*it).task());
    }

    /* the frame containing the thanks to .. */
    KAboutContainer* contrib = new KwaveAboutContainer(thanksframe);
    for (it = about_data->credits().begin();
        it != about_data->credits().end(); ++it){
        contrib->addPerson((*it).name(),(*it).emailAddress(),
            (*it).webAddress(),i18n((*it).task()));
    }

    /* the frame containing the license */
    licenseframe->setText(about_data->license());
}

//***************************************************************************
AboutKwaveDialog::~AboutKwaveDialog()
{
}

//***************************************************************************
//***************************************************************************
