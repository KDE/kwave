/***************************************************************************
 UndoAddLabelAction.cpp  -  Undo action for deleting labels
			     -------------------
    begin                : Wed Aug 16 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include <new>

#include <klocale.h>

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAddMetaDataAction.h"
#include "libkwave/undo/UndoDeleteMetaDataAction.h"

//***************************************************************************
Kwave::UndoDeleteMetaDataAction::UndoDeleteMetaDataAction(
    const Kwave::MetaDataList &meta_data)
    :UndoAction(), m_meta_data(meta_data)
{
}

//***************************************************************************
Kwave::UndoDeleteMetaDataAction::~UndoDeleteMetaDataAction()
{
}

//***************************************************************************
QString Kwave::UndoDeleteMetaDataAction::description()
{
    // sanity check: list should not be empty
    Q_ASSERT(!m_meta_data.isEmpty());
    if (m_meta_data.isEmpty()) return _("");

    QString name;
    const Kwave::MetaData &m = m_meta_data.values().first();
    if (m.hasProperty(Kwave::MetaData::STDPROP_TYPE))
	name = m[Kwave::MetaData::STDPROP_TYPE].toString();

    // if the meta data list contains only one object: try to find
    // out the object's name
    if ((m_meta_data.count() == 1) && name.length()) {
	return i18nc(
	    "name of the undo action for deleting a meta data object",
	    "Delete %1",
	    name
	);
    }

    // check if the list contains only objects of the same type
    bool all_same_type = true;
    foreach (const Kwave::MetaData &m, m_meta_data) {
	QString n = m[Kwave::MetaData::STDPROP_TYPE].toString();
	if (!n.length() || (n != name)) {
	    all_same_type = false;
	    break;
	}
    }
    if (all_same_type) {
	return i18nc(
	    "name of the undo action for deleting multiple "
	    "meta data objects of the same type: "
	    "%1=number of elements, %2=name of one element in singular",
	    "Delete %1 %2 objects",
	    name
	);
    }

    return i18n("Delete Meta Data");
}

//***************************************************************************
qint64 Kwave::UndoDeleteMetaDataAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
qint64 Kwave::UndoDeleteMetaDataAction::redoSize()
{
    return sizeof(Kwave::UndoAddMetaDataAction);
}

//***************************************************************************
bool Kwave::UndoDeleteMetaDataAction::store(Kwave::SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoDeleteMetaDataAction::undo(
    Kwave::SignalManager &manager, bool with_redo)
{
    Q_ASSERT(!m_meta_data.isEmpty());
    if (m_meta_data.isEmpty()) return 0;

    Kwave::UndoAction *redo = 0;

    // add the stored meta data to the signal managers' meta data
    manager.metaData().merge(m_meta_data);

    // store data for redo
    if (with_redo) {
	redo = new(std::nothrow) Kwave::UndoAddMetaDataAction(m_meta_data);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    return redo;
}

//***************************************************************************
void Kwave::UndoDeleteMetaDataAction::dump(const QString &indent)
{
    foreach (const Kwave::MetaData &m, m_meta_data) {
	qDebug("%sundo delete meta data object '%s'",
	       DBG(indent), DBG(m.id()));

	// dump all properties of the object
	foreach (const QString &key, m.keys()) {
	    QVariant v = m[key];
	    QString value;
	    if (v.type() == QVariant::List) {
		foreach (const QVariant &v1, v.toList())
		    value += _("'") + v1.toString() + _("' ");
	    } else {
		value = v.toString();
	    }
	    qDebug("%s    '%s' = '%s", DBG(indent), DBG(key), DBG(value));
	}
    }
}

//***************************************************************************
//***************************************************************************
