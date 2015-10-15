/***************************************************************************
        AboutDialog.cpp  -  dialog for Kwave's "Help-About"
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

#include <algorithm>

#include <QApplication>
#include <QHBoxLayout>
#include <QList>
#include <QListIterator>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QtAlgorithms>

#include <KAboutData>
#include <KLocalizedString>
#include <KTextEdit>
#include <KUrlLabel>
#include <kxmlgui_version.h>

#include "libkwave/String.h"

#include "AboutContainer.h"
#include "AboutDialog.h"
#include "LogoWidget.h"

//***************************************************************************
static bool pluginInfoDescriptionLessThan(
    const Kwave::PluginManager::PluginModule &info1,
    const Kwave::PluginManager::PluginModule &info2)
{
    return info1.m_description.toLower() < info2.m_description.toLower();
}

//***************************************************************************
Kwave::AboutDialog::AboutDialog(
    QWidget *parent,
    const QList<Kwave::PluginManager::PluginModule> &plugin_info
)
    :QDialog(parent), Ui::AboutDialogBase()
{
    setupUi(this);

    /* get the about data defined in main() */
    const KAboutData about_data = KAboutData::applicationData();

    /* display version information in the header */
    QString kde_version = QString::fromLatin1(KXMLGUI_VERSION_STRING);
    QString kwave_version = about_data.componentName()+
        _(" ") + about_data.version() + _(" ");
    QString header_text = _("<h2>") + kwave_version +
        i18n("(built for KDE %1)", kde_version) + _("</h2>");
    header->setText(header_text);

    /* the frame containing the developer information */
    Kwave::AboutContainer *about = new Kwave::AboutContainer(this);
    foreach (const KAboutPerson &author, about_data.authors()) {
	about->addPerson(
	    i18n(author.name().toUtf8()),
	    author.emailAddress(),
	    i18n(author.webAddress().toUtf8()),
	    i18n(author.task().toUtf8())
	);
    }
    authorframe->setWidget(about);
    authorframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* the frame containing the thanks to ... */
    Kwave::AboutContainer *contrib = new Kwave::AboutContainer(this);
    foreach (const KAboutPerson &credit, about_data.credits()) {
	contrib->addPerson(
	    i18n(credit.name().toUtf8()),
	    credit.emailAddress(),
	    i18n(credit.webAddress().toUtf8()),
	    i18n(credit.task().toUtf8())
	);
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

    QList<Kwave::PluginManager::PluginModule> list = plugin_info;
    if (!list.isEmpty()) {
	std::sort(list.begin(), list.end(), pluginInfoDescriptionLessThan);
	foreach (const Kwave::PluginManager::PluginModule &info, list) {
	    QStringList item;
	    item << i18n(info.m_description.toUtf8())
	         << info.m_version
	         << i18n(info.m_author.toUtf8());
	    plugins.append(new QTreeWidgetItem(
		static_cast<QTreeWidget *>(0), item));
	}
	pluginsinfo->insertTopLevelItems(0, plugins);
    }
    pluginsinfo->resizeColumnToContents(1);
    pluginsinfo->resizeColumnToContents(0);

    QString num_plugins = i18n("Plugins found: %1", plugins.count());
    pluginsnumval->setText(num_plugins);
    pluginsinfoframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* set the url of the kwave homepage */
    kwave_url_label->setText(_("<a href=\"") +
	about_data.homepage() + _("\">") +
	about_data.homepage() + _("</a>"));
    kwave_url_label->setOpenExternalLinks(true);
    kwave_url_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    /* the frame containing the translators */
    Kwave::AboutContainer *trans = new Kwave::AboutContainer(this);
    QList<KAboutPerson> translators = about_data.translators();
    if ((translators.count() == 1) &&
        (translators.first().name() == _("NAME OF TRANSLATORS"))) {
	tabwidget->removeTab(4);
    } else {
	foreach (const KAboutPerson &translator, translators) {
	    QString website = translator.webAddress();

	    // if the translator is already listed in the "authors" section,
	    // give him the same web address
	    foreach (const KAboutPerson &author, about_data.authors())
		if (author.name() == translator.name()) {
		    website = author.webAddress();
		    break;
		}

	    // if the translator is already listed in the "credits" section,
	    // give him the same web address
	    foreach (const KAboutPerson &credit, about_data.credits())
		if (credit.name() == translator.name()) {
		    website = credit.webAddress();
		    break;
		}

	    trans->addPerson(
		i18n(translator.name().toUtf8()),
		translator.emailAddress(),
		i18n(website.toUtf8()),
		i18n(translator.task().toUtf8())
	    );
	}
	translatorsframe->setWidget(trans);
	translatorsframe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    /* the frame containing the license(s) */
    licenseframe->setReadOnly(true);
    QString licenses;
    foreach (const KAboutLicense &license, about_data.licenses()) {
	licenses += license.text();
    }
    licenseframe->setText(licenses);

    // set the focus onto the "OK" button
    buttonBox->setFocus();
}

//***************************************************************************
Kwave::AboutDialog::~AboutDialog()
{
}

//***************************************************************************
//***************************************************************************
