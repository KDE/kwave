/***************************************************************************
            LabelList.h  -  list of labels
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
#ifndef _LABEL_LIST_H_
#define _LABEL_LIST_H_

#include "config.h"

#include <QList>
#include <QListIterator>

#include <kdemacros.h>

#include "Label.h"

/** not more than a typedef since Qt4 */
class KDE_EXPORT LabelList: public QList<Label *>
{
public:

    /** sorts the list by ascending position */
    void sort();

};

/** Iterator for the list of labels */
typedef QListIterator<Label *> LabelListIterator;

#endif /* _LABEL_LIST_H_ */
