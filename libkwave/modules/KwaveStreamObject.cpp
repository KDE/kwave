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

#include <QString>
#include <QVariant>

#include "libkwave/modules/KwaveStreamObject.h"

//***************************************************************************
Kwave::StreamObject::StreamObject(QObject *parent)
    :QObject(parent), m_lock_set_attribute(QMutex::Recursive)
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

    for (unsigned int track = 0; track < tracks(); track++) {
	Kwave::StreamObject *obj = (*this)[track];
	if (!obj) continue;

	// temporary establish a signal->slot connection
	QObject::connect(this, SIGNAL(attributeChanged(const QVariant)),
                         obj, attribute.toAscii());

	// emit the new value through our own signal
	emit attributeChanged(value);

	// remove the temporary signal->slot connection
	QObject::disconnect(this, SIGNAL(attributeChanged(const QVariant)),
                            obj, attribute.toAscii());
    }
}

//***************************************************************************
#include "KwaveStreamObject.moc"
//***************************************************************************
//***************************************************************************
