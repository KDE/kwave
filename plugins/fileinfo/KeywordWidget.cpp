/***************************************************************************
      KeywordWidget.cpp  -  widget for editing a list of keywords
			     -------------------
    begin                : Fri 02 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klineedit.h>
#include <klistbox.h>
#include <kpushbutton.h>

#include "KeywordWidget.h"

/***************************************************************************/
KeywordWidget::KeywordWidget(QWidget *parent, const char *name)
    :KeywordWidgetBase(parent, name)
{
    ASSERT(edKeyword);
    ASSERT(btAdd);
    ASSERT(btAuto);
    ASSERT(btRemove);
    ASSERT(lstKeywords);

    connect(edKeyword, SIGNAL(textChanged(const QString &)),
            this, SLOT(editChanged(const QString &)));
    connect(btAdd, SIGNAL(clicked()),
            this, SLOT(add()));
    connect(btAuto, SIGNAL(clicked()),
            this, SLOT(autoClicked()));
    connect(btRemove, SIGNAL(clicked()),
            this, SLOT(remove()));
    connect(lstKeywords, SIGNAL(highlighted(const QString &)),
            this, SLOT(selected(const QString &)));
    connect(lstKeywords, SIGNAL(selected(const QString &)),
            this, SLOT(selected(const QString &)));
    connect(lstKeywords, SIGNAL(clicked(QListBoxItem *)),
            this, SLOT(listClicked(QListBoxItem *)));

    // if the user presses return in the edit control, this means
    // the same as clicking on the "Add" button
    edKeyword->setTrapReturnKey(true);
    connect(edKeyword, SIGNAL(returnPressed(const QString &)),
            this, SLOT(returnPressed(const QString &)));

    update();
}

/***************************************************************************/
KeywordWidget::~KeywordWidget()
{
}

/***************************************************************************/
bool KeywordWidget::contained(const QString &item)
{
    if (!item.length()) return false;
    return (lstKeywords->findItem(item, Qt::ExactMatch));
}

/***************************************************************************/
QStringList KeywordWidget::keywords()
{
    QStringList list;
    unsigned int count = lstKeywords->count();
    for (unsigned int index=0; index < count; ++index) {
	list.append(lstKeywords->text(index));
    }
    return list;
}

/***************************************************************************/
void KeywordWidget::setKeywords(const QStringList &keywords)
{
    lstKeywords->clear();
    edKeyword->clear();
    unsigned int count = keywords.count();
    for (unsigned int index = 0; index < count; ++index) {
	QString item = keywords[index].simplifyWhiteSpace();
	if (contained(item)) continue; // skip duplicate
	lstKeywords->insertItem(item);
    }
    lstKeywords->sort();

    edKeyword->clear();
    update();
    edKeyword->clear();
}

/***************************************************************************/
void KeywordWidget::update()
{
    QString edit = edKeyword->text().simplifyWhiteSpace();

    // "Add" is only allowed if the current edit space is not empty and
    // the entered text is not already in the list
    btAdd->setEnabled(edit.length() && !contained(edit));

    // "Remove" is only enabled if something out of the list has been selected
    btRemove->setEnabled((lstKeywords->currentItem() >= 0) &&
                         (contained(edit) || !edit.length()));

    // the list is only enabled if it is not empty
    lstKeywords->setEnabled(lstKeywords->count() != 0);

    // the current item should always be visible
    lstKeywords->ensureCurrentVisible();
}

/***************************************************************************/
void KeywordWidget::editChanged(const QString &edit)
{
    QString text = edit.simplifyWhiteSpace();
    int index = lstKeywords->index(lstKeywords->findItem(
                                   text, Qt::BeginsWith));
    if (index >= 0) {
	lstKeywords->setCurrentItem(index);
	update();
	edKeyword->setText(edit);
    } else {
	update();
    }
}

/***************************************************************************/
void KeywordWidget::returnPressed(const QString &)
{
    add(); // means the same as pressing "Add"
}

/***************************************************************************/
void KeywordWidget::add()
{
    QString text = edKeyword->text().simplifyWhiteSpace();
    if (!text.length()) return;
    if (contained(text)) return;

    // insert the current edit text and sort the list
    lstKeywords->insertItem(text);
    lstKeywords->sort();

    // find the new item again and make it the current selection
    int index = lstKeywords->index(lstKeywords->findItem(text,
                                   Qt::ExactMatch));
    lstKeywords->setCurrentItem(index);
    edKeyword->clear();

    // now we do no longer need the edit
    update();
    edKeyword->clear();
}

/***************************************************************************/
void KeywordWidget::remove()
{
    // remove the item from the list
    int index = lstKeywords->currentItem();
    lstKeywords->removeItem(index);
    edKeyword->clear();

    // set the previous item as current
    if (index) --index;
    lstKeywords->setSelected(index, true);

    edKeyword->clear();
    update();
}

/***************************************************************************/
void KeywordWidget::selected(const QString &selection)
{
    edKeyword->setText(selection);
    update();
}

/***************************************************************************/
void KeywordWidget::listClicked(QListBoxItem *item)
{
    if (!item) return;
    selected(lstKeywords->text(lstKeywords->index(item)));
}

/***************************************************************************/
void KeywordWidget::autoClicked()
{
    emit autoGenerate();
}

/***************************************************************************/
/***************************************************************************/
