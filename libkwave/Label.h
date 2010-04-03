/***************************************************************************
                Label.h  -  representation of a label within a signal
                             -------------------
    begin                : Mon Jul 31 2006
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
#ifndef _LABEL_H_
#define _LABEL_H_

#include "config.h"

#include <QString>

#include <kdemacros.h>

#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"

class KDE_EXPORT Label: public Kwave::MetaData
{
public:

    /** Default constructor, creates an empty label */
    Label();

    /**
     * Constructor
     *
     * @param position the label position [samples]
     * @param name the name of the label, user defined
     */
    Label(sample_index_t position, const QString &name);

    /** destructor */
    virtual ~Label();

    /**
     * Set a new position of the label
     * @param position the new position [samples]
     */
    virtual void moveTo(sample_index_t position);

    /** Returns the label's position [samples] */
    virtual sample_index_t pos() const;

    /**
     * change the name of the label
     * @param name the new name, user defined
     */
    virtual void rename(const QString &name);

    /** returns the name of the string */
    virtual QString name() const;

    /** less-than operator, needed for sorting the list */
    inline bool operator < (const Label &other) const {
	return (pos() < other.pos());
    }

    /** equal operator */
    inline bool operator == (const Label &other) const {
	return ((pos() == other.pos()) && (name() == other.name()));
    }

};

#endif /* _LABEL_H_ */
