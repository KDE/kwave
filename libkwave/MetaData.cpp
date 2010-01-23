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

#include "MetaData.h"

//***************************************************************************
// initializers of the standard property names
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
    if (m_data) m_data->m_properties[p] = value;
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
//***************************************************************************
Kwave::MetaData::MetaDataPriv::MetaDataPriv()
    :QSharedData(), m_scope(), m_properties()
{
}

//***************************************************************************
Kwave::MetaData::MetaDataPriv::MetaDataPriv(const MetaDataPriv &other)
    :QSharedData(), m_scope(other.m_scope), m_properties(other.m_properties)
{
}

//***************************************************************************
Kwave::MetaData::MetaDataPriv::~MetaDataPriv()
{
}

//***************************************************************************
//***************************************************************************
