/***************************************************************************
     PitchShiftDialog.h  -  dialog for the "pitch_shift" plugin
                             -------------------
    begin                : Sun Mar 23 2003
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

#ifndef _PITCH_SHIFT_DIALOG_H_
#define _PITCH_SHIFT_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include "libkwave/KwavePluginSetupDialog.h"
#include "PitchShiftDlg.h"

class QDialog;
class QStringList;

class PitchShiftDialog: public PitchShiftDlg,
                        public KwavePluginSetupDialog
{
    Q_OBJECT
public:

    /** Constructor */
    PitchShiftDialog(QWidget *parent);

    /** Destructor */
    virtual ~PitchShiftDialog();

    /** Returns the parameters as string list */
    virtual QStringList params();

    /** Sets the from a list of parameters */
    virtual void setParams(QStringList &params);

    /** retruns a pointer to this as a QDialog */
    virtual QDialog *dialog() { return this; };

signals:

    /**
     * Emitted whenever the speed or the frequency changes
     * @param speed the speed factor, floating point
     * @param freq the frequency parameter in Hz
     */
    void changed(double speed, double freq);

    /** Pre-listen mode has been started */
    void startPreListen();

    /** Pre-listen mode has been stopped */
    void stopPreListen();

protected slots:

    /** called when the mode radio buttons changed */
    void modeChanged(bool);

    /** called when the speed slider's position has changed */
    void sliderChanged(int pos);

    /** called when the spped spinbox value has changed */
    void spinboxChanged(int pos);

    /** called if the frequency sbinbox value has changed */
    void frequencyChanged(int pos);

    /**
     * called when the "Listen" button has been toggled,
     * to start or stop the pre-listen mode
     */
    void listenToggled(bool listen);

    /**
     * called when the pre-listen mode stopped/aborted
     */
    void listenStopped();

protected:

    /**
     * Mode for amplification selection
     * by factor (x1, x2, x5,...) or by percentage
     */
    typedef enum {
	MODE_FACTOR  = 0,
	MODE_PERCENT = 1
    } Mode;

    /** Sets a new volume selection mode */
    void setMode(Mode mode);

    /** Update the speed slider position and the speed spinbox value */
    void updateSpeed(double speed);

private:

    /** speed factor */
    double m_speed;

    /** base frequency, @see aRts documentation */
    double m_frequency;

    /** mode for selecting speed (factor or percentage) */
    Mode m_mode;

    /** if false, ignore the signals of slider and spinbox */
    bool m_enable_updates;
};

#endif /* _PITCH_SHIFT_DIALOG_H_ */
