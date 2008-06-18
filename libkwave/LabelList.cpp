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

#include <QtAlgorithms>
#include "libkwave/LabelList.h"

//***************************************************************************
static bool compare_labels(Label *a, Label *b)
{
    Q_ASSERT(a);
    Q_ASSERT(b);
    return ((a) && (b) && (*a < *b));
}

//***************************************************************************
void LabelList::sort()
{
    qSort(begin(), end(), compare_labels);
}

//***************************************************************************
//***************************************************************************
