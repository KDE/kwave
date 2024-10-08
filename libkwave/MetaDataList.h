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
#ifndef META_DATA_LIST_H
#define META_DATA_LIST_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>
#include <QMap>
#include <QMapIterator>
#include <QMutableMapIterator>
#include <QString>
#include <QVariant>

#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"

namespace Kwave
{

    class LIBKWAVE_EXPORT MetaDataList: public QMap<QString, MetaData>
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
        explicit MetaDataList(const MetaData &meta);

        /** Destructor */
        virtual ~MetaDataList();

        /**
         * Create a simple list of meta data items, sorted by the position
         * of the first sample. All meta data items that do not correspond
         * to a position or a "first" sample are mapped to the start (zero).
         * @return a QList of meta data, sorted by position
         */
        virtual QList<Kwave::MetaData> toSortedList() const;

        /**
         * select elements from the meta data list that have the standard
         * property STDPROP_TYPE set to a specific value.
         *
         * @param type the type to select @see Kwave::MetaData::STDPROP_TYPE
         * @return list with found meta data objects
         */
        virtual MetaDataList selectByType(const QString &type) const;

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
         * @param pos index of the sample to select
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
         * If the object is a null object and an object with the same
         * ID exists in the list, this will work as remove().
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
         * copy elements from the meta data list that overlap a given
         * range of samples (selects elements with scope "Range" as well
         * as elements with scope "Position")
         *
         * @param offset index of the first sample
         * @param length number of samples of the range
         * @return list with a copy of found meta data objects
         */
        virtual MetaDataList copy(
            sample_index_t offset,
            sample_index_t length
        ) const;

        /**
         * delete elements from the meta data list that overlap a given
         * range of samples (selects elements with scope "Range" as well
         * as elements with scope "Position") and have a binding to a
         * track.
         *
         * @param offset index of the first sample
         * @param length number of samples to delete
         */
        virtual void deleteRange(sample_index_t offset, sample_index_t length);

        /**
         * shift the positions or start/end of all elements that are after
         * a given offset to the left.
         *
         * @param offset index of the first sample
         * @param shift number of samples to shift left
         */
        virtual void shiftLeft(sample_index_t offset, sample_index_t shift);

        /**
         * shift the positions or start/end of all elements that are after
         * a given offset to the right.
         *
         * @param offset index of the first sample
         * @param shift number of samples to shift right
         */
        virtual void shiftRight(sample_index_t offset, sample_index_t shift);

        /**
         * scale the positions or start/end of all elements by a given factor
         *
         * @param scale the factor that is applied to all positions
         */
        virtual void scalePositions(double scale);

        /** dump all meta data to stdout (for debugging) */
        virtual void dump() const;

    };

}

#endif /* META_DATA_LIST_H */

//***************************************************************************
//***************************************************************************
