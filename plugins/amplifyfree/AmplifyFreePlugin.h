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

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/Curve.h"
#include "libkwave/KwavePlugin.h"

/** @todo add support for logarithmic scale */
class AmplifyFreePlugin: public Kwave::Plugin
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
     * @see Kwave::Plugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the fade operation */
    virtual void run(QStringList);

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:

    /** List of parameters */
    QStringList m_params;

    /** curve used for interpolation */
    Curve m_curve;

};

#endif /* _AMPLIFY_FREE_PLUGIN_H_ */
