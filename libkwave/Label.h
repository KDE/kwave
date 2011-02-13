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

#include "libkwave/Sample.h"

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

private:

    /** internal container class with label data */
    class LabelData: public QSharedData {
    public:

	/** constructor */
	LabelData();

	/** copy constructor */
	LabelData(const LabelData &other);

	/** destructor */
	virtual ~LabelData();

	/** position of the label [samples] */
	sample_index_t m_position;

	/** name of the label, user defined */
	QString m_name;
    };

    /** pointer to the shared label data */
    QSharedDataPointer<LabelData> m_data;

};

#endif /* _LABEL_H_ */
