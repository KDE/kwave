/***************************************************************************
    IntValidatorProxy.h  -  QIntValidator that emits a signal on changes
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

#ifndef _INT_VALIDATOR_PROXY_H_
#define _INT_VALIDATOR_PROXY_H_

#include "config.h"
#include <qobject.h>
#include <qvalidator.h>

class QString;
class QWidget;

#ifdef HAVE_KDEVERSION_H
#include <kdeversion.h>
#else
#define KDE_VERSION_MAJOR 2
#endif

/**
 * @class IntValidatorProxy
 * Like a QIntValidator, but emits a signal valueChanged if the
 * current value has changed and is valid.
 */
class IntValidatorProxy: public QIntValidator
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @see QValidator
     */
#if KDE_VERSION_MAJOR < 2
    IntValidatorProxy(QWidget *parent, const char *name = 0);
#else
    IntValidatorProxy(QObject *parent, const char *name = 0);
#endif

    /** Destructor */
    virtual ~IntValidatorProxy();

    /** @see QValidator::validate */
    virtual QValidator::State validate(QString &s, int &i) const;

signals:
    /**
     * Will be emitted whenever the value of the control has changed
     * and is valid
     */
    void valueChanged(int value);

};

#endif /* _INT_VALIDATOR_PROXY_H_ */
