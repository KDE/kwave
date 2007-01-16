/***************************************************************************
    InvertableSpinBox.h  -  a spinbox with invertable spin buttons
                             -------------------
    begin                : Sun Jan 12 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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
#include <qspinbox.h>

#ifndef _INVERTABLE_SPIN_BOX_H_
#define _INVERTABLE_SPIN_BOX_H_

class QWidget;

class InvertableSpinBox: public QSpinBox
{
    Q_OBJECT
public:

    /** Constructor */
    InvertableSpinBox(QWidget *parent, const char *name);

    /** Destructor */
    virtual ~InvertableSpinBox() {};

    /** enable/disable inverse mode */
    virtual void setInverse(bool inverse);

    /** returns the current inversion state */
    virtual bool inverse() { return m_inverse; };

public slots:

    /**
     * calls QSpinBox::stepUp() in normal mode and
     * QSpinBox::stepDown() in inverse mode
     */
    virtual void stepUp();

    /**
     * calls QSpinBox::stepDown() in normal mode and
     * QSpinBox::stepUp() in inverse mode
     */
    virtual void stepDown();

protected slots:

    /** checks for range in inverse mode */
    void checkValueChange(int value);

private:

    /** if true, spin buttons work in inverse mode */
    bool m_inverse;
    
};

#endif /* _INVERTABLE_SPIN_BOX_H_ */
