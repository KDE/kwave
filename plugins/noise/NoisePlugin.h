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

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/Plugin.h"

namespace Kwave
{
    class NoisePlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	NoisePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~NoisePlugin() {}

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * Fills the selected area with noise
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

    };
}

#endif /* _NOISE_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
