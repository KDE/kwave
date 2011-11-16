/***************************************************************************
 UndoAddMetaDataAction.cpp  -  Undo action for insertion of labels
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

#include <QVariant>
#include <QtAlgorithms>

#include <klocale.h>

#include "libkwave/MetaData.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAddMetaDataAction.h"
#include "libkwave/undo/UndoDeleteMetaDataAction.h"

//***************************************************************************
UndoAddMetaDataAction::UndoAddMetaDataAction(
    const Kwave::MetaDataList &meta_data)
    :UndoAction(),
     m_description(),
     m_offset(0),
     m_length(SAMPLE_INDEX_MAX),
     m_tracks()
{
    sample_index_t first = SAMPLE_INDEX_MAX;
    sample_index_t last  = 0;

    // sanity check: list should not be empty
    Q_ASSERT(!meta_data.isEmpty());
    if (meta_data.isEmpty()) return;

    /*
     * loop over the list to find out the first/last sample offset
     * and the list of affected tracks
     */
    foreach (const Kwave::MetaData &m, meta_data)
    {
	// search over a list of known properties which contain range/position
	QStringList properties = Kwave::MetaData::positionBoundPropertyNames();
	foreach (const QString &tag, properties) {
	    // check for a "start" property
	    QVariant v = m[tag];
	    bool ok = false;
	    sample_index_t pos = static_cast<sample_index_t>(v.toULongLong(&ok));
	    if (ok && (pos < first)) first = pos;
	    if (ok && (pos > last))  last  = pos;
	}

	// convert the track list into a usable list of unsigned int
	const QList<QVariant> v_track_list =
	    m[Kwave::MetaData::STDPROP_TRACKS].toList();
	QList<unsigned int> bound_tracks;
	foreach (const QVariant &v, v_track_list) {
	    bool ok = false;
	    unsigned int t = v.toUInt(&ok);
	    if (ok) bound_tracks += t;
	}

	foreach (unsigned int t, bound_tracks)
	    if (!m_tracks.contains(t)) m_tracks.append(t);;
    }

    // fix first/last in case that nothing was found, select everything
    if (first > last) {
	m_offset = 0;
	m_length = SAMPLE_INDEX_MAX;
    } else {
	m_offset = first;
	m_length = (last - first) + 1;
    }
    qSort(m_tracks);

    /*
     * determine the description of the action
     */

    for (;;) {
	QString name;
	const Kwave::MetaData &m = meta_data.values().first();
	if (m.hasProperty(Kwave::MetaData::STDPROP_TYPE))
	    name = m[Kwave::MetaData::STDPROP_TYPE].toString();

	// if the meta data list contains only one object: try to find
	// out the object's name
	if ((meta_data.count() == 1) && name.length()) {
	    m_description = i18nc(
		"name of the undo action for inserting a meta data object",
		"Insert %1",
		name
	    );
	    break;
	}

	// check if the list contains only objects of the same type
	bool all_same_type = true;
	foreach (const Kwave::MetaData &m, meta_data) {
	    QString n = m[Kwave::MetaData::STDPROP_TYPE].toString();
	    if (!n.length() || (n != name)) {
		all_same_type = false;
		break;
	    }
	}
	if (all_same_type) {
	    m_description = i18nc(
		"name of the undo action for inserting multiple "
		"meta data objects of the same type: "
		"%1=number of elements, %2=name of one element in singular",
		"Insert %1 %2 objects",
		name
	    );
	    break;
	}

	m_description = i18n("Insert Meta Data");
	break;
    }
}

//***************************************************************************
UndoAddMetaDataAction::~UndoAddMetaDataAction()
{
}

//***************************************************************************
QString UndoAddMetaDataAction::description()
{
    return m_description;
}

//***************************************************************************
unsigned int UndoAddMetaDataAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
int UndoAddMetaDataAction::redoSize()
{
    return sizeof(UndoDeleteMetaDataAction);
}

//***************************************************************************
bool UndoAddMetaDataAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
UndoAction *UndoAddMetaDataAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo = 0;

    Kwave::MetaDataList meta_data =
	manager.metaData().copy(m_offset, m_length, m_tracks);

    // store data for redo
    if (with_redo && !meta_data.isEmpty()) {
	redo = new UndoDeleteMetaDataAction(meta_data);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // remove the meta data from the signal manager
    manager.metaData().deleteRange(m_offset, m_length, m_tracks);

    return redo;
}

//***************************************************************************
//***************************************************************************
