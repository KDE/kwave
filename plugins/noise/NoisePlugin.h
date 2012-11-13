/*************************************************************************
    NoisePlugin.h  -  overwrites the selected range of samples with noise
                             -------------------
    begin                : Wed Dec 12 2001
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

#ifndef _NOISE_PLUGIN_H_
#define _NOISE_PLUGIN_H_

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"

class NoisePlugin: public Kwave::Plugin
{
    Q_OBJECT
public:

    /** Constructor */
    NoisePlugin(const Kwave::PluginContext &c);

    /** Destructor */
    virtual ~NoisePlugin() {};

    /** Fills the selected area with noise */
    virtual void run(QStringList);

};

#endif /* _NOISE_PLUGIN_H_ */

