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

#include "libgui/FilterPlugin.h"

namespace Kwave
{
    class NoisePlugin: public Kwave::FilterPlugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	NoisePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~NoisePlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** Creates the setup dialog and connects it's signals */
	virtual Kwave::PluginSetupDialog *createDialog(QWidget *parent);

	/**
	 * Creates a multi-track filter with the given number of tracks
	 * @param tracks number of tracks that the filter should have
	 * @return pointer to the filter or null if failed
	 */
	virtual Kwave::SampleSource *createFilter(unsigned int tracks);

	/**
	 * Returns true if the parameters have changed during pre-listen.
	 */
	virtual bool paramsChanged();

	/**
	 * Update the filter with new parameters if it has changed
	 * changed during the pre-listen.
	 * @param filter the Kwave::SampleSource to be updated, should be the
	 *               same one as created with createFilter()
	 * @param force if true, even update if no settings have changed
	 */
	virtual void updateFilter(Kwave::SampleSource *filter,
	                          bool force = false);

	/**
	 * Returns a verbose name of the performed action. Used for giving
	 * the undo action a readable name.
	 */
	virtual QString actionName();

    protected:

	/** Reads values from the parameter list */
	int interpreteParameters(QStringList &params);

    protected slots:

	/**
	 * called when the noise level setting changed during pre-listen
	 * @param level
	 */
	void setNoiseLevel(double level);

    private:

	/** noise level, as linear factor ]0 ... 1.0] */
	double m_level;

	/** last value of m_level */
	double m_last_level;

    };
}

#endif /* _NOISE_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
