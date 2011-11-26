/***************************************************************************
    UndoModifyMetaDataAction.cpp  -  Undo action for modifying meta data
			     -------------------
    begin                : Sun Apr 03 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

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
#include <klocale.h>

#include "libkwave/Label.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoModifyMetaDataAction.h"

#include "libgui/SignalWidget.h"

//***************************************************************************
UndoModifyMetaDataAction::UndoModifyMetaDataAction(
    const Kwave::MetaDataList &meta_data)
    :UndoAction(), m_saved_data(meta_data)
{
}

//***************************************************************************
UndoModifyMetaDataAction::~UndoModifyMetaDataAction()
{
}

//***************************************************************************
QString UndoModifyMetaDataAction::description()
{
    // sanity check: list should not be empty
    Q_ASSERT(!m_saved_data.isEmpty());
    if (m_saved_data.isEmpty()) return "";

    QString name;
    const Kwave::MetaData &m = m_saved_data.values().first();
    if (m.hasProperty(Kwave::MetaData::STDPROP_TYPE))
	name = m[Kwave::MetaData::STDPROP_TYPE].toString();

    // if the meta data list contains only one object: try to find
    // out the object's name
    if ((m_saved_data.count() == 1) && name.length()) {
	return i18nc(
	    "name of the undo action for modifying a meta data object",
	    "Modify %1",
	    name
	);
    }

    // check if the list contains only objects of the same type
    bool all_same_type = true;
    foreach (const Kwave::MetaData &m, m_saved_data) {
	QString n = m[Kwave::MetaData::STDPROP_TYPE].toString();
	if (!n.length() || (n != name)) {
	    all_same_type = false;
	    break;
	}
    }
    if (all_same_type) {
	return i18nc(
	    "name of the undo action for modifying multiple "
	    "meta data objects of the same type: "
	    "%1=number of elements, %2=name of one element in singular",
	    "Modify %1 %2 objects",
	    name
	);
    }

    return i18n("Modify Meta Data");
}

//***************************************************************************
unsigned int UndoModifyMetaDataAction::undoSize()
{
    return sizeof(*this) + sizeof(m_saved_data);
}

//***************************************************************************
int UndoModifyMetaDataAction::redoSize()
{
    return undoSize();
}

//***************************************************************************
bool UndoModifyMetaDataAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
UndoAction *UndoModifyMetaDataAction::undo(SignalManager &manager,
                                           bool with_redo)
{
    if (m_saved_data.isEmpty()) return 0;

    // store data for redo
    if (with_redo) {
	Kwave::MetaDataList old_data;
	Kwave::MetaDataList current_data = manager.metaData();

	foreach (const Kwave::MetaData &meta, m_saved_data) {
	    if (current_data.contains(meta)) {
		// add a new entry that will replace the old one
		old_data.add(current_data[meta.id()]);
	    } else {
		// add an empty entry that will delete the old one when added
		Kwave::MetaData empty_element(meta);
		old_data.add(empty_element);
		old_data[empty_element.id()].clear();
	    }
	}

	// restore the saved meta data
	manager.metaData().replace(m_saved_data);

	// take the previous values as new undo data
	m_saved_data = old_data;

	return this;
    } else {
	// restore the saved meta data
	manager.metaData().replace(m_saved_data);
	return 0;
    }
}

//***************************************************************************
void UndoModifyMetaDataAction::dump(const QString &indent)
{
   foreach (const Kwave::MetaData &m, m_saved_data) {
	qDebug("%sundo modify meta data object '%s'",
	    indent.toLocal8Bit().data(),
	    m.id().toLocal8Bit().data()
	);

	// dump all properties of the object
	foreach (const QString &key, m.keys()) {
	    QVariant v = m[key];
	    QString value = "";
	    if (v.type() == QVariant::List) {
		foreach (const QVariant &v1, v.toList())
		    value += "'" + v1.toString() + "' ";
	    } else {
		value = v.toString();
	    }
	    qDebug("%s    '%s' = '%s",
		indent.toLocal8Bit().data(),
		key.toLocal8Bit().data(),
		value.toLocal8Bit().data()
	    );
	}
    }
}

//***************************************************************************
//***************************************************************************