/***************************************************************************
          LabelList.cpp  -  list of labels
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

#include "config.h"

#include <QtAlgorithms>

#include "libkwave/LabelList.h"
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"

//***************************************************************************
LabelList::LabelList()
    :QList<Label>()
{
}

//***************************************************************************
LabelList::LabelList(const Kwave::MetaDataList &meta_data_list)
    :QList<Label>()
{
    if (!meta_data_list.isEmpty()) {
	// get a list sorted by position
	QList<Kwave::MetaData> list =
	    meta_data_list.selectByType(Label::metaDataType()).toSortedList();

	if (!list.isEmpty()) {
	    // append a Label for each meta data object
	    foreach (const Kwave::MetaData &meta_data, list)
		append(Label(meta_data));
	}

	sort();
    }
}

//***************************************************************************
LabelList::~LabelList()
{
}

//***************************************************************************
static bool compare_labels(Label a, Label b)
{
    return (a < b);
}

//***************************************************************************
void LabelList::sort()
{
    qSort(begin(), end(), compare_labels);
}

//***************************************************************************
Kwave::MetaDataList LabelList::toMetaDataList() const
{
    Kwave::MetaDataList list;
    foreach (const Label &label, *this)
	list.add(label);
    return list;
}

//***************************************************************************
sample_index_t LabelList::nextLabelLeft(sample_index_t from)
{
    sample_index_t best  = 0;
    bool           found = false;
    if (!isEmpty()) {
	foreach (const Label &label, *this) {
	    sample_index_t lp = label.pos();
	    if (lp >= from) break;
	    best  = lp;
	    found = true;
	}
    }
    return (found) ? best : 0;
}

//***************************************************************************
sample_index_t LabelList::nextLabelRight(sample_index_t from)
{
    if (!isEmpty()) {
	foreach (const Label &label, *this) {
	    sample_index_t lp = label.pos();
	    if (lp  > from)
		return lp; // found the first label after "from"
	}
    }
    // nothing found: return "infinite"
    return SAMPLE_INDEX_MAX;
}

//***************************************************************************
//***************************************************************************
