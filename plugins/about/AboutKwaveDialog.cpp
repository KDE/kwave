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

#include <QtAlgorithms>
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
static bool pluginInfoDescriptionLessThan(
    const Kwave::PluginManager::PluginInfo &info1,
    const Kwave::PluginManager::PluginInfo &info2)
{
    return info1.m_description.toLower() < info2.m_description.toLower();
}

//***************************************************************************
AboutKwaveDialog::AboutKwaveDialog(
    QWidget *parent,
    const QList<Kwave::PluginManager::PluginInfo> &plugin_info
)
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

    QList<QTreeWidgetItem *> plugins;

    QList<Kwave::PluginManager::PluginInfo> list = plugin_info;
    qSort(list.begin(), list.end(), pluginInfoDescriptionLessThan);
    foreach (const Kwave::PluginManager::PluginInfo &info, list) {
	QStringList item;
	item << info.m_description << info.m_version << info.m_author;
	plugins.append(new QTreeWidgetItem(
	    static_cast<QTreeWidget *>(0), item));
    }
    pluginsinfo->insertTopLevelItems(0, plugins);
    pluginsinfo->resizeColumnToContents(1);
    pluginsinfo->resizeColumnToContents(0);

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
    QList<KAboutPerson> translators = about_data->translators();
    if ((translators.count() == 1) &&
        (translators.first().name() == "NAME OF TRANSLATORS")) {
	tabwidget->removeTab(4);
    } else {
	foreach (KAboutPerson translator, translators) {
	    QString website = translator.webAddress();

	    // if the translator is already listed in the "authors" section,
	    // give him the same web address
	    foreach (const KAboutPerson &author, about_data->authors())
		if (author.name() == translator.name()) {
		    website = author.webAddress();
		    break;
		}

	    // if the translator is already listed in the "credits" section,
	    // give him the same web address
	    foreach (const KAboutPerson &credit, about_data->credits())
		if (credit.name() == translator.name()) {
		    website = credit.webAddress();
		    break;
		}

	    trans->addPerson(translator.name(), translator.emailAddress(),
		website, translator.task());
	}
	translatorsframe->setWidget(trans);
	translatorsframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

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
