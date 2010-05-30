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
    foreach (const Kwave::MetaData &meta_data, meta_data_list) {
	QVariant v_pos  = meta_data[Kwave::MetaData::STDPROP_POS];
	QVariant v_name = meta_data[Kwave::MetaData::STDPROP_DESCRIPTION];

	bool ok = false;
	sample_index_t pos = v_pos.toULongLong(&ok);
	if (!ok) continue;

	QString name = v_name.toString();

	// create a new label, from name and position
	Label label(pos, name);
	append(label);
    }
    sort();
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
//***************************************************************************
