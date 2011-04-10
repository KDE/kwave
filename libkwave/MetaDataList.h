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

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QMapIterator>
#include <QMutableMapIterator>

#include <kdemacros.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"

namespace Kwave {

    class KDE_EXPORT MetaDataList: public QMap<QString, MetaData>
    {
    public:

	/** const iterator for the meta data list */
	typedef QMapIterator<QString, MetaData> Iterator;

	/** mutable iterator for the meta data list */
	typedef QMutableMapIterator<QString, MetaData> MutableIterator;

	/** Default constructor */
	MetaDataList();
	
	/**
	 * Constructor, creates a meta data list with only one single meta 
	 * data item. In some cases you need to pass a meta data list to a
	 * function but you have only a single meta data item, so this might 
	 * be quite handy
	 * @param meta const reference to a single meta data item
	 */
	MetaDataList(const MetaData &meta);

	/** Destructor */
	virtual ~MetaDataList();

	/**
	 * select elements from the meta data list that have the standard
	 * property STDPROP_TYPE set to a specific value.
	 *
	 * @param type the type to select @see Kwave::MetaData::STDPROP_TYPE
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByType(const QString &type) const;

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
	 * @param tracks list of track indices to select
	 * @return list with found meta data objects
	 */
	virtual MetaDataList selectByTracks(const QList<unsigned int> &tracks) const;

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
	 * Replaces all meta data objects that have the same type as one
	 * of the objects in the passed list with newer versions. If an
	 * object did not exist, it will be created. If an object is not
	 * in the passed list, it will be deleted.
	 * 
	 * @param list listof meta data objects that should be replaced
	 * @note affects only objects with a type that was found in the
	 *       passed list
	 */
	virtual void replace(const MetaDataList &list);

	/**
	 * Adds a single meta data object to the list. If it is already
	 * present, the old version will be silently replaced.
	 *
	 * @param metadata the meta data object that should be added
	 */
	virtual void add(const MetaData &metadata);

	/**
	 * Adds a list of meta data objects to the list. Old versions of
	 * existing objects will be silently replaced.
	 *
	 * @param list list of meta data objects that should be added
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

	/**
	 * Crops this list to a given range of samples. All position aware
	 * elements that are not covered by the given range will be removed,
	 * all covered elements will be adjusted.
	 *
	 * @param first index of the first sample
	 * @param last  index of the last sample
	 */
	virtual void cropByRange(sample_index_t first, sample_index_t last);

	/**
	 * Crops this list to a given set of tracks. All elements that are
	 * bound to a track or list of tracks which are not covered by the
	 * given selection will be removed. The tracks of the remaining
	 * elements will be re-numbered to start from zero and counted up
	 * without gaps.
	 *
	 * @param tracks list of track indices
	 */
	virtual void cropByTracks(const QList<unsigned int> &tracks);

	/**
	 * copy elements from the meta data list that overlap a given
	 * range of samples (selects elements with scope "Range" as well
	 * as elements with scope "Position") and have a binding to a
	 * track.
	 *
	 * @param offset index of the first sample
	 * @param length number of samples of the range
	 * @param tracks list of track indices
	 * @return list with a copy of found meta data objects
	 */
	virtual MetaDataList copy(
	    sample_index_t offset,
	    sample_index_t length,
	    const QList<unsigned int> &tracks
	) const;

	/**
	 * Merges a list of other meta data items
	 * @param meta_data list of meta data items
	 */
	void merge(const MetaDataList &meta_data);

	/**
	 * delete elements from the meta data list that overlap a given
	 * range of samples (selects elements with scope "Range" as well
	 * as elements with scope "Position") and have a binding to a
	 * track.
	 *
	 * @param offset index of the first sample
	 * @param length number of samples to delete
	 * @param tracks list of track indices
	 * @return list with a copy of found meta data objects
	 */
	virtual void deleteRange(
	    sample_index_t offset,
	    sample_index_t length,
	    const QList<unsigned int> &tracks
	);

	/**
	 * shift the positions or start/end of all elements that are after
	 * a given offset to the left.
	 *
	 * @param offset index of the first sample
	 * @param shift number of samples to shift left
	 * @param tracks list of track indices
	 */
	virtual void shiftLeft(
	    sample_index_t offset,
	    sample_index_t shift,
	    const QList<unsigned int> &tracks
	);

	/**
	 * shift the positions or start/end of all elements that are after
	 * a given offset to the right.
	 *
	 * @param offset index of the first sample
	 * @param shift number of samples to shift right
	 * @param tracks list of track indices
	 */
	virtual void shiftRight(
	    sample_index_t offset,
	    sample_index_t shift,
	    const QList<unsigned int> &tracks
	);

	/**
	 * scale the positions or start/end of all elements by a given factor
	 *
	 * @param scale the factor that is applied to all positions
	 * @param tracks list of track indices
	 */
	virtual void scalePositions(
	    double scale,
	    const QList<unsigned int> &tracks
	);

	/** dump all meta data to stdout (for debugging) */
	virtual void dump() const;

    protected:

	/**
	 * Splits the list at a given position. The given position will
	 * be the start of the new fragment(s), so that splitting multiple
	 * times at the same offset does not produce further fragments.
	 * @param offset index of the sample position before which the
	 *               list should be split
	 * @param tracks list of track indices
	 */
	void split(sample_index_t offset, const QList<unsigned int> &tracks);

    };

}

#endif /* _META_DATA_LIST_H_ */
