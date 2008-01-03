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
#include "Label.h"

class LabelList: public QList<Label *>
{
public:
    /** Constructor */
    LabelList();

    /** Destructor */
    virtual ~LabelList();

    /** Copy everything from another LabelList */
    void copy(const LabelList &source);

    /** Return true if the list is equal to another list */
    bool equals(const LabelList &other) const;

    /** Erase all elements and set size to zero */
    virtual void clear();

    /** Assignment operator */
    inline LabelList & operator = (const LabelList &source) {
	copy(source);
	return *this;
    };

    /** Compare operator */
    inline bool operator == (const LabelList &other) const {
	return equals(other);
    };

};

/** Iterator for the list of labels */
typedef QListIterator<Label *> LabelListIterator;

#endif /* _LABEL_LIST_H_ */
