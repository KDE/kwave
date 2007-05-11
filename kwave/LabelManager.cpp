/***************************************************************************
          LabelManager.cpp  -  manager class for kwave's labels
			     -------------------
    begin                : Mon Jun 2 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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
#include <kapp.h>

#include "LabelManager.h"

//***************************************************************************
LabelManager::LabelManager()
    :QObject()
{
}

//***************************************************************************
bool LabelManager::isOK()
{
    return true;
}

//***************************************************************************
LabelManager::~LabelManager()
{
}

//***************************************************************************
#include "LabelManager.moc"
//***************************************************************************
//***************************************************************************
