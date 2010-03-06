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
Kwave::MetaDataList Kwave::MetaDataList::selectByTrack(
    unsigned int track) const
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
		if ((v.toUInt(&ok) == track) && ok) {
		    list.add(m);
		    break;
		}
	    }
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
//***************************************************************************
