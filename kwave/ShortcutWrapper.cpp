/***************************************************************************
          ShortcutWrapper.cpp  -  wrapper for keyboard shortcuts
                             -------------------
    begin                : Sat Jan 12 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include <QKeySequence>
#include <QObject>
#include <QWidget>

#include "ShortcutWrapper.h"

//***************************************************************************
Kwave::ShortcutWrapper::ShortcutWrapper(QWidget *parent,
                                        const QKeySequence &key,
                                        int id)
    :QShortcut(key, parent), m_id(id)
{
    connect(this, SIGNAL(activated()),
            this, SLOT(triggered()));
}

//***************************************************************************
Kwave::ShortcutWrapper::~ShortcutWrapper()
{
}

//***************************************************************************
void Kwave::ShortcutWrapper::triggered()
{
    emit activated(m_id);
}

//***************************************************************************
//***************************************************************************
