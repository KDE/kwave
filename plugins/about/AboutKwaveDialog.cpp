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
#include <qscrollview.h>
#include <qstring.h>
#include <qtextview.h>
#include <qhbox.h>

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

    /* the frame containing the developer information */
    QHBox *author_layout = new QHBox(authorframe->viewport());
    authorframe->addChild(author_layout);
    KAboutContainer* about = new KwaveAboutContainer(author_layout);

    QValueList<KAboutPerson>::ConstIterator it;
    for (it = about_data->authors().begin();
        it != about_data->authors().end();++it){
            about->addPerson((*it).name(),(*it).emailAddress(),
            (*it).webAddress(),(*it).task());
    }
    authorframe->setResizePolicy(QScrollView::AutoOneFit);

    /* the frame containing the thanks to .. */
    QHBox *thanks_layout = new QHBox(thanksframe->viewport());
    thanksframe->addChild(thanks_layout);
    KAboutContainer* contrib = new KwaveAboutContainer(thanks_layout);
    for (it = about_data->credits().begin();
        it != about_data->credits().end(); ++it){
        contrib->addPerson((*it).name(),(*it).emailAddress(),
            (*it).webAddress(),i18n((*it).task()));
    }
    thanksframe->setResizePolicy(QScrollView::AutoOneFit);

    /* set the url of the kwave homepage */
    kwave_url_label->setText(about_data->homepage());
    kwave_url_label->setURL(about_data->homepage());
    connect(kwave_url_label, SIGNAL(leftClickedURL(const QString &)),
            about, SLOT(openURL(const QString &)));

    /* the frame containing the license */
    licenseframe->setText(about_data->license());
}

//***************************************************************************
AboutKwaveDialog::~AboutKwaveDialog()
{
}

//***************************************************************************
//***************************************************************************
