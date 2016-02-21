/*************************************************************************
     SampleRatePlugin.h  -  sample rate conversion
                             -------------------
    begin                : Tue Jul 07 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef SAMPLE_RATE_PLUGIN_H
#define SAMPLE_RATE_PLUGIN_H

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * @class SampleRatePlugin
     * Change the sample rate of a signal
     */
    class SampleRatePlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit SampleRatePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~SampleRatePlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * changes the sample rate
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

    protected:

	/**
	 * reads values from the parameter list
	 * @param params list of strings with parameters
	 * @return 0 if succeeded or negative error code if failed
	 */
	int interpreteParameters(QStringList &params);

    private:

	/** list of parameters */
	QStringList m_params;

	/** new sample rate */
	double m_new_rate;

	/** if true, ignore selection and change whole signal */
	bool m_whole_signal;

    };
}

#endif /* SAMPLE_RATE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
