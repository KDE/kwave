/***************************************************************************
     PitchShiftPlugin.h  -  plugin for modifying the "pitch_shift"
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

#ifndef _PITCH_SHIFT_PLUGIN_H_
#define _PITCH_SHIFT_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "libkwave/KwavePlugin.h"

class QStringList;

class PitchShiftPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    PitchShiftPlugin(PluginContext &context);

    /** Destructor */
    virtual ~PitchShiftPlugin();

    /**
     * Shows a dialog for selecting a pitch shift
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the amplification operation */
    virtual void run(QStringList);

    /** Aborts the process (if running). */
    virtual int stop();

protected slots:

    /**
     * Called when the parameters of the pre-listen have changed
     * @param speed the speed factor, floating point
     * @param frequency the frequency parameter in Hz
     */
    void setValues(double speed, double frequency);

    /** Start the pre-listening */
    void startPreListen();

    /** Stop the pre-listening */
    void stopPreListen();
    
protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:
    /** List of parameters */
    QStringList m_params;

    /** speed factor */
    double m_speed;

    /** base frequency, @see aRts documentation */
    double m_frequency;

    /** mode for selecting speed (factor or percentage) */
    bool m_percentage_mode;
    
    /** flag for stopping the process */
    bool m_stop;

    /** flag for indicating pre-listen mode */
    bool m_listen;
    
};

#endif /* _PITCH_SHIFT_PLUGIN_H_ */
