/***************************************************************************
          LabelList.cpp  -  list of labels
                             -------------------
    begin                : Sat Aug 05 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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
#include "LabelList.h"

//***************************************************************************
LabelList::LabelList()
    :QList<Label *>()
{
}

//***************************************************************************
LabelList::~LabelList()
{
    clear();
}

/***************************************************************************/
bool LabelList::equals(const LabelList &other) const
{
    // shortcut: both empty ?
    if (isEmpty() && other.isEmpty()) return true;

    // quick check: does the number of elements match?
    if (count() != other.count()) return false;

    LabelListIterator it_mine(*this);
    LabelListIterator it_other(other);

    while (it_mine.hasNext() && it_other.hasNext()) {
	Label *mine  = it_mine.next();
	Label *other = it_other.next();

	if (mine->pos()  != other->pos())  return false;
	if (mine->name() != other->name()) return false;
    }

    return true;
}

//***************************************************************************
void LabelList::clear()
{
    qDeleteAll(*this);
    QList<Label *>::clear();
}

/***************************************************************************/
void LabelList::copy(const LabelList &source)
{
    // throw away any old stuff
    clear();

    // always make a deep copy, copy all elements
    foreach (Label *label, source) {
	Q_ASSERT(label);
	Label *copy  = new Label(*label);
	Q_ASSERT(copy);
	if (copy) append(copy);
    }
}

//***************************************************************************
//***************************************************************************
