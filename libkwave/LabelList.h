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

#include <config.h>
#include <qptrlist.h>

#include "Label.h"

class LabelList: public QPtrList<Label>
{
public:
    /** Constructor */
    LabelList();

    /** Destructor */
    virtual ~LabelList();

protected:
    /**
     * compare two labels, for sorting
     * @see QPtrList::compareItems
     */
    int compareItems(Label *a, Label *b);
};

#endif /* _LABEL_LIST_H_ */
