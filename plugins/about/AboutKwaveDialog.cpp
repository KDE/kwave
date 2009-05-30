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

#include "config.h"
#include <dlfcn.h>

#include <QList>
#include <QListIterator>
#include <QHBoxLayout>
#include <QString>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kstandarddirs.h>

#include "AboutKwaveDialog.h"
#include "KwaveAboutContainer.h"
#include "LogoWidget.h"

//***************************************************************************
AboutKwaveDialog::AboutKwaveDialog(QWidget *parent)
    :QDialog(parent), Ui::KwaveAboutDialogBase()
{
    setupUi(this);

    /* get the about data defined in main() */
    const KAboutData *about_data = KGlobal::mainComponent().aboutData();

    /* display version information in the header */
    QString kde_version = QString::fromLatin1(KDE_VERSION_STRING);
    QString kwave_version = about_data->programName()+
        " " + about_data->version() + " ";
    QString header_text = "<h2>"+kwave_version+
        i18n("(built for KDE %1)", kde_version)+"</h2>";
    header->setText(header_text);

    /* the frame containing the developer information */
    KwaveAboutContainer *about = new KwaveAboutContainer(this);
    foreach (const KAboutPerson &author, about_data->authors()) {
	about->addPerson(author.name(), author.emailAddress(),
	    author.webAddress(), author.task());
    }
    authorframe->setWidget(about);
    authorframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* the frame containing the thanks to ... */
    KwaveAboutContainer *contrib = new KwaveAboutContainer(this);
    foreach (const KAboutPerson &credit, about_data->credits()) {
	contrib->addPerson(credit.name(), credit.emailAddress(),
	    credit.webAddress(), credit.task());
    }
    thanksframe->setWidget(contrib);
    thanksframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* the frame containing the plugins info */
    QTreeWidget *pluginsinfo = new QTreeWidget(pluginsinfoframe);
    QVBoxLayout *plugins_layout = new QVBoxLayout(pluginsinfoframe);
    plugins_layout->addWidget(pluginsinfo);
    pluginsinfoframe->setLayout(plugins_layout);

    pluginsinfo->setColumnCount(3);
    pluginsinfo->setSizePolicy(QSizePolicy::MinimumExpanding,
                               QSizePolicy::Expanding);
    pluginsinfo->setSelectionMode(QAbstractItemView::NoSelection);
    QStringList headers;
    headers << i18n("name") << i18n("version") << i18n("authors");
    pluginsinfo->setHeaderLabels(headers);
    pluginsinfo->setAllColumnsShowFocus( false );
    pluginsinfo->setItemsExpandable(false);
    pluginsinfo->setRootIsDecorated(false);

    KStandardDirs dirs;
    QStringList files = dirs.findAllResources("module",
	    "plugins/kwave/*", KStandardDirs::NoDuplicates);

    /* fallback: search also in the old location (useful for developers) */
    files += dirs.findAllResources("data",
	    "kwave/plugins/*", KStandardDirs::NoDuplicates);

    QStringList::Iterator it_file;
    QList<QTreeWidgetItem *> plugins;
    foreach (QString file, files) {
	void *handle = dlopen(file.toLocal8Bit(), RTLD_NOW);
	if (handle) {
	    const char **name    =
		static_cast<const char **>(dlsym(handle, "name"));
	    const char **version =
		static_cast<const char **>(dlsym(handle, "version"));
	    const char **author  =
		static_cast<const char **>(dlsym(handle, "author"));

	    // skip it if something is missing or null
	    if (!name || !version || !author) continue;
	    if (!*name || !*version || !*author) continue;

	    QStringList item;
	    item << i18n(*name) << *version << *author;
	    plugins.append(new QTreeWidgetItem(
		static_cast<QTreeWidget *>(0), item));

	    dlclose (handle);
        }
    }
    pluginsinfo->insertTopLevelItems(0, plugins);

    QString num_plugins = i18n("Plugins found: %1", plugins.count());
    pluginsnumval->setText(num_plugins);
    pluginsinfoframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* set the url of the kwave homepage */
    kwave_url_label->setText("<a href=\"" +
	about_data->homepage() + "\">" +
	about_data->homepage() + "</a>");
    kwave_url_label->setOpenExternalLinks(true);
    kwave_url_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    /* the frame containing the translators */
    KwaveAboutContainer *trans = new KwaveAboutContainer(this);
    foreach (const KAboutPerson &translator, about_data->translators()) {
	trans->addPerson(translator.name(), translator.emailAddress(),
	    translator.webAddress(), translator.task());
    }
    translatorsframe->setWidget(trans);
    translatorsframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* the frame containing the license */
    licenseframe->setReadOnly(true);
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
