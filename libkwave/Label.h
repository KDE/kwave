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
#include <QSharedData>
#include <QSharedDataPointer>

#include <kdemacros.h>

class KDE_EXPORT Label
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

    /** returns true if this is an empty label */
    virtual bool isNull() const;

    /** less-than operator, needed for sorting the list */
    inline bool operator < (const Label &other) const {
	return (pos() < other.pos());
    }

    /** equal operator */
    inline bool operator == (const Label &other) const {
	return ((pos() == other.pos()) && (name() == other.name()));
    }

    /** equal operator, compares with pointer to Label */
    inline bool operator == (const Label *other) const {
	// both Null -> equal
	if (isNull() && !other) return true;

	// only one is null -> not equal
	if (isNull() || !other) return false;

	// otherwise -> compare...
	return ((pos() == other->pos()) && (name() == other->name()));
    }

private:

    class LabelData: public QSharedData {
    public:

	LabelData();

	LabelData(const LabelData &other);

	virtual ~LabelData();

	/** position of the label [samples] */
	unsigned int m_position;

	/** name of the label, user defined */
	QString m_name;
    };

    QSharedDataPointer<LabelData> m_data;

};

#endif /* _LABEL_H_ */
