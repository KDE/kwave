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

#ifndef _SAMPLE_RATE_PLUGIN_H_
#define _SAMPLE_RATE_PLUGIN_H_

#include "config.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Sample.h"

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
	SampleRatePlugin(const Kwave::PluginContext &c);

	/** Destructor */
	virtual ~SampleRatePlugin();

	/** changes the sample rate */
	virtual void run(QStringList);

    protected:

	/** reads values from the parameter list */
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

#endif /* _SAMPLE_RATE_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
