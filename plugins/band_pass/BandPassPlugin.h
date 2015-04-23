/***************************************************************************
        BandPassPlugin.h  -  Plugin for band pass filtering
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BAND_PASS_PLUGIN_H
#define BAND_PASS_PLUGIN_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "libkwave/PluginSetupDialog.h"
#include "libkwave/Plugin.h"

#include "libgui/FilterPlugin.h"

namespace Kwave
{
    class KwaveSampleSource;

    class BandPassPlugin: public Kwave::FilterPlugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit BandPassPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~BandPassPlugin();

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
	virtual int interpreteParameters(QStringList &params);

    protected slots:

	/**
	 * Called when the parameter changed during pre-listen
	 * @param frequency the frequency parameter in Hz
	 */
	void setFreqValue(double frequency);

	/**
	 * Called when the bandwidth parameter changed during pre-listen
	 * @param bw relative bandwidth [0...1]
	 */
	void setBwValue(double bw);

    private:

	/** cutoff frequency [Hz] */
	double m_frequency;

	/** last value of m_frequency */
	double m_last_freq;

	/** bandwidth value [Hz] */
	double m_bw;

	/** last value of m_bw */
	double m_last_bw;

    };
}

#endif /* BAND_PASS_PLUGIN_H */

//***************************************************************************
//***************************************************************************
