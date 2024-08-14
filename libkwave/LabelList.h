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
#ifndef LABEL_LIST_H
#define LABEL_LIST_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>
#include <QListIterator>

#include "libkwave/Label.h"

namespace Kwave
{
    class MetaDataList;

    /** not more than a typedef since Qt4 */
    class LIBKWAVE_EXPORT LabelList: public QList<Kwave::Label>
    {
    public:

        /** Default constructor */
        LabelList();

        /**
         * Constructor, creates a label list from a list of meta data objects,
         * by filtering out all objects of label type (already sorted by
         * position)
         * @param meta_data_list list of meta data
         */
        explicit LabelList(const Kwave::MetaDataList &meta_data_list);

        /** Destructor */
        virtual ~LabelList();

        /** sorts the list by ascending position */
        virtual void sort();

        /**
        * returns the content of this list as a list of Kwave::MetaData objects
        * @return a meta data list
        */
        Kwave::MetaDataList toMetaDataList() const;

        /**
        * returns the position of the next label left from a given position
        * or zero (begin of signal) if there is none
        * @return a sample index [0...length-1]
        */
        sample_index_t nextLabelLeft(sample_index_t from);

        /**
        * returns the position of the next label right from a given position
        * or SAMPLE_INDEX_MAX if there is none
        * @return a sample index [0...length-1] or SAMPLE_INDEX_MAX
        */
        sample_index_t nextLabelRight(sample_index_t from);

    };

    /** Iterator for the list of labels */
    typedef QListIterator<Kwave::Label> LabelListIterator;

}

#endif /* LABEL_LIST_H */

//***************************************************************************
//***************************************************************************
