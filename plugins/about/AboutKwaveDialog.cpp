/***************************************************************************
   AboutKwaveDialog.cpp  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Feb 10 2002
    copyright            : (C) 2002 by Ralf Waspe & Gilles Caulier
    email                : rwaspe@web.de / caulier.gilles@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <dlfcn.h>

#include <qlabel.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qsizepolicy.h>
#include <qstring.h>
#include <qtextview.h>
#include <qlistview.h>
#include <qhbox.h>
#include <qvbox.h>

#include <kaboutdialog.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurllabel.h>
#include <klistview.h>
#include <kstddirs.h>

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
    QVBox *author_layout = new QVBox(authorframe->viewport());
    authorframe->addChild(author_layout);
    KAboutContainer *about = new KwaveAboutContainer(author_layout);

    QValueList<KAboutPerson>::ConstIterator it;
    for (it = about_data->authors().begin();
        it != about_data->authors().end();++it)
    {
        about->addPerson((*it).name(),(*it).emailAddress(),
        (*it).webAddress(),(*it).task());
    }
    authorframe->setResizePolicy(QScrollView::AutoOneFit);

    /* the frame containing the thanks to ... */
    QHBox *thanks_layout = new QHBox(thanksframe->viewport());
    thanksframe->addChild(thanks_layout);
    KAboutContainer* contrib = new KwaveAboutContainer(thanks_layout);

    for (it = about_data->credits().begin();
        it != about_data->credits().end(); ++it)
    {
        contrib->addPerson((*it).name(),(*it).emailAddress(),
            (*it).webAddress(),(*it).task());
    }
    thanksframe->setResizePolicy(QScrollView::AutoOneFit);

    /* the frame containing the plugins info */
    QHBox *pluginsinfo_layout = new QHBox(pluginsinfoframe->viewport());
    pluginsinfoframe->addChild(pluginsinfo_layout);
    KListView *pluginsinfo = new KListView(pluginsinfo_layout);

    pluginsinfo->setSelectionMode (QListView::Single);
    pluginsinfo->addColumn( i18n("name") );
    pluginsinfo->addColumn( i18n("version") );
    pluginsinfo->addColumn( i18n("authors") );
    pluginsinfo->setAllColumnsShowFocus( false );
    pluginsinfo->setShowSortIndicator (false);
    pluginsinfo->setSorting (0);

    QStringList files = KGlobal::dirs()->findAllResources(
        "data", "kwave/plugins/*", false, true);
    QStringList::Iterator it_file;
    for (it_file=files.begin(); it_file != files.end(); ++it_file) {
	QString file = *it_file;
	void *handle = dlopen(file, RTLD_NOW);

        if (handle) {
	     const char **name    = (const char **)dlsym(handle, "name");
	     const char **version = (const char **)dlsym(handle, "version");
	     const char **author  = (const char **)dlsym(handle, "author");

	     // skip it if something is missing or null
	     if (!name || !version || !author) continue;
	     if (!*name || !*version || !*author) continue;

	     QListViewItem *plugins_item;
	     plugins_item = new QListViewItem(pluginsinfo, i18n(*name),
	         *version, *author);
	     dlclose (handle);
        }
    }

    QString num_plugins;
    num_plugins.setNum(pluginsinfo->childCount());
    pluginsnumval->setText(num_plugins);
    pluginsinfoframe->setResizePolicy(QScrollView::AutoOneFit);

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
#include "AboutKwaveDialog.moc"
//***************************************************************************
//***************************************************************************
