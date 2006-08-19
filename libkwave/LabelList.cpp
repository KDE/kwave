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
