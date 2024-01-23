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

#include <algorithm>

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

    if (!list.isEmpty())
        std::stable_sort(list.begin(), list.end(), isLessThan);

    return list;
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::selectByType(const QString &type) const
{
    return selectByValue(Kwave::MetaData::STDPROP_TYPE, type);
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
        if (m.scope() == Kwave::MetaData::Position) {
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
            // it is out of scope
            const QVariant v_pos = m[Kwave::MetaData::STDPROP_POS];
            bool ok = false;
            sample_index_t pos = static_cast<sample_index_t>(
                v_pos.toULongLong(&ok));
            if (!ok) continue;

            if ((pos < first) || (pos > last)) {
                // out of the selected area -> remove
                it.remove();
            }
        }
    }
}

//***************************************************************************
Kwave::MetaDataList Kwave::MetaDataList::copy(sample_index_t offset,
    sample_index_t length) const
{
    if (!length)
        return Kwave::MetaDataList(); // no range selected - empty list

    Kwave::MetaDataList list(*this);
    list.cropByRange(offset, offset + length - 1);
    return list;
}

//***************************************************************************
void Kwave::MetaDataList::deleteRange(sample_index_t offset,
                                      sample_index_t length)
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

        // position bound -> remove completely
        if (meta.hasProperty(Kwave::MetaData::STDPROP_POS))
            it.remove();
    }
}

//***************************************************************************
void Kwave::MetaDataList::shiftLeft(sample_index_t offset,
                                    sample_index_t shift)
{
    MutableIterator it(*this);
    while (it.hasNext()) {
        it.next();
        Kwave::MetaData &meta = it.value();

        // check: is it before the offset ?
        sample_index_t meta_first  = meta.firstSample();
        if (meta_first < offset)
            continue;

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
        }
    }
}

//***************************************************************************
void Kwave::MetaDataList::shiftRight(sample_index_t offset,
                                     sample_index_t shift)
{
    MutableIterator it(*this);
    it.toBack();
    while (it.hasPrevious()) {
        it.previous();
        Kwave::MetaData &meta = it.value();

        // check: is it before the offset ?
        sample_index_t meta_last   = meta.lastSample();
        if (meta_last < offset)
            continue;

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
        }
    }
}

//***************************************************************************
void Kwave::MetaDataList::scalePositions(double scale)
{
    MutableIterator it(*this);
    while (it.hasNext()) {
        it.next();
        Kwave::MetaData &meta = it.value();

        // position bound -> move position
        if (meta.hasProperty(Kwave::MetaData::STDPROP_POS)) {
            bool ok = false;
            sample_index_t pos = static_cast<sample_index_t>(
                meta[Kwave::MetaData::STDPROP_POS].toULongLong(&ok));
            if (!ok) continue;

            sample_index_t scaled_pos = static_cast<sample_index_t>(
                static_cast<double>(pos) * scale);
            if (scaled_pos <= SAMPLE_INDEX_MAX) {
                // scale position
                meta[Kwave::MetaData::STDPROP_POS] = scaled_pos;
            } else  {
                // do not produce a coordinate overflow
                // -> moving outside range means deleting!
                it.remove();
            }
        }
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
