/***************************************************************************
         VolumeDialog.h  -  dialog for the "volume" plugin
                             -------------------
    begin                : Sun Oct 27 2002
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _VOLUME_DIALOG_H_
#define _VOLUME_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include "VolumeDlg.uih.h"

class QStringList;

class VolumeDialog: public VolumeDlg
{
    Q_OBJECT
public:

    /** Constructor */
    VolumeDialog(QWidget *parent);

    /** Destructor */
    virtual ~VolumeDialog();

    /** Returns a command string for the curve */
    QString getCommand();

    /** Sets the curve parameters and points from a list of parameters */
    void setParams(QStringList &params);

protected slots:

    /** called when the mode radio buttons changed */
    void modeChanged(bool);

    /** called when the slider's position has changed */
    void sliderChanged(int pos);

    /** called when the spinbox value has changed */
    void spinboxChanged(int pos);
    
protected:

    /**
     * Mode for amplification selection
     * by factor (x1, x2, x5,...), by percentage or by decibel
     */
    typedef enum {
	MODE_FACTOR  = 0,
	MODE_PERCENT = 1,
	MODE_DECIBEL = 2
    } Mode;

    /** Sets a new volume selection mode */
    void setMode(Mode mode);

    /** Update the slider position and the spinbox value */
    void updateDisplay(double value);
    
private:    

    /** amplification factor */
    double m_factor;

    /**
     * current mode for amplification selection
     */
    Mode m_mode;

    /** if false, ignore the signals of slider and spinbox */
    bool m_enable_updates;
};

#endif /* _VOLUME_DIALOG_H_ */
