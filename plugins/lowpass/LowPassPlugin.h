/***************************************************************************
        LowPassPlugin.h  -  Plugin for simple lowpass filtering
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

#ifndef _LOW_PASS_PLUGIN_H_
#define _LOW_PASS_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "libkwave/KwavePlugin.h"

class QStringList;

class LowPassPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    LowPassPlugin(PluginContext &context);

    /** Destructor */
    virtual ~LowPassPlugin();

    /**
     * Shows a dialog for selecting a cutoff frequency.
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the amplification operation */
    virtual void run(QStringList);

    /** Aborts the process (if running). */
    virtual int stop();

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:
    /** List of parameters */
    QStringList m_params;

    /** cutoff frequency [Hz] */
    double m_frequency;

    /** flag for stopping the process */
    bool m_stop;

};

#endif /* _LOW_PASS_PLUGIN_H_ */
