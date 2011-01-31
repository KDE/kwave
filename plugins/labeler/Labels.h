/***************************************************************************
             Labels.h    - representation of a Labeler plugin labels
                           -------------------
    begin                : Mon Oct 11 2010
    copyright            : (C) 2010 by Daniel Tihelka
    email                : dtihelka@kky.zcu.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _LABELS_H_
#define _LABELS_H_


#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"



/**
 * The type of all Labeler plugin labels. For simple use in
 * Kwave::MetaDataList::selectByType(QString)
 */
class KDE_EXPORT LabelerLabel
{
public:

    /** The identifier of the "type" of this meta data object */
    static const QString METADATA_TYPE;

    /** The name of property identifying individual types of the label.
     *  Each label returns unique value */
    static const QString LABELPROP_TYPE;
};

/**
 * Kwave::MetaData decorator class representing single signal instant label.
 * The class defines several new methods simplifying the access to signal
 * instant label-related properties.
 *
 * The class can be identified by LabelInstant::LABEL_TYPE value of
 * LabelerLabel::LABELPROP_TYPE property
 */
class KDE_EXPORT LabelInstant: public Kwave::MetaData, LabelerLabel
{
public:

    /** Returns the value of property identifying this type of label. */
    static const QVariant LABEL_TYPE;


    /** Default constructor, creates an empty label */
    LabelInstant();

    /**
     * Constructor
     *
     * @param position the label position [samples]
     * @param name the name of the label, user defined
     */
    LabelInstant(sample_index_t position, const QString &name);
    /**
     * Copy-Constructor. It copies all the data from the parent
     * <b>DO NOT FORGET move the changes to parent, if this class would change!</b>
     *
     * @param parent the instance of LabelInstant::LABEL_TYPE to copy data from
     * @todo  it would be much better just to store pointer. But in this case,
     *        this class will need to re-implement all the methods from Kwave::MetaData
     *        I would suggest to create pure virtual class which would define the define
     *        the interface of MetaData. This would ensure that all methods will be
     *        implemented.
     */
    LabelInstant(Kwave::MetaData & metadata);

    /** Destructor */
    virtual ~LabelInstant();

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

    /** returns the name of the label */
    virtual QString name() const;

    /** less-than operator, needed for sorting the list */
    inline bool operator < (const LabelInstant & other) const {
        return (pos() < other.pos());
    }

    /** equal operator */
    inline bool operator == (const LabelInstant & other) const {
        return ((pos() == other.pos()) && (name() == other.name()));
    }

};


/**
 * Signal region label.
 * The class can be identified by LabelRegion::LABEL_TYPE value of
 * LabelerLabel::LABELPROP_TYPE property
 */
class KDE_EXPORT LabelRegion: public Kwave::MetaData, LabelerLabel
{
public:

    /** Returns the value of property identifying this type of label. */
    static const QVariant LABEL_TYPE;


    /**
     * Constructor
     *
     * @param beg the region beginning [samples]
     * @param end the region end [samples]
     * @param name the name of the label, user defined
     */
    LabelRegion(sample_index_t beg, sample_index_t end, const QString &name);
    /**
     * Copy-Constructor. It copies all the data from the parent.
     * <b>DO NOT FORGET move the changes to parent, if this class would change!</b>
     *
     * @param parent the instance of LabelRegion::LABEL_TYPE to copy data from
     */
    LabelRegion(Kwave::MetaData & metadata);

    /** Destructor */
    virtual ~LabelRegion();


    /** Returns the label's beginning position [samples] */
    // TODO: there is method MetaData::first_sample
    virtual sample_index_t beg() const;
    /** Returns the label's end position [samples] */
    // TODO: there is method MetaData::last_sample
    virtual sample_index_t end() const;

    /**
     * Change the name of the label
     * @param name the new name, user defined
     */
    virtual void rename(const QString &name);

    /** Returns the name of the label */
    virtual QString name() const;


    // other methods here ...

    /** less-than operator, needed for sorting the list */
    inline bool operator < (const LabelRegion & other) const {
        return (beg() < other.end());
    }

    /** equal operator */
    inline bool operator == (const LabelRegion & other) const {
        return ((beg() == other.beg()) && (end() == other.end()) && (name() == other.name()));
    }

};


#endif /* _LABELS_H_ */
