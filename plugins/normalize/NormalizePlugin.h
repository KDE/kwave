/***************************************************************************
      NormalizePlugin.h  -  plugin for level normalizing
                             -------------------
    begin                : Fri May 01 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    original algorithms  : (C) 1999-2005 Chris Vaill <chrisvaill at gmail>
                           taken from "normalize-0.7.7"
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NORMALIZE_PLUGIN_H
#define NORMALIZE_PLUGIN_H

#include "config.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    class SampleReader;

    /**
     * This is a two-pass plugin that determines the average volume level
     * of a signal and then calls the volume plugin to adjust the volume.
     */
    class NormalizePlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit NormalizePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~NormalizePlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * normalizes the volume
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

    private:
	typedef struct {
	    QVector<double> fifo; /**< FIFO for power values */
	    unsigned int    wp;   /**< FIFO write pointer */
	    unsigned int    n;    /**< number of elements in the FIFO */
	    double          sum;  /**< sum of queued power values */
	    double          max;  /**< maximum power value */
	} Average;

	/**
	 * get the maximum power level of the input
	 */
	double getMaxPower(Kwave::MultiTrackReader &source);

	/**
	 * calculate the maximum power of one track
	 *
	 * @param reader reference to a SampleReader to read from
	 * @param average reference to smoothing information
	 * @param window_size length of the sliding window for volume detection
	 */
	void getMaxPowerOfTrack(Kwave::SampleReader *reader,
	                        Kwave::NormalizePlugin::Average *average,
	                        unsigned int window_size);

    };
}

#endif /* NORMALIZE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
