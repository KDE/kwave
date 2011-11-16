/***************************************************************************
          MetaData.cpp  -  base class for associated meta data
                             -------------------
    begin                : Sat Jan 23 2010
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

#include <QApplication>
#include <QDateTime>
#include <QMutexLocker>
#include <QUuid>

#include "MetaData.h"

//***************************************************************************
// initializers of the standard property names
const QString Kwave::MetaData::STDPROP_TYPE("STDPROP_TYPE");
const QString Kwave::MetaData::STDPROP_TRACKS("STDPROP_TRACKS");
const QString Kwave::MetaData::STDPROP_START("STDPROP_START");
const QString Kwave::MetaData::STDPROP_END("STDPROP_END");
const QString Kwave::MetaData::STDPROP_POS("STDPROP_POS");
const QString Kwave::MetaData::STDPROP_DESCRIPTION("STDPROP_DESCRIPTION");

//***************************************************************************
Kwave::MetaData::MetaData()
    :m_data(0)
{
}

//***************************************************************************
Kwave::MetaData::MetaData(const Kwave::MetaData &other)
    :m_data(other.m_data)
{
}

//***************************************************************************
Kwave::MetaData::MetaData(Scope scope)
    :m_data(new MetaDataPriv)
{
    setScope(scope);
}

//***************************************************************************
Kwave::MetaData::~MetaData()
{
}

//***************************************************************************
void Kwave::MetaData::clear()
{
    if (m_data != 0) m_data->m_properties.clear();
}

//***************************************************************************
bool Kwave::MetaData::isNull() const
{
    return ((m_data == 0) || (m_data->m_properties.isEmpty()));
}

//***************************************************************************
QString Kwave::MetaData::id() const
{
    return (m_data) ? m_data->m_id : QString();
}

//***************************************************************************
Kwave::MetaData::Scope Kwave::MetaData::scope() const
{
    return (m_data) ? m_data->m_scope : None;
}

//***************************************************************************
void Kwave::MetaData::setScope(Kwave::MetaData::Scope scope)
{
    if (m_data) m_data->m_scope = scope;
}

//***************************************************************************
void Kwave::MetaData::setProperty(const QString &p,
                                  const QVariant &value)
{
    if (m_data) {
	if (value.isValid())
	    m_data->m_properties[p] = value;
	else
	    m_data->m_properties.remove(p);
    }
}

//***************************************************************************
bool Kwave::MetaData::hasProperty(const QString &property) const
{
    return (m_data && m_data->m_properties.contains(property));
}

//***************************************************************************
QVariant Kwave::MetaData::property(const QString &p) const
{
    if (m_data && m_data->m_properties.contains(p))
	return m_data->m_properties[p];
    else
	return QVariant();
}

//***************************************************************************
QVariant &Kwave::MetaData::property(const QString &p)
{
    if (m_data && m_data->m_properties.contains(p))
	return m_data->m_properties[p];
    else {
	static QVariant dummy;
	dummy.clear();
	return dummy;
    }
}

//***************************************************************************
bool Kwave::MetaData::operator == (const Kwave::MetaData &other) const
{
    if (!m_data && !other.m_data)
	return true; // both are null objects

    if ((!m_data) ^ (!other.m_data))
	return false; // only one is a null object

    if (m_data->m_scope != other.m_data->m_scope)
	return false; // different scope

    if (m_data->m_properties != other.m_data->m_properties)
	return false; // properties differ

    return true; // no mismatch found
}

//***************************************************************************
QStringList Kwave::MetaData::keys() const
{
    return (m_data) ? m_data->m_properties.keys() : QStringList();
}

//***************************************************************************
QStringList Kwave::MetaData::positionBoundPropertyNames()
{
    QStringList list;
    list << Kwave::MetaData::STDPROP_START;
    list << Kwave::MetaData::STDPROP_END;
    list << Kwave::MetaData::STDPROP_POS;
    return list;
}

//***************************************************************************
sample_index_t Kwave::MetaData::firstSample() const
{
    // single position?
    if ((scope() & Kwave::MetaData::Position) &&
	hasProperty(Kwave::MetaData::STDPROP_POS)) {
	bool ok = false;
	sample_index_t pos =
	    property(Kwave::MetaData::STDPROP_POS).toULongLong(&ok);
	if (ok) return pos;
    }

    // start sample index given
    if ((scope() & Kwave::MetaData::Range) &&
	hasProperty(Kwave::MetaData::STDPROP_START)) {
	bool ok = false;
	sample_index_t start = static_cast<sample_index_t>(
	    property(Kwave::MetaData::STDPROP_START).toULongLong(&ok));
	if (ok) return start;
    }

    // fallback: start at zero
    return 0;
}

//***************************************************************************
sample_index_t Kwave::MetaData::lastSample() const
{
    // single position?
    if ((scope() & Kwave::MetaData::Position) &&
	hasProperty(Kwave::MetaData::STDPROP_POS)) {
	bool ok = false;
	sample_index_t pos =
	    property(Kwave::MetaData::STDPROP_POS).toULongLong(&ok);
	if (ok) return pos;
    }

    // bound to a scope
    if (scope() & Kwave::MetaData::Range) {
	// end sample index given
	if (hasProperty(Kwave::MetaData::STDPROP_END)) {
	    bool ok = false;
	    sample_index_t end = static_cast<sample_index_t>(
		property(Kwave::MetaData::STDPROP_END).toULongLong(&ok));
	    if (ok) return end;
	}

	// fallback: no end => use start
	if (hasProperty(Kwave::MetaData::STDPROP_START)) {
	    bool ok = false;
	    sample_index_t start = static_cast<sample_index_t>(
		property(Kwave::MetaData::STDPROP_START).toULongLong(&ok));
	    if (ok) return start;
	}
    }

    // fallback: infinite
    return SAMPLE_INDEX_MAX;
}

//***************************************************************************
QList<unsigned int> Kwave::MetaData::boundTracks() const
{
    QList<unsigned int> tracks;
    if (hasProperty(Kwave::MetaData::STDPROP_TRACKS)) {
	const QList<QVariant> v_track_list =
	    property(Kwave::MetaData::STDPROP_TRACKS).toList();
	foreach (const QVariant &v, v_track_list) {
	    bool ok = false;
	    unsigned int t = v.toUInt(&ok);
	    if (ok) tracks += t;
	}
    }
    return tracks;
}

//***************************************************************************
void Kwave::MetaData::dump() const
{
    QString scope_list;
    const Scope s = scope();
    if (s == All)
	scope_list = "all";
    else {
	if (s & Signal)   scope_list += " signal";
	if (s & Track)    scope_list += " track";
	if (s & Range)    scope_list += " range";
	if (s & Position) scope_list += " position";
    }
    qDebug("    scope =%s", scope_list.toLocal8Bit().data());
    const QStringList props = keys();
    foreach (const QString &p, props) {
	QVariant prop = property(p);
	const QList<QVariant> v_vals = prop.toList();
	QString value;
	if (!v_vals.isEmpty()) {
	    foreach (QVariant v, v_vals)
		value += "{'" + v.toString() + "'} ";
	} else {
	    value += "'" + prop.toString() + "'";
	}

	qDebug("    '%s' = %s",
	    p.toLocal8Bit().data(),
	    value.toLocal8Bit().data()
	);
    }
}

//***************************************************************************
//***************************************************************************

/** static initializer: counter for unique id generation */
quint64 Kwave::MetaData::MetaDataPriv::m_id_counter = 0;

/** static initializer: mutex for protecting the id generator */
QMutex Kwave::MetaData::MetaDataPriv::m_id_lock;

//***************************************************************************
Kwave::MetaData::MetaDataPriv::MetaDataPriv()
    :QSharedData(),
     m_id(newUid()),
     m_scope(),
     m_properties()
{
}

//***************************************************************************
Kwave::MetaData::MetaDataPriv::MetaDataPriv(const MetaDataPriv &other)
    :QSharedData(),
     m_id(other.m_id),
     m_scope(other.m_scope),
     m_properties(other.m_properties)
{
}

//***************************************************************************
Kwave::MetaData::MetaDataPriv::~MetaDataPriv()
{
}

//***************************************************************************
QString Kwave::MetaData::MetaDataPriv::newUid()
{
    // create a new unique ID:
    // <64 bit number> - <date/time> - <session id> - <pseudo uuid from Qt>
    QMutexLocker _lock(&m_id_lock);

    QString uid;

    uid += QString::number(++m_id_counter, 16);
    uid += "-";
    uid += QDateTime::currentDateTime().toString(Qt::ISODate);
    uid += "-";
    uid += qApp->sessionKey();
    uid += "-";
    uid += QUuid::createUuid().toString();

    return uid;
}

//***************************************************************************
//***************************************************************************
