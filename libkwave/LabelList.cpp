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

#include <algorithm>

#include "libkwave/LabelList.h"
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"

//***************************************************************************
Kwave::LabelList::LabelList()
    :QList<Kwave::Label>()
{
}

//***************************************************************************
Kwave::LabelList::LabelList(const Kwave::MetaDataList &meta_data_list)
    :QList<Kwave::Label>()
{
    if (!meta_data_list.isEmpty()) {
	// get a list sorted by position
	QList<Kwave::MetaData> list =
	    meta_data_list.selectByType(Kwave::Label::metaDataType()).toSortedList();

	if (!list.isEmpty()) {
	    // append a label for each meta data object
	    foreach (const Kwave::MetaData &meta_data, list)
		append(Kwave::Label(meta_data));
	}

	sort();
    }
}

//***************************************************************************
Kwave::LabelList::~LabelList()
{
}

//***************************************************************************
static bool compare_labels(Kwave::Label a, Kwave::Label b)
{
    return (a < b);
}

//***************************************************************************
void Kwave::LabelList::sort()
{
    if (!isEmpty())
	std::sort(begin(), end(), compare_labels);
}

//***************************************************************************
Kwave::MetaDataList Kwave::LabelList::toMetaDataList() const
{
    Kwave::MetaDataList list;
    foreach (const Kwave::Label &label, *this)
	list.add(label);
    return list;
}

//***************************************************************************
sample_index_t Kwave::LabelList::nextLabelLeft(sample_index_t from)
{
    sample_index_t best  = 0;
    bool           found = false;
    if (!isEmpty()) {
	foreach (const Kwave::Label &label, *this) {
	    sample_index_t lp = label.pos();
	    if (lp >= from) break;
	    best  = lp;
	    found = true;
	}
    }
    return (found) ? best : 0;
}

//***************************************************************************
sample_index_t Kwave::LabelList::nextLabelRight(sample_index_t from)
{
    if (!isEmpty()) {
	foreach (const Kwave::Label &label, *this) {
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
