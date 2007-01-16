/***************************************************************************
    AmplifyFreePlugin.h  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
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

#ifndef _AMPLIFY_FREE_PLUGIN_H_
#define _AMPLIFY_FREE_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "libkwave/Curve.h"
#include "libkwave/KwavePlugin.h"

class QStringList;

/** @todo add support for logarithmic scale */
class AmplifyFreePlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    AmplifyFreePlugin(const PluginContext &context);

    /** Destructor */
    virtual ~AmplifyFreePlugin();

    /**
     * Shows a dialog for editing the amplification curve and emits a command
     * for applying the curve if OK has been pressed.
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the fade operation */
    virtual void run(QStringList);

    /** Aborts the process (if running). */
    virtual int stop();

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:
    /** List of parameters */
    QStringList m_params;

    /** curve used for interpolation */
    Curve m_curve;

    /** flag for stopping the process */
    bool m_stop;

};

#endif /* _AMPLIFY_FREE_PLUGIN_H_ */
