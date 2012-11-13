/***************************************************************************
    VolumePlugin.h  -  Plugin for adjusting a signal's volume
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

#ifndef _VOLUME_PLUGIN_H_
#define _VOLUME_PLUGIN_H_

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"

class VolumePlugin: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    VolumePlugin(const Kwave::PluginContext &context);

    /** Destructor */
    virtual ~VolumePlugin();

    /**
     * Shows a dialog for selecting a volume.
     * @see Kwave::Plugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the amplification operation */
    virtual void run(QStringList);

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:
    /** List of parameters */
    QStringList m_params;

    /** amplification factor */
    float m_factor;

    /** mode for amplification selection */
    int m_mode;

};

#endif /* _VOLUME_PLUGIN_H_ */
