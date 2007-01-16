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

#include "LabelList.h"

//***************************************************************************
LabelList::LabelList()
    :QPtrList<Label>()
{
    setAutoDelete(true);
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

    for (; it_mine.current() && it_other.current(); ++it_mine, ++it_other) {
    	Label *mine  = it_mine.current();
    	Label *other = it_other.current();

    	if (mine->pos()  != other->pos())  return false;
    	if (mine->name() != other->name()) return false;
    }

    return true;
}

/***************************************************************************/
void LabelList::copy(const LabelList &source)
{
    // throw away any old stuff
    clear();

    // always make a deep copy, copy all elements
    LabelListIterator it(source);
    for (; it.current(); ++it) {
    	Label *label = it.current();
    	Q_ASSERT(label);
	Label *copy  = new Label(*label);
	Q_ASSERT(copy);
	if (copy) append(copy);
    }
}

//***************************************************************************
int LabelList::compareItems(QPtrCollection::Item a, QPtrCollection::Item b)
{
    Q_ASSERT(a);
    Q_ASSERT(b);
    if (!a || !b) return -1; // not allowed!
    if (a == b) return 0;    // simple case: compare with itself

    Label *label_a = reinterpret_cast<Label *>(a);
    Label *label_b = reinterpret_cast<Label *>(b);

    if (label_a->pos() == label_b->pos()) return 0;
    return (label_a->pos() < label_b->pos()) ? -1 : +1;
}

//***************************************************************************
//***************************************************************************
