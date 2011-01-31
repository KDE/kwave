/***************************************************************************
             Labels.cpp  - representation of a Labeler plugin labels
                           -------------------
    begin                : Mon Oct 11 2010
    copyright            : (C) 2010 by Daniel Tihelka
    email                : dtihelka@kky.zcu.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Labels.h"

//***************************************************************************
// initializers of the static attributes
const QString LabelerLabel::METADATA_TYPE("LABELER_LABEL");
const QString LabelerLabel::LABELPROP_TYPE("LABELER_LABEL_PROPERTY");
const QVariant LabelInstant::LABEL_TYPE(1);
const QVariant LabelRegion::LABEL_TYPE(2);


//***************************************************************************
LabelInstant::LabelInstant(sample_index_t position, const QString &name)
    :Kwave::MetaData(Kwave::MetaData::Position)
{
    setProperty(LABELPROP_TYPE, LABEL_TYPE);
    setProperty(Kwave::MetaData::STDPROP_TYPE, METADATA_TYPE);
    setProperty(Kwave::MetaData::STDPROP_POS, position);
    if (name.length())
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
}

//***************************************************************************
LabelInstant::LabelInstant(Kwave::MetaData & parent)
    :Kwave::MetaData(parent.scope())
{
  foreach(const QString & key, parent.keys()) {
    setProperty(key, parent.property(key));
  }
}

//***************************************************************************
LabelInstant::~LabelInstant()
{
}

//***************************************************************************
void LabelInstant::moveTo(sample_index_t position)
{
    // TODO: ensure that it cannot move before and behind the neighbours
    setProperty(Kwave::MetaData::STDPROP_POS, position);
}

//***************************************************************************
sample_index_t LabelInstant::pos() const
{
    qDebug("LabelInstant::pos(), %p\n", this);
    return static_cast<sample_index_t>(property(Kwave::MetaData::STDPROP_POS).toULongLong());
}

//***************************************************************************
void LabelInstant::rename(const QString &name)
{
    if (name.length())
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
    else
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QVariant());
}

//***************************************************************************
QString LabelInstant::name() const
{
    return property(Kwave::MetaData::STDPROP_DESCRIPTION).toString();
}

//***************************************************************************
//***************************************************************************

LabelRegion::LabelRegion(sample_index_t beg, sample_index_t end, const QString &name)
    :Kwave::MetaData(Kwave::MetaData::Range)
{
    setProperty(LABELPROP_TYPE, LABEL_TYPE);
    setProperty(Kwave::MetaData::STDPROP_TYPE, METADATA_TYPE);
    setProperty(Kwave::MetaData::STDPROP_START, beg);
    setProperty(Kwave::MetaData::STDPROP_END, end);
    if (name.length())
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
}

//***************************************************************************
LabelRegion::LabelRegion(Kwave::MetaData & parent)
    :Kwave::MetaData(parent.scope())
{
  foreach(const QString & key, parent.keys()) {
    setProperty(key, parent.property(key));
  }
}

//***************************************************************************
LabelRegion::~LabelRegion()
{
}

//***************************************************************************
sample_index_t LabelRegion::beg() const
{
    return static_cast<sample_index_t>(property(Kwave::MetaData::STDPROP_START).toULongLong());
}

//***************************************************************************
sample_index_t LabelRegion::end() const
{
    return static_cast<sample_index_t>(property(Kwave::MetaData::STDPROP_END).toULongLong());
}

//***************************************************************************
void LabelRegion::rename(const QString &name)
{
    if (name.length())
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
    else
        setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, QVariant());
}

//***************************************************************************
QString LabelRegion::name() const
{
    return property(Kwave::MetaData::STDPROP_DESCRIPTION).toString();
}

//***************************************************************************
//***************************************************************************
