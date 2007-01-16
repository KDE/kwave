/***************************************************************************
	    LabelManager.h  -  manager class for kwave's labels
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

#ifndef _LABEL_MANAGER_H_
#define _LABEL_MANAGER_H_

#include "config.h"
#include "qobject.h"

/**
 * Manages a list of labels for kwave, including saving and loading.
 * @author Thomas Eschenbacher
 */
class LabelManager : public QObject
{
    Q_OBJECT

public:    // Public methods
    /**
     * Default constructor
     */
    LabelManager();

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /**
     * Default destructor
     */
    virtual ~LabelManager();

};

#endif // _LABEL_MANAGER_H_
