/***************************************************************************
       AboutContainer.h  -  Authors and thanks field in the about dialog
                             -------------------
    begin                : Sat Dec 29 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    based on class K3AboutContainer
    copied from k3aboutdialog.h / kdelibs-3.97.0

    Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
                            Espen Sand (espen@kde.org)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KWAVE_ABOUT_CONTAINER_H_
#define _KWAVE_ABOUT_CONTAINER_H_

#include "config.h"

#include <QtGui/QFrame>
#include <QtCore/QSize>
#include <QtGui/QVBoxLayout>

class QString;
class QLabel;
class QWidget;

namespace Kwave
{
    /**
    * simplified clone of K3AboutContainer
    * @see K3AboutContainer
    */
    class AboutContainer: public QFrame
    {
    Q_OBJECT

    public:
	explicit AboutContainer(QWidget *parent = 0);

	virtual ~AboutContainer();

	void addPerson(const QString &name, const QString &email,
		    const QString &url, const QString &task);

	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;

    protected:

	void addWidget(QWidget *widget);

    private:

	QVBoxLayout *m_vbox;
    };

    /**
     * Used internally by KwaveAboutWidget
     * @see K3AboutContributor
     * @internal
     */
    class AboutContributor: public QFrame
    {
    Q_OBJECT

    public:
	AboutContributor(QWidget *parent,
	                 const QString &username,
	                 const QString &email,
	                 const QString &url,
	                 const QString &work);

	virtual ~AboutContributor();

	virtual QSize sizeHint() const;

    protected:
	virtual void fontChange( const QFont &oldFont );

	virtual void updateLayout();

    private:

	QLabel *m_text[4];

    };
}

#endif /* _KWAVE_ABOUT_CONTAINER_H_ */

//***************************************************************************
//***************************************************************************
