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

#include "libkwave/MetaDataList.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

//***************************************************************************
Kwave::MetaDataList::MetaDataList()
    :QMap<QString, Kwave::MetaData>()
{
}

//***************************************************************************
Kwave::MetaDataList::MetaDataList(const Kwave::MetaData &meta)
    :QMap<QString, Kwave::MetaData>()
{
    add(meta);
}

//***************************************************************************
Kwave::MetaDataList::~MetaDataList()
{
}

//***************************************************************************
static bool isLessThan(const Kwave::MetaData &m1, const Kwave::MetaData &m2)
{
     return m1.firstSample() < m2.firstSample();
}

//***************************************************************************
QList<Kwave::MetaData> Kwave::MetaDataList::toSortedList() const
{
    QList<Kwave::MetaData> list = this->values();

    qStableSort(list.begin(), list.end(), isLessThan);

    return list;
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
	// iterate over all meta data items
	it.next();
	const Kwave::MetaData &m = it.value();
	bool match = true;
	if (m.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {
	    // iterate over the list of tracks of the item
	    match = false;
	    QList<QVariant> track_list =
		m[Kwave::MetaData::STDPROP_TRACKS].toList();
	    foreach (const QVariant &v, track_list) {
		bool ok = false;
		unsigned int t = v.toUInt(&ok);
		Q_ASSERT(ok);
		if (ok && (tracks.contains(t))) {
		    match = true;
		    break;
		}
	    }
	}
	if (match) {
	    Q_ASSERT(!list.keys().contains(m.id()));
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
void Kwave::MetaDataList::replace(const Kwave::MetaDataList &list)
{
    if (list.isEmpty()) return;

    // find out which meta data types are affected
    QStringList types;
    foreach (const Kwave::MetaData &meta, list) {
	QString type = meta[Kwave::MetaData::STDPROP_TYPE].toString();
	if (!types.contains(type)) {
	    // remember this type in our list
	    types.append(type);

	    // remove all elements of that type that are not in the new list
	    MutableIterator it(*this);
	    while (it.hasNext()) {
		it.next();
		Kwave::MetaData &m = it.value();
		if (m[Kwave::MetaData::STDPROP_TYPE] == type) {
		    if (!list.contains(m)) {
			it.remove();
		    }
		}
	    }
	}
    }

    // now the same as in add() has to be done
    add(list);
}

//***************************************************************************
void Kwave::MetaDataList::add(const Kwave::MetaData &metadata)
{
    if (!metadata.isNull())
	(*this)[metadata.id()] = metadata;
    else
	remove(metadata);
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
void Kwave::MetaDataList::cropByRange(sample_index_t first,
                                      sample_index_t last)
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
		    v_track_list.append(Kwave::toUint(i));

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
    Kwave::MetaDataList list(*this);
    list.cropByRange(offset, offset + length - 1);
    list.cropByTracks(tracks);
    return list;
}

//***************************************************************************
void Kwave::MetaDataList::merge(const Kwave::MetaDataList &meta_data)
{
    const QStringList position_bound_properties =
	Kwave::MetaData::positionBoundPropertyNames();

    foreach (const Kwave::MetaData &meta, meta_data) {
	// check if some meta data with the same type already
	// exists at an overlapping position
	bool found = false;
	if (meta.hasProperty(Kwave::MetaData::STDPROP_TYPE)) {
	    MutableIterator it(*this);
	    while (it.hasNext()) {
		it.next();
		Kwave::MetaData &other = it.value();

		/* --- analysis phase --- */

		// check: both have the same type?
		if (!other.hasProperty(Kwave::MetaData::STDPROP_TYPE))
		    continue;
		if (other[Kwave::MetaData::STDPROP_TYPE] !=
		    meta[Kwave::MetaData::STDPROP_TYPE])
		    continue;

		// check: sampe scope?
		if (!(meta.scope() == other.scope()))
		    continue;

		// check: ranges overlap or touch?
		sample_index_t meta_first  = meta.firstSample();
		sample_index_t meta_last   = meta.lastSample();
		sample_index_t other_first = other.firstSample();
		sample_index_t other_last  = other.lastSample();
		if ((meta_last < other_first) && (meta_last + 1 != other_first))
		    continue;
		if ((meta_first > other_last) && (meta_first != other_last + 1))
		    continue;

		// determine list of overlapping/non-overlapping tracks
		QList<unsigned int> overlapping_tracks;
		QList<unsigned int> non_overlapping_tracks;
		if (meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS) &&
		    other.hasProperty(Kwave::MetaData::STDPROP_TRACKS))
		{
		    QList<unsigned int> meta_tracks  = meta.boundTracks();
		    QList<unsigned int> other_tracks = other.boundTracks();

		    foreach (unsigned int t, meta_tracks) {
			if (other_tracks.contains(t))
			    overlapping_tracks.append(t);
			else
			    non_overlapping_tracks.append(t);
		    }
		}

		// check: no overlapping tracks?
		if (overlapping_tracks.isEmpty())
		    continue;

		// check: all non-positional properties have to match
		bool match = true;
		foreach (const QString &p, meta.keys()) {
		    if (!other.hasProperty(p)) {
			match = false;
			break;
		    }

		    // ignore internal properties
		    if (p == Kwave::MetaData::STDPROP_TRACKS)
			continue;
		    if (position_bound_properties.contains(p))
			continue;

		    if (meta[p] != other[p]) {
			match = false;
			break;
		    }
		}
		if (!match) continue;

		/* --- merge phase --- */

		found = true;

		// split all data bound to non-overlapping tracks into
		// a separate meta data object
		if (!non_overlapping_tracks.isEmpty()) {
		    Kwave::MetaData copy = other;

		    QVariantList list;
		    foreach (unsigned int t, non_overlapping_tracks)
			list.append(QVariant(t));
		    other.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		    list.clear();
		    foreach (unsigned int t, overlapping_tracks)
			list.append(QVariant(t));
		    copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		    add(copy);
		}

		// merge range
		if (other.hasProperty(Kwave::MetaData::STDPROP_START)) {
		    other.setProperty(
			Kwave::MetaData::STDPROP_START,
			qMin(meta_first, other_first));
		}
		if (other.hasProperty(Kwave::MetaData::STDPROP_END)) {
		    other.setProperty(
			Kwave::MetaData::STDPROP_END,
			qMax(meta_last, other_last));
		}
	    }
	}

	// no matching meta data item for merging found => add as new one
	if (!found) {
	    add(meta);
	}
    }
}

//***************************************************************************
void Kwave::MetaDataList::deleteRange(sample_index_t offset,
                                      sample_index_t length,
                                      const QList<unsigned int> &tracks)
{
    const sample_index_t del_first = offset;
    const sample_index_t del_last  = offset + length - 1;

    if (!length) return;

    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &meta = it.value();

	sample_index_t meta_first  = meta.firstSample();
	sample_index_t meta_last   = meta.lastSample();

	// check: range overlap?
	if ((meta_first > del_last) || (meta_last  < del_first))
	    continue;

	// only operate on the matching tracks:
	if (!tracks.isEmpty() &&
	    meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {

	    // determine list of overlapping/non-overlapping tracks
	    QList<unsigned int> overlapping_tracks;
	    QList<unsigned int> non_overlapping_tracks;
	    QList<unsigned int> meta_tracks  = meta.boundTracks();

	    foreach (unsigned int t, meta_tracks) {
		if (tracks.contains(t))
		    overlapping_tracks.append(t);
		else
		    non_overlapping_tracks.append(t);
	    }

	    // skip if no overlap
	    if (overlapping_tracks.isEmpty())
		continue;

	    // split all data bound to non-overlapping tracks into
	    // a separate meta data object
	    if (!non_overlapping_tracks.isEmpty()) {
		Kwave::MetaData copy = meta;

		QVariantList list;
		foreach (unsigned int t, overlapping_tracks)
		    list.append(QVariant(t));
		meta.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		list.clear();
		foreach (unsigned int t, non_overlapping_tracks)
		    list.append(QVariant(t));
		copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		add(copy);
	    }
	}

	/* --- we have a position/range/track overlap --- */

	// position bound -> remove completely
	if (meta.hasProperty(Kwave::MetaData::STDPROP_POS)) {
	    it.remove();
	    continue;
	}

	// complete overlap -> remove completely
	if ((meta.scope() & Kwave::MetaData::Range) &&
	    (meta_first >= del_first) && (meta_last <= del_last)) {
	    it.remove();
	    continue;
	}

	// check: no range -> no adjustment
	if (!meta.hasProperty(Kwave::MetaData::STDPROP_START) ||
	    !meta.hasProperty(Kwave::MetaData::STDPROP_END)) {
	    continue;
	}

	// cut out a piece from the middle -> adjust right
	if ((del_first > meta_first) && (del_last < meta_last)) {
	    meta_last -= length;
	    meta[Kwave::MetaData::STDPROP_END] = QVariant(meta_last);
	    continue;
	}

	// cut away a part from left
	if (del_last < meta_last) {
	    meta[Kwave::MetaData::STDPROP_START] = QVariant(del_last + 1);
	    continue;
	}

	// cut away a part from right
	if (del_first > meta_first) {
	    meta[Kwave::MetaData::STDPROP_END] = QVariant(del_first - 1);
	    continue;
	}

	Q_ASSERT(false); // we should never reach this, no overlap?
    }
}

//***************************************************************************
void Kwave::MetaDataList::shiftLeft(sample_index_t offset, sample_index_t shift,
                                    const QList<unsigned int> &tracks)
{
    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &meta = it.value();

	sample_index_t meta_first  = meta.firstSample();
	sample_index_t meta_last   = meta.lastSample();

	// check: is it before the offset ?
	if (meta_first < offset)
	    continue;

	// only operate on the matching tracks:
	if (!tracks.isEmpty() &&
	    meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {

	    // determine list of overlapping/non-overlapping tracks
	    QList<unsigned int> overlapping_tracks;
	    QList<unsigned int> non_overlapping_tracks;
	    QList<unsigned int> meta_tracks  = meta.boundTracks();

	    foreach (unsigned int t, meta_tracks) {
		if (tracks.contains(t))
		    overlapping_tracks.append(t);
		else
		    non_overlapping_tracks.append(t);
	    }

	    // skip if no overlap
	    if (overlapping_tracks.isEmpty())
		continue;

	    // split all data bound to non-overlapping tracks into
	    // a separate meta data object
	    if (!non_overlapping_tracks.isEmpty()) {
		Kwave::MetaData copy = meta;

		QVariantList list;
		foreach (unsigned int t, overlapping_tracks)
		    list.append(QVariant(t));
		meta.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		list.clear();
		foreach (unsigned int t, non_overlapping_tracks)
		    list.append(QVariant(t));
		copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		add(copy);
	    }
	}

	/* --- we have a position/range/track overlap --- */

	// position bound -> move position
	if (meta.hasProperty(Kwave::MetaData::STDPROP_POS)) {
	    bool ok = false;
	    sample_index_t pos = static_cast<sample_index_t>(
		meta[Kwave::MetaData::STDPROP_POS].toULongLong(&ok));
	    if (!ok) continue;

	    if (pos >= shift) {
		// shift position left
		meta[Kwave::MetaData::STDPROP_POS] = pos - shift;
	    } else  {
		// do not produce negative coordinates
		// -> moving into negative means deleting!
		it.remove();
	    }
	    continue;
	}

	// check: no range -> no adjustment
	if (!meta.hasProperty(Kwave::MetaData::STDPROP_START) ||
	    !meta.hasProperty(Kwave::MetaData::STDPROP_END)) {
	    continue;
	}

	// check: moving into negative
	Q_ASSERT(meta_last >= shift);
	if (meta_last < shift) {
	    it.remove();
	    continue;
	}

	// move to the left, clip start to zero
	Q_ASSERT(meta_first >= shift);
	meta_first  = (meta_first >= shift) ? (meta_first - shift) : 0;
	meta_last  -= shift;

	if (meta.hasProperty(Kwave::MetaData::STDPROP_START))
	    meta[Kwave::MetaData::STDPROP_START] = QVariant(meta_first);
	if (meta.hasProperty(Kwave::MetaData::STDPROP_END))
	    meta[Kwave::MetaData::STDPROP_END]   = QVariant(meta_last);
    }
}

//***************************************************************************
void Kwave::MetaDataList::shiftRight(sample_index_t offset, sample_index_t shift,
                                     const QList<unsigned int> &tracks)
{
    MutableIterator it(*this);
    it.toBack();
    while (it.hasPrevious()) {
	it.previous();
	Kwave::MetaData &meta = it.value();

	sample_index_t meta_first  = meta.firstSample();
	sample_index_t meta_last   = meta.lastSample();

	// check: is it before the offset ?
	if (meta_last < offset)
	    continue;

	// only operate on the matching tracks:
	if (!tracks.isEmpty() &&
	    meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {

	    // determine list of overlapping/non-overlapping tracks
	    QList<unsigned int> overlapping_tracks;
	    QList<unsigned int> non_overlapping_tracks;
	    QList<unsigned int> meta_tracks  = meta.boundTracks();

	    foreach (unsigned int t, meta_tracks) {
		if (tracks.contains(t))
		    overlapping_tracks.append(t);
		else
		    non_overlapping_tracks.append(t);
	    }

	    // skip if no overlap
	    if (overlapping_tracks.isEmpty())
		continue;

	    // split all data bound to non-overlapping tracks into
	    // a separate meta data object
	    if (!non_overlapping_tracks.isEmpty()) {
		Kwave::MetaData copy = meta;

		QVariantList list;
		foreach (unsigned int t, overlapping_tracks)
		    list.append(QVariant(t));
		meta.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		list.clear();
		foreach (unsigned int t, non_overlapping_tracks)
		    list.append(QVariant(t));
		copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		add(copy);
	    }
	}

	/* --- we have a position/range/track overlap --- */

	// position bound -> move position
	if (meta.hasProperty(Kwave::MetaData::STDPROP_POS)) {
	    bool ok = false;
	    sample_index_t pos = static_cast<sample_index_t>(
		meta[Kwave::MetaData::STDPROP_POS].toULongLong(&ok));
	    if (!ok) continue;

	    Q_ASSERT(pos + shift >= pos);
	    if (pos + shift >= pos) {
		// shift position right
		meta[Kwave::MetaData::STDPROP_POS] = pos + shift;
	    } else  {
		// do not produce a coordinate overflow
		// -> moving outside range means deleting!
		it.remove();
	    }
	    continue;
	}

	// check: no range -> no adjustment
	if (!meta.hasProperty(Kwave::MetaData::STDPROP_START) ||
	    !meta.hasProperty(Kwave::MetaData::STDPROP_END)) {
	    continue;
	}

	// check: range overflow
	Q_ASSERT(meta_first + shift >= meta_first);
	if (meta_first + shift < meta_first) {
	    it.remove();
	    continue;
	}

	// move to the right, clip end to maximum coordinate
	Q_ASSERT(meta_last + shift >= meta_last);
	meta_last  = (meta_last + shift >= meta_last) ?
	    (meta_last + shift) : SAMPLE_INDEX_MAX;
	if (meta_first >= offset)
	    meta_first += shift;

	if (meta.hasProperty(Kwave::MetaData::STDPROP_START))
	    meta[Kwave::MetaData::STDPROP_START] = QVariant(meta_first);
	if (meta.hasProperty(Kwave::MetaData::STDPROP_END))
	    meta[Kwave::MetaData::STDPROP_END]   = QVariant(meta_last);
    }
}

//***************************************************************************
void Kwave::MetaDataList::scalePositions(double scale,
                                         const QList<unsigned int> &tracks)
{
    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &meta = it.value();

	sample_index_t meta_first  = meta.firstSample();
	sample_index_t meta_last   = meta.lastSample();

	// only operate on the matching tracks:
	if (!tracks.isEmpty() &&
	    meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {

	    // determine list of overlapping/non-overlapping tracks
	    QList<unsigned int> overlapping_tracks;
	    QList<unsigned int> non_overlapping_tracks;
	    QList<unsigned int> meta_tracks  = meta.boundTracks();

	    foreach (unsigned int t, meta_tracks) {
		if (tracks.contains(t))
		    overlapping_tracks.append(t);
		else
		    non_overlapping_tracks.append(t);
	    }

	    // skip if no overlap
	    if (overlapping_tracks.isEmpty())
		continue;

	    // split all data bound to non-overlapping tracks into
	    // a separate meta data object
	    if (!non_overlapping_tracks.isEmpty()) {
		Kwave::MetaData copy = meta;

		QVariantList list;
		foreach (unsigned int t, overlapping_tracks)
		    list.append(QVariant(t));
		meta.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		list.clear();
		foreach (unsigned int t, non_overlapping_tracks)
		    list.append(QVariant(t));
		copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		add(copy);
	    }
	}

	/* --- we have a position/range/track overlap --- */

	// position bound -> move position
	if (meta.hasProperty(Kwave::MetaData::STDPROP_POS)) {
	    bool ok = false;
	    sample_index_t pos = static_cast<sample_index_t>(
		meta[Kwave::MetaData::STDPROP_POS].toULongLong(&ok));
	    if (!ok) continue;

	    if ((pos * scale) <= SAMPLE_INDEX_MAX) {
		// scale position
		meta[Kwave::MetaData::STDPROP_POS] = (pos * scale);
	    } else  {
		// do not produce a coordinate overflow
		// -> moving outside range means deleting!
		it.remove();
	    }
	    continue;
	}

	// check: no range -> no adjustment
	if (!meta.hasProperty(Kwave::MetaData::STDPROP_START) ||
	    !meta.hasProperty(Kwave::MetaData::STDPROP_END)) {
	    continue;
	}

	// check: range overflow
	if ((meta_first * scale) >= SAMPLE_INDEX_MAX) {
	    it.remove();
	    continue;
	}

	// scale, clip end to maximum coordinate
	meta_last  = static_cast<sample_index_t>(
	    qMin<double>(SAMPLE_INDEX_MAX, meta_last * scale));
	meta_first = static_cast<sample_index_t>(meta_first * scale);

	if (meta.hasProperty(Kwave::MetaData::STDPROP_START))
	    meta[Kwave::MetaData::STDPROP_START] = QVariant(meta_first);
	if (meta.hasProperty(Kwave::MetaData::STDPROP_END))
	    meta[Kwave::MetaData::STDPROP_END]   = QVariant(meta_last);
    }
}

//***************************************************************************
void Kwave::MetaDataList::insertTrack(unsigned int track)
{
    const QString prop = Kwave::MetaData::STDPROP_TRACKS;
    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &m = it.value();
	if (m.hasProperty(prop)) {
	    // iterate over the list of tracks
	    QList<QVariant> old_list = m[prop].toList();
	    QList<QVariant> new_list;
	    foreach (const QVariant &v, old_list) {
		bool ok = false;
		unsigned int t = v.toUInt(&ok);
		if (ok && (t >= track))
		    new_list.append(QVariant(t + 1));
		else
		    new_list.append(v);
	    }
	    m[prop] = new_list; // set updated list of bound tracks
	}
    }
}

//***************************************************************************
void Kwave::MetaDataList::deleteTrack(unsigned int track)
{
    const QString prop = Kwave::MetaData::STDPROP_TRACKS;
    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &m = it.value();
	if (m.hasProperty(prop)) {
	    // iterate over the list of tracks
	    QList<QVariant> old_list = m[prop].toList();
	    QList<QVariant> new_list;
	    foreach (const QVariant &v, old_list) {
		bool ok = false;
		unsigned int t = v.toUInt(&ok);
		Q_ASSERT(ok);
		if (!ok) continue;
		if (t < track)
		    new_list.append(v);
		else if (t > track)
		    new_list.append(QVariant(t - 1));
	    }
	    if (!new_list.isEmpty())
		m[prop] = new_list; // set updated list of bound tracks
	    else
		it.remove(); // list is empty now -> delete whole item
	}
    }
}

//***************************************************************************
void Kwave::MetaDataList::split(sample_index_t offset,
                                const QList<unsigned int> &tracks)
{
    // check: splitting at offset zero makes no sense, but is not forbidden
    if (!offset) return;

    MutableIterator it(*this);
    while (it.hasNext()) {
	it.next();
	Kwave::MetaData &meta = it.value();

	sample_index_t meta_first  = meta.firstSample();
	sample_index_t meta_last   = meta.lastSample();

	// check: is the split done in our range?
	if ((offset <= meta_first) || (offset > meta_last))
	    continue;

	// check: no range -> no splitting
	if (!meta.hasProperty(Kwave::MetaData::STDPROP_START) ||
	    !meta.hasProperty(Kwave::MetaData::STDPROP_END)) {
	    continue;
	}

	// only operate on the matching tracks:
	if (!tracks.isEmpty() &&
	    meta.hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {

	    // determine list of overlapping/non-overlapping tracks
	    QList<unsigned int> overlapping_tracks;
	    QList<unsigned int> non_overlapping_tracks;
	    QList<unsigned int> meta_tracks  = meta.boundTracks();

	    foreach (unsigned int t, meta_tracks) {
		if (tracks.contains(t))
		    overlapping_tracks.append(t);
		else
		    non_overlapping_tracks.append(t);
	    }

	    // skip if no overlap
	    if (overlapping_tracks.isEmpty())
		continue;

	    // split all data bound to non-overlapping tracks into
	    // a separate meta data object
	    if (!non_overlapping_tracks.isEmpty()) {
		Kwave::MetaData copy = meta;

		QVariantList list;
		foreach (unsigned int t, overlapping_tracks)
		    list.append(QVariant(t));
		meta.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		list.clear();
		foreach (unsigned int t, non_overlapping_tracks)
		    list.append(QVariant(t));
		copy.setProperty(Kwave::MetaData::STDPROP_TRACKS, list);

		add(copy);
	    }
	}

	/* --- we have a range/track overlap --- */

	Kwave::MetaData copy = meta;
	copy[Kwave::MetaData::STDPROP_START] = QVariant(offset);
	meta[Kwave::MetaData::STDPROP_END]   = QVariant(offset - 1);
	add(copy);
    }
}

//***************************************************************************
void Kwave::MetaDataList::dump() const
{
    qDebug("--- meta data ---");

    Iterator it(*this);
    while (it.hasNext()) {
	it.next();
	const Kwave::MetaData &meta = it.value();
	qDebug("* meta data #%s", DBG(it.key()));
	meta.dump();
    }
    qDebug("-----------------");
}


//***************************************************************************
//***************************************************************************
