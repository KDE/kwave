/***************************************************************************
       BitrateSpinBox.h  -  spinbox for selecting a bitrate for MP3 or Ogg
			     -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _BITRATE_SPIN_BOX_H_
#define _BITRATE_SPIN_BOX_H_

#include "config.h"
#include <qobject.h>
#include <qspinbox.h>

class BitrateSpinBox: public QSpinBox
{
    Q_OBJECT
public:
    /** Constructor */
    BitrateSpinBox(QWidget *parent, const char *name);

    /** Destructor */
    ~BitrateSpinBox();

    /** sets a list of allowed bitrates */
    virtual void allowRates(const QValueList<int> &list);

signals:

    /** emitted when the value changed and snapped in to a bitrate */
    void snappedIn(int bitrate);
    
public slots:

    /** snaps in to a new value */
    virtual void snapIn(int value);
    
    /** select the next higher bitrate */
    virtual void stepUp();

    /** select the next lower bitrate */
    virtual void stepDown();

protected:

    /** find the nearest bitrate index of a current position */
    int nearestIndex(int rate);

private:

    /** list of allowed bitrates, sorted ascending */
    QValueList<int> m_rates;
    
};

#endif /* _BITRATE_SPIN_BOX_H_ */
