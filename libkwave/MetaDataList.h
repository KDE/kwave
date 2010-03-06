/***************************************************************************
         MetaDataList.h  -  list with meta data objects
                             -------------------
    begin                : Sat Mar 06 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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
#ifndef _META_DATA_LIST_H_
#define _META_DATA_LIST_H_

#include "config.h"

#include <QMap>
#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"

namespace Kwave {

    class KDE_EXPORT MetaDataList: public QMap<QString, MetaData>
    {
    public:

	/** Iterator for the meta data list */
	typedef QMapIterator<QString, MetaData> Iterator;

	/** Default constructor */
	MetaDataList();

	/** Destructor */
	virtual ~MetaDataList();

	/**
	 * select elements from the meta data list that have a specific scope
	 *
	 * @param scope the scope to select @see Kwave::MetaData::Scope
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByScope(MetaData::Scope scope) const;

	/**
	 * select elements from the meta data list that belong to
	 * a given track.
	 *
	 * @param track index of the track to select
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByTrack(unsigned int track) const;

	/**
	 * select elements from the meta data list that overlap a given
	 * range of samples (selects elements with scope "Range" as well
	 * as elements with scope "Position")
	 *
	 * @param first index of the first sample
	 * @param last index of the last sample
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByRange(
	    sample_index_t first, sample_index_t last) const;

	/**
	 * select elements from the meta data list that are exactly at a
	 * given position.
	 *
	 * @param first index of the first sample
	 * @param last index of the last sample
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByPosition(sample_index_t pos) const;

	/**
	 * select elements from the meta data list that contain a given
	 * property.
	 *
	 * @param property the property to search for
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByProperty(const QString &property) const;

	/**
	 * select elements from the meta data list that contain a given
	 * property and a given value
	 *
	 * @param property the property to search for
	 * @param value the value that the property should have
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByValue(
	    const QString &property, QVariant value) const;

	/**
	 * Checks whether a meta data object is contained in this list,
	 * the check is based on the unique id of the meta data object.
	 *
	 * @param metadata the object to search
	 * @return true if found, otherwise false
	 */
	virtual bool contains(const MetaData &metadata) const;

	/**
	 * Adds a single meta data object to the list. If it is already
	 * present, the old version will be silently replaced.
	 *
	 * @param metadata the object that should be added
	 */
	virtual void add(const MetaData &metadata);


	/**
	 * Adds a list of meta data objects to the list. Old versions of
	 * existing objects will be silently replaced.
	 *
	 * @param list the object that should be added
	 */
	virtual void add(const MetaDataList &list);

	/**
	 * Removes one meta data object from the list (if it exists).
	 *
	 * @param metadata the object that should be removed
	 */
	virtual void remove(const MetaData &metadata);

	/**
	 * Removes a list of meta data objects from this list (if they exist).
	 *
	 * @param list the list of meta data objects to remove
	 */
	virtual void remove(const MetaDataList &list);

    };

}

#endif /* _META_DATA_LIST_H_ */
