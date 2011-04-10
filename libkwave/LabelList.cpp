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
    Kwave::MetaDataList list(
	meta_data_list.selectByType(Label::metaDataType())
    );

    if (!list.isEmpty()) {
	foreach (const Kwave::MetaData &meta_data, list) {
	    // skip everything that is not a label

	    // create a copy of the label
	    Label label(meta_data);
	    
	    // and append it to the list
	    append(label);
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
//***************************************************************************
