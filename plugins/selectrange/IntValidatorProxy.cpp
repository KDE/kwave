/***************************************************************************
  IntValidatorProxy.cpp  -  QIntValidator that emits a signal on changes
                             -------------------
    begin                : Sun Jun 16 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "IntValidatorProxy.h"

//***************************************************************************
#if KDE_VERSION_MAJOR < 3
IntValidatorProxy::IntValidatorProxy(QWidget *parent, const char *name)
#else
IntValidatorProxy::IntValidatorProxy(QObject *parent, const char *name)
#endif
    :QIntValidator(parent, name)
{
}

//***************************************************************************
IntValidatorProxy::~IntValidatorProxy()
{
}

//***************************************************************************
QValidator::State IntValidatorProxy::validate(QString &s, int &i) const
{
    QValidator::State state = QIntValidator::validate(s, i);

    if (state == Acceptable) {
	int value = s.toInt();
	((IntValidatorProxy*)this)->emit valueChanged(value);
    }
    return state;
}

//***************************************************************************
//***************************************************************************
