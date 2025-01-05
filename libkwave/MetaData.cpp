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

#include <new>

#include <QApplication>
#include <QDateTime>
#include <QMetaType>
#include <QMutexLocker>
#include <QUuid>

#include "libkwave/MetaData.h"
#include "libkwave/String.h"

//***************************************************************************
// initializers of the standard property names
const QString Kwave::MetaData::STDPROP_TYPE(       _("STDPROP_TYPE"));
const QString Kwave::MetaData::STDPROP_POS(        _("STDPROP_POS"));
const QString Kwave::MetaData::STDPROP_DESCRIPTION(_("STDPROP_DESCRIPTION"));

//***************************************************************************
Kwave::MetaData::MetaData()
    :m_data(nullptr)
{
}

//***************************************************************************
Kwave::MetaData::MetaData(const Kwave::MetaData &other)
    :m_data(other.m_data)
{
}

//***************************************************************************
Kwave::MetaData::MetaData(Scope scope)
    :m_data(new(std::nothrow) MetaDataPriv)
{
    setScope(scope);
}

//***************************************************************************
Kwave::MetaData::~MetaData()
{
    m_data = nullptr;
}

//***************************************************************************
void Kwave::MetaData::clear()
{
    if (m_data) m_data->m_properties.clear();
}

//***************************************************************************
bool Kwave::MetaData::isNull() const
{
    return (!m_data || (m_data->m_properties.isEmpty()));
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
void Kwave::MetaData::setProperty(const QString &p, const QVariant &value)
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

    // fallback: infinite
    return SAMPLE_INDEX_MAX;
}

//***************************************************************************
static inline QString shortened(const QString &str)
{
    if (str.length() < 1024)
        return _("'") + str + _("'");
    else
        return _("'") + str.left(1024) + _("'...");
}

//***************************************************************************
void Kwave::MetaData::dump() const
{
    QString scope_list;
    const Scope s = scope();

    if (s & Signal)   scope_list += _(" signal");
    if (s & Position) scope_list += _(" position");

    qDebug("    scope =%s", DBG(scope_list));
    const QStringList props = keys();
    foreach (const QString &name, props) {
        QVariant    val  = property(name);
        QStringList list = val.toStringList();
        if (list.size() > 1)
        {
            int index = 0;
            foreach (const QString &v, list)
            {
                if (index > 32) {
                    qDebug("    ..."); // emergency break
                    break;
                }
                qDebug("   '%s[%2d]' = %s", DBG(name), index++,
                       DBG(shortened(v)));
            }
        }
        else
            qDebug("   '%s' = %s", DBG(name), DBG(shortened(val.toString())));
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
    uid += _("-");
    uid += QDateTime::currentDateTime().toString(Qt::ISODate);
    uid += _("-");
    uid += qApp->sessionKey();
    uid += _("-");
    uid += QUuid::createUuid().toString();

    return uid;
}

//***************************************************************************
//***************************************************************************
