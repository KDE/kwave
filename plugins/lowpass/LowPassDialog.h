/***************************************************************************
        LowPassDialog.h  -  dialog for the "lowpass" plugin
                             -------------------
    begin                : Fri Mar 07 2003
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

#ifndef _LOW_PASS_DIALOG_H_
#define _LOW_PASS_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include "LowPassDlg.uih.h"

class QStringList;

class LowPassDialog: public LowPassDlg
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent parent widget
     * @param sample_rate sample rate of the audio data to be processed,
     *                    needed for determining the allowed range of
     *                    the cutoff frequency 
     */
    LowPassDialog(QWidget *parent, double sample_rate);

    /** Destructor */
    virtual ~LowPassDialog();

    /** Returns the parameters as string list */
    QStringList params();

    /** Sets the from a list of parameters */
    void setParams(QStringList &params);

protected slots:

    /** called when the spinbox or spinbox value has changed */
    void valueChanged(int pos);
    
protected:

    /** Update the graphic display */
    void updateDisplay();
 
private:    

    /** the cutoff frequency in Hz */
    double m_frequency;

    /** sample rate of the signal in samples/sec */
    double m_sample_rate;
    
};

#endif /* _LOW_PASS_DIALOG_H_ */
