/*************************************************************************
    KwaveStreamObject.cpp  -  base class with a generic sample source/sink
                             -------------------
    begin                : Thu Nov 10 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include <qstring.h>
#include <qvariant.h>

#include "libkwave/KwaveStreamObject.h"

//***************************************************************************
Kwave::StreamObject::StreamObject(QObject *parent, const char *name)
    :QObject(parent, name),
    m_lock_set_attribute(true /* recursive! */)
{
}

//***************************************************************************
Kwave::StreamObject::~StreamObject()
{
}

//***************************************************************************
void Kwave::StreamObject::setAttribute(const QString &attribute,
                                       const QVariant &value)
{
    QMutexLocker lock(&m_lock_set_attribute);

    // temporary establish a signal->slot connection
    QObject::connect(this, SIGNAL(attributeChanged(const QVariant &)),
                     this, attribute);

    // emit the new value through our own signal
    emit attributeChanged(value);

    // remove the temporary signal->slot connection
    QObject::disconnect(this, SIGNAL(attributeChanged(const QVariant &)),
                        this, attribute);
}

//***************************************************************************
#include "KwaveStreamObject.moc"
//***************************************************************************
//***************************************************************************
