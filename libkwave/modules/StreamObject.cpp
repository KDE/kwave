/*************************************************************************
       StreamObject.cpp  -  base class with a generic sample source/sink
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

#include "libkwave/modules/StreamObject.h"

/** interactive mode */
bool Kwave::StreamObject::m_interactive = false;

//***************************************************************************
Kwave::StreamObject::StreamObject(QObject *parent)
    :QObject(Q_NULLPTR /*parent*/),
     m_lock_set_attribute(),
     m_canceled(false)
{
    Q_UNUSED(parent)
}

//***************************************************************************
Kwave::StreamObject::~StreamObject()
{
}

//***************************************************************************
void Kwave::StreamObject::setAttribute(const char *attribute,
                                       const QVariant &value)
{
    QMutexLocker lock(&m_lock_set_attribute);

    for (unsigned int track = 0; track < tracks(); track++) {
        Kwave::StreamObject *obj = (*this)[track];
        if (!obj) continue;

        // temporary establish a signal->slot connection
        QObject::connect(this, SIGNAL(attributeChanged(QVariant)),
                         obj, attribute,
                         Qt::DirectConnection);

        // emit the new value through our own signal
        emit attributeChanged(value);

        // remove the temporary signal->slot connection
        QObject::disconnect(this, SIGNAL(attributeChanged(QVariant)),
                            obj, attribute);
    }
}

//***************************************************************************
unsigned int Kwave::StreamObject::blockSize() const
{
    return (m_interactive) ? 8*1024 : 512*1024;
}

//***************************************************************************
void Kwave::StreamObject::setInteractive(bool interactive)
{
    m_interactive = interactive;
}

//***************************************************************************
void Kwave::StreamObject::cancel()
{
    if (!m_canceled) {
        m_canceled = true;
        emit sigCancel();
    }
}

//***************************************************************************
//***************************************************************************
