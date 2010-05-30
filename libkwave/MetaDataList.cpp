/***************************************************************************
       MetaDataList.cpp  -  list with meta data objects
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

#include "config.h"

#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/MetaDataList.h"

//***************************************************************************
Kwave::MetaDataList::MetaDataList()
    :QMap<QString, Kwave::MetaData>()
{
}

//***************************************************************************
Kwave::MetaDataList::~MetaDataList()
{
}

//***************************************************************************
FileInfo Kwave::MetaDataList::fileInfo() const
{
    FileInfo file_info(selectByType(FileInfo::metaDataType()));
    return file_info;
}

//***************************************************************************
void Kwave::MetaDataList::setFileInfo(const FileInfo &file_info)
{
    // remove the old file info completely
    remove(selectByType(FileInfo::metaDataType()));

    // set the new file info (FileInfo is derived from MetaData)
    add(file_info);
}

//***************************************************************************
LabelList Kwave::MetaDataList::labels() const
{
    LabelList label_list(selectByType(Label::metaDataType()));
    return label_list;
}

//***************************************************************************
void Kwave::MetaDataList::setLabels(const LabelList &labels)
{
    // remove all existing labels
    remove(selectByType(Label::metaDataType()));

    // add the new labels
    foreach (const Label &label, labels) {
	add(label);
    }
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByType(const QString &type) const
{
    return selectByValue(Kwave::MetaData::STDPROP_TYPE, type);
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByScope(
    MetaData::Scope scope) const
{
    Kwave::MetaDataList list;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.scope() == scope)
	    list.add(m);
    }
    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByTracks(
    const QList<unsigned int> &tracks) const
{
    Kwave::MetaDataList list;
    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {
	    // iterate over the list of tracks
	    QList<QVariant> track_list =
		m[Kwave::MetaData::STDPROP_TRACKS].toList();
	    foreach (const QVariant &v, track_list) {
		bool ok = false;
		bool match = false;
		foreach (unsigned int track, tracks) {
		    if ((v.toUInt(&ok) == track) && ok) {
			match = true;
			break;
		    }
		}
		if (match) {
		    if (!list.keys().contains(m.id())) list.add(m);
		    break;
		}
	    }
	}
	else
	{
	    // element is not bound to a track
	    if (!list.keys().contains(m.id())) list.add(m);
	}
    }
    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByRange(
    sample_index_t first, sample_index_t last) const
{
    Kwave::MetaDataList list;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.scope() == Kwave::MetaData::Range) {
	    if (m.hasProperty(Kwave::MetaData::STDPROP_START) &&
		m.hasProperty(Kwave::MetaData::STDPROP_END))
	    {
		// check for overlap with a meta data that has a range
		bool start_ok = false, end_ok = false;
		const sample_index_t start =
		    m[Kwave::MetaData::STDPROP_START].toULongLong(&start_ok);
		const sample_index_t end =
		    m[Kwave::MetaData::STDPROP_END].toULongLong(&end_ok);
		if (!start_ok || !end_ok) continue;

		if ((start <= last) && (end >= first))
		    list.add(m);
	    }
	}
	else if (m.scope() == Kwave::MetaData::Position) {
	    if (m.hasProperty(Kwave::MetaData::STDPROP_POS)) {
		// check for position within the range
		bool pos_ok = false;
		const sample_index_t pos =
		    m[Kwave::MetaData::STDPROP_POS].toLongLong(&pos_ok);
		if (pos_ok && (pos >= first) && (pos <= last))
		    list.add(m);
	    }
	}
    }
    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByPosition(
    sample_index_t pos) const
{
    Kwave::MetaDataList list;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.scope() == Kwave::MetaData::Position) {
	    if (m.hasProperty(Kwave::MetaData::STDPROP_POS)) {
		// check for position within the range
		bool pos_ok = false;
		const sample_index_t p =
		    m[Kwave::MetaData::STDPROP_POS].toLongLong(&pos_ok);
		if (pos_ok && (p == pos))
		    list.add(m);
	    }
	}
    }
    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByProperty(
    const QString &property) const
{
    Kwave::MetaDataList list;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.hasProperty(property))
	    list.add(m);
    }
    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByValue(
	    const QString &property, QVariant value) const
{
    Kwave::MetaDataList list;

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.hasProperty(property) && (m[property] == value))
	    list.add(m);
    }
    return list;
}

//***************************************************************************
bool Kwave::MetaDataList::contains(const Kwave::MetaData &metadata) const
{
    Kwave::MetaDataList list;

    QString id = metadata.id();
    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.id() == id)
	    return true;
    }
    return false;
}

//***************************************************************************
void Kwave::MetaDataList::add(const Kwave::MetaData &metadata)
{
    (*this)[metadata.id()] = metadata;
}

//***************************************************************************
void Kwave::MetaDataList::add(const Kwave::MetaDataList &list)
{
    foreach (const Kwave::MetaData &metadata, list)
	add(metadata);
}

//***************************************************************************
void Kwave::MetaDataList::remove(const Kwave::MetaData &metadata)
{
    if (contains(metadata))
	QMap<QString, Kwave::MetaData>::remove(metadata.id());
}

//***************************************************************************
void Kwave::MetaDataList::remove(const Kwave::MetaDataList &list)
{
    foreach (const Kwave::MetaData &metadata, list)
	remove(metadata);
}

//***************************************************************************
void Kwave::MetaDataList::cropByRange(sample_index_t first, sample_index_t last)
{

    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &m = it.value();

	if (m.scope() & Kwave::MetaData::Position) {
	    // if the meta data is bound to a position, remove it if
	    // it is out of scope and adjust the position if it is
	    // within the selection
	    const QVariant v_pos = m[Kwave::MetaData::STDPROP_POS];
	    bool ok = false;
	    sample_index_t pos = static_cast<sample_index_t>(
		v_pos.toULongLong(&ok));
	    if (!ok) continue;

	    if ((pos < first) || (pos > last)) {
		// out of the selected area -> remove
		it.remove();
		continue;
	    } else {
		// move object left
		pos -= first;
		m[Kwave::MetaData::STDPROP_POS] = pos;
	    }
	} else if (m.scope() & Kwave::MetaData::Range) {
	    // if the meta data is bound to a scope, remove it if
	    // it does not overlap with the current selection,
	    // otherwise clip it to the bounds of the current selection
	    const QVariant v_start = m[Kwave::MetaData::STDPROP_START];
	    bool ok = false;
	    sample_index_t start = static_cast<sample_index_t>(
		v_start.toULongLong(&ok));
	    if (!ok) continue;

	    const QVariant v_end = m[Kwave::MetaData::STDPROP_END];
	    ok = false;
	    sample_index_t end = static_cast<sample_index_t>(
		v_start.toULongLong(&ok));
	    if (!ok) continue;

	    if ((end < start) || (start >= last)) {
		// out of the selected area -> remove
		it.remove();
		continue;
	    } else {
		// clip to the seleced range
		if (start < first) start = first;
		if (end   > last)  end   = last;

		// adjust start and end
		start -= first;
		end   -= first;
		m[Kwave::MetaData::STDPROP_START] = start;
		m[Kwave::MetaData::STDPROP_END]   = end;
	    }

	}
    }

}

//***************************************************************************
void Kwave::MetaDataList::cropByTracks(const QList<unsigned int> &tracks)
{
    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();

	if (m.scope() & Kwave::MetaData::Track) {
	    if (m.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {
		// convert the track list into a usable list of unsigned int
		QList<QVariant> v_track_list =
		    m[Kwave::MetaData::STDPROP_TRACKS].toList();
		QList<unsigned int> bound_tracks;
		foreach (const QVariant &v, v_track_list) {
		    bool ok = false;
		    unsigned int t = v.toUInt(&ok);
		    if (ok) bound_tracks += t;
		}

		foreach (unsigned int t, bound_tracks)
		    if (!tracks.contains(t)) bound_tracks.removeAll(t);
		if (bound_tracks.isEmpty()) {
		    // no overlapping track indices -> remove
		    it.remove();
		    continue;
		}

		// do the renumbering
		v_track_list.clear();
		for (int i = 0; i < bound_tracks.count(); i++)
		    v_track_list.append(static_cast<unsigned int>(i));

		// set a new track list
		m[Kwave::MetaData::STDPROP_TRACKS] = v_track_list;
	    }
	}
    }
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::copy(sample_index_t offset,
    sample_index_t length, const QList<unsigned int> &tracks) const
{
}

//***************************************************************************
void Kwave::MetaDataList::merge(const Kwave::MetaDataList &meta_data)
{
}

//***************************************************************************
void Kwave::MetaDataList::deleteRange(sample_index_t offset,
                                      sample_index_t length,
                                      const QList<unsigned int> &tracks)
{
}

//***************************************************************************
void Kwave::MetaDataList::shiftLeft(sample_index_t offset, sample_index_t shift,
                                    const QList<unsigned int> &tracks)
{
}

//***************************************************************************
void Kwave::MetaDataList::split(sample_index_t offset,
                                const QList<unsigned int> &tracks)
{
}

//***************************************************************************
//***************************************************************************
