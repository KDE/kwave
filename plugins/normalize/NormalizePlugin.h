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

#ifndef _NORMALIZE_PLUGIN_H_
#define _NORMALIZE_PLUGIN_H_

#include "config.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include "libkwave/KwavePlugin.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"

//***************************************************************************
/**
 * @class NormalizePlugin
 * This is a two-pass plugin that determines the average volume level
 * of a signal and then calls the volume plugin to adjust the volume.
 */
class NormalizePlugin: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    NormalizePlugin(const PluginContext &c);

    /** Destructor */
    virtual ~NormalizePlugin();

    /** normalizes the volume */
    virtual void run(QStringList);

private:

    typedef struct {
	QVector<double> fifo; /**< FIFO for power values */
	unsigned int    wp;   /**< FIFO write pointer */
	unsigned int    n;    /**< number of elements in the FIFO */
	double          sum;  /**< sum of queued power values */
    } average_t;

    /**
     * get the maximum power level of the input
     */
    double getMaxPower(MultiTrackReader &source);

};

#endif /* _NORMALIZE_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
