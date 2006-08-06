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
#include <qstring.h>

class Label
{
public:
    /**
     * Constructor
     *
     * @param position the label position [samples]
     * @param name the name of the label, user defined
     */
    Label(unsigned int position, const QString &name);

    /** destructor */
    virtual ~Label();

    /**
     * Set a new position of the label
     * @param pos the new position [samples]
     */
    virtual void moveTo(unsigned int position);

    /** Returns the label's position [samples] */
    virtual unsigned int pos() const;

    /**
     * change the name of the label
     * @param name the new name, user defined
     */
    virtual void rename(const QString &name);

    /** returns the name of the string */
    virtual QString name() const;

private:

    /** position of the label [samples] */
    unsigned int m_position;

    /** name of the label, user defined */
    QString m_name;
};

#endif /* _LABEL_H_ */
