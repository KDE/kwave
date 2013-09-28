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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/Plugin.h"

namespace Kwave
{
    class VolumePlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	VolumePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~VolumePlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * Shows a dialog for selecting a volume.
	 * @see Kwave::Plugin::setup
	 */
	virtual QStringList *setup(QStringList &previous_params);

	/**
	 * Does the amplification operation
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

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
}

#endif /* _VOLUME_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
