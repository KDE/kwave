/***************************************************************************
     AboutContainer.cpp  -  Authors and thanks field in the about dialog
                              -------------------
    begin                : Sat Dec 29 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    based on class K3AboutContainer
    copied from k3aboutdialog.cpp / kdelibs-3.97.0

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

#include "config.h"

#include <new>

#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>

#include <KLocalizedString>
#include <KHelpClient>

#include "libkwave/String.h"

#include "AboutContainer.h"

//***************************************************************************
Kwave::AboutContainer::AboutContainer(QWidget *parent)
    :QFrame(parent)
{
    setFrameStyle(QFrame::NoFrame);

    QGridLayout* const gbox = new(std::nothrow) QGridLayout(this);
    Q_ASSERT(gbox);
    if (!gbox) return;

    gbox->setContentsMargins(0, 0, 0, 0);
    gbox->setColumnStretch(0, 10);
    gbox->setColumnStretch(2, 10);
    gbox->setRowStretch(0, 10);
    gbox->setRowStretch(2, 10);

    m_vbox = new(std::nothrow) QVBoxLayout();
    Q_ASSERT(m_vbox);
    if (!m_vbox) return;

    m_vbox->setSpacing(0);
    gbox->addLayout(m_vbox, 1, 1);
//     gbox->activate();
}

//***************************************************************************
Kwave::AboutContainer::~AboutContainer()
{
}

//***************************************************************************
QSize Kwave::AboutContainer::sizeHint() const
{
    //
    // The size is computed by adding the sizeHint().height() of all
    // widget children and taking the width of the widest child and adding
    // layout()->margin() and layout()->spacing()
    //

    QSize total_size;

    int numChild = 0;
    const QList<QObject*> l = children(); // silence please
    foreach (QObject *o, l) {
	if (o->isWidgetType()) {
	    ++numChild;
	    QWidget * const w = static_cast<QWidget *>(o);

	    QSize s = w->minimumSize();
	    if (s.isEmpty()) {
		s = w->minimumSizeHint();
		if (s.isEmpty()) {
		    s = w->sizeHint();
		    if (s.isEmpty())
			s = QSize(100, 100); // Default size
		}
	    }
	    total_size.setHeight(total_size.height() + s.height());
	    if (s.width() > total_size.width()) {
		total_size.setWidth(s.width());
	    }
	}
    }

    if (numChild > 0) {
	//
	// Seems I have to add 1 to the height to properly show the border
	// of the last entry if layout()->margin() is 0
	//
	total_size.setHeight(total_size.height() +
	    layout()->spacing() * (numChild - 1));
	total_size += QSize(layout()->contentsMargins().left()*2, layout()->contentsMargins().top()*2 + 1);
    } else {
	total_size = QSize(1, 1);
    }
    return total_size;
}

//***************************************************************************
QSize Kwave::AboutContainer::minimumSizeHint() const
{
    return sizeHint();
}

//***************************************************************************
void Kwave::AboutContainer::addWidget(QWidget *widget)
{
    widget->setParent(this);

    m_vbox->addWidget(widget, 0, Qt::AlignCenter);
    const QSize s(sizeHint());
    setMinimumSize(s);

    const QList<QObject *> l = children(); // silence please
    foreach (QObject *o, l) {
	if (o->isWidgetType())
	    static_cast<QWidget *>(o)->setMinimumWidth(s.width());
    }
}

//***************************************************************************
void Kwave::AboutContainer::addPerson(const QString &_name, const QString &_email,
				      const QString &_url, const QString &_task)
{
    Kwave::AboutContributor * const cont = new(std::nothrow)
	Kwave::AboutContributor(this,
	    _name, _email, _url, _task
	);
    Q_ASSERT(cont);
    if (!cont) return;

    addWidget(cont);
}

//***************************************************************************
Kwave::AboutContributor::AboutContributor(QWidget *_parent,
                                          const QString &_name,
                                          const QString &_email,
                                          const QString &_url,
                                          const QString &_work)
    :QFrame(_parent)
{
    for (int i=0; i < 4; ++i) {
	m_text[i] = new(std::nothrow) QLabel(this);
	Q_ASSERT(m_text[i]);
	if (!m_text[i]) return;
	m_text[i]->setOpenExternalLinks(true);
	m_text[i]->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    }

    // set name
    m_text[0]->setText(_name);

    // set email
    if (!_email.isEmpty())
	m_text[1]->setText(_("<a href=\"mailto:%1\">%1</a>").arg(_email));

    // set url
    if (!_url.isEmpty())
	m_text[2]->setText(_("<a href=\"%1\">%1</a>").arg(_url));

    // set work
    m_text[3]->setText(_work);

    fontChange(font());
    updateLayout();
}

//***************************************************************************
Kwave::AboutContributor::~AboutContributor()
{
}

//***************************************************************************
void Kwave::AboutContributor::fontChange(const QFont &/*oldFont*/)
{
    update();
}

//***************************************************************************
QSize Kwave::AboutContributor::sizeHint() const
{
    return minimumSizeHint();
}

//***************************************************************************
void Kwave::AboutContributor::updateLayout()
{
    if (layout()) delete layout();

    int row = 0;
    if (!m_text[0] || !m_text[0]->text().isEmpty()) { ++row; }
    if (!m_text[1] || !m_text[1]->text().isEmpty()) { ++row; }
    if (!m_text[2] || !m_text[2]->text().isEmpty()) { ++row; }
    if (!m_text[3] || !m_text[3]->text().isEmpty()) { ++row; }

    QGridLayout *gbox = Q_NULLPTR;
    if (row == 0) {
	gbox = new(std::nothrow) QGridLayout(this);
	Q_ASSERT(gbox);
	if (!gbox) return;
	gbox->setSpacing(1);
	for (int i=0; i<4; ++i)
	    if (m_text[i]) m_text[i]->hide();
    } else {
	if (m_text[0] && m_text[0]->text().isEmpty()) {
	    gbox = new(std::nothrow) QGridLayout(this);
	    Q_ASSERT(gbox);
	    if (!gbox) return;
	    gbox->setContentsMargins(frameWidth()+1, frameWidth()+1, frameWidth()+1, frameWidth()+1);
	    gbox->setSpacing(2);
	} else {
	    gbox = new(std::nothrow) QGridLayout(this);
	    Q_ASSERT(gbox);
	    if (!gbox) return;
	    gbox->setContentsMargins(frameWidth()+1, frameWidth()+1, frameWidth()+1, frameWidth()+1);
	    gbox->setSpacing(2);
	    gbox->addItem(new(std::nothrow) QSpacerItem(20, 0), 0, 0);
	    gbox->setColumnStretch(1, 10);
	}

	for (int i = 0, r = 0; i < 4; ++i) {
	    if (!m_text[i]) continue;

	    if (i != 3) {
		m_text[i]->setFixedHeight(fontMetrics().lineSpacing());
	    }

	    if (!m_text[i]->text().isEmpty()) {
		if (!i) {
		    gbox->addWidget(m_text[i], r, 0, 1, 2, Qt::AlignLeft);
		} else {
		    gbox->addWidget(m_text[i], r, 1, Qt::AlignLeft );
		}
		m_text[i]->show();
		++r;
	    } else {
		m_text[i]->hide();
	    }
	}
    }

    gbox->activate();
    setMinimumSize(sizeHint());
}

//***************************************************************************
//***************************************************************************
