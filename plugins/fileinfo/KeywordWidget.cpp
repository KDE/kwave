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

#include <config.h>

#include <klineedit.h>
#include <klistwidget.h>
#include <kpushbutton.h>

#include "KeywordWidget.h"

//***************************************************************************
Kwave::KeywordWidget::KeywordWidget(QWidget *parent)
    :QWidget(parent), Ui::KeywordWidgetBase()
{
    setupUi(this);

    Q_ASSERT(edKeyword);
    Q_ASSERT(btAdd);
    Q_ASSERT(btAuto);
    Q_ASSERT(btRemove);
    Q_ASSERT(lstKeywords);

    connect(edKeyword, SIGNAL(textChanged(const QString &)),
            this, SLOT(editChanged(const QString &)));
    connect(btAdd, SIGNAL(clicked()),
            this, SLOT(add()));
    connect(btAuto, SIGNAL(clicked()),
            this, SLOT(autoClicked()));
    connect(btRemove, SIGNAL(clicked()),
            this, SLOT(remove()));
    connect(lstKeywords, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(listClicked(QListWidgetItem *)));
    connect(lstKeywords, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(listClicked(QListWidgetItem *)));
    connect(lstKeywords, SIGNAL(executed(QListWidgetItem *)),
            this, SLOT(listClicked(QListWidgetItem *)));

    // if the user presses return in the edit control, this means
    // the same as clicking on the "Add" button
    edKeyword->setTrapReturnKey(true);
    connect(edKeyword, SIGNAL(returnPressed(const QString &)),
            this, SLOT(returnPressed(const QString &)));

    update();
}

//***************************************************************************
Kwave::KeywordWidget::~KeywordWidget()
{
}

//***************************************************************************
bool Kwave::KeywordWidget::contained(const QString &item)
{
    if (!item.length()) return false;
    return (!lstKeywords->findItems(item, Qt::MatchExactly).isEmpty());
}

//***************************************************************************
QStringList Kwave::KeywordWidget::keywords()
{
    QStringList list;
    unsigned int count = lstKeywords->count();
    for (unsigned int index=0; index < count; ++index) {
	QListWidgetItem *item = lstKeywords->item(index);
	if (item && item->text().length())
	    list.append(item->text());
    }
    return list;
}

//***************************************************************************
void Kwave::KeywordWidget::setKeywords(const QStringList &keywords)
{
    lstKeywords->clear();
    edKeyword->clear();

    foreach (QString it, keywords) {
	QString item = it.simplified();
	if (contained(item)) continue; // skip duplicate
	lstKeywords->addItem(item);
    }
    lstKeywords->setSortingEnabled(true);
    lstKeywords->sortItems();

    edKeyword->clear();
    update();
    edKeyword->clear();
}

//***************************************************************************
void Kwave::KeywordWidget::update()
{
    QString edit = edKeyword->text().simplified();

    // "Add" is only allowed if the current edit space is not empty and
    // the entered text is not already in the list
    btAdd->setEnabled(edit.length() && !contained(edit));

    // "Remove" is only enabled if something out of the list has been selected
    btRemove->setEnabled((lstKeywords->currentItem() != 0) &&
                         (contained(edit) || !edit.length()));

    // the list is only enabled if it is not empty
    lstKeywords->setEnabled(lstKeywords->count() != 0);

    // the current item should always be visible
    lstKeywords->scrollToItem(lstKeywords->currentItem(),
	                      QAbstractItemView::EnsureVisible);
}

//***************************************************************************
void Kwave::KeywordWidget::editChanged(const QString &edit)
{
    QString text = edit.simplified();
    QList<QListWidgetItem *> matches =
	lstKeywords->findItems(text, Qt::MatchStartsWith);
    if (edit.length() && !matches.isEmpty()) {
	lstKeywords->setCurrentItem(matches.takeFirst());
	update();
	edKeyword->setText(edit);
    } else {
	update();
    }
}

//***************************************************************************
void Kwave::KeywordWidget::returnPressed(const QString &)
{
    add(); // means the same as pressing "Add"
}

//***************************************************************************
void Kwave::KeywordWidget::add()
{
    QString text = edKeyword->text().simplified();
    if (!text.length()) return;
    if (contained(text)) return;

    // insert the current edit text and sort the list
    lstKeywords->addItem(text);
    lstKeywords->sortItems();

    // find the new item again and make it the current selection
    QList<QListWidgetItem *> matches =
	lstKeywords->findItems(text, Qt::MatchStartsWith);
    if (!matches.isEmpty())
	lstKeywords->setCurrentItem(matches.takeFirst());
    edKeyword->clear();

    // now we do no longer need the edit
    update();
    edKeyword->clear();
}

//***************************************************************************
void Kwave::KeywordWidget::remove()
{
    // remove the item from the list
    int index = lstKeywords->currentRow();
    QListWidgetItem *item = lstKeywords->takeItem(index);
    if (item) delete item;
    edKeyword->clear();

    // set the previous item as current
    if (index) --index;
    if (lstKeywords->item(index))
	lstKeywords->item(index)->setSelected(true);

    edKeyword->clear();
    update();
}

//***************************************************************************
void Kwave::KeywordWidget::listClicked(QListWidgetItem *item)
{
    if (!item) return;
    edKeyword->setText(item->text());
    update();
}

//***************************************************************************
void Kwave::KeywordWidget::autoClicked()
{
    emit autoGenerate();
}

//***************************************************************************
#include "KeywordWidget.moc"
//***************************************************************************
//***************************************************************************
