/*************************************************************************
        ReversePlugin.h  -  reverses the current selection
                             -------------------
    begin                : Tue Jun 09 2009
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

#ifndef REVERSE_PLUGIN_H
#define REVERSE_PLUGIN_H

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * @class ReversePlugin
     * Reverts the current selection by exchanging blocks of samples
     * from front and back and reversing their content
     */
    class ReversePlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit ReversePlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~ReversePlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * reverses the selection
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

    private slots:

	/**
	 * multiplies the progress by factor two and
	 * calls Kwave::Plugin::updateProgress
	 */
	virtual void updateProgress(qreal progress);

    private:

	/**
	 * common slice parameters, which can be passed to all worker
	 * threads
	 */
	typedef struct {
	    sample_index_t m_first;      /**< index of the first sample */
	    sample_index_t m_last;       /**< index of the last sample  */
	    unsigned int   m_block_size; /**< block size [samples]      */
	} SliceParams;

	/**
	 * reverse a slice of samples
	 * @param track index of the track
	 * @param src_a reader for reading from the start (forward)
	 * @param src_b reader fro reading from the end (backwards)
	 * @param params common slice parameters (first/last/block size)
	 */
	void reverseSlice(unsigned int track,
	                  Kwave::SampleReader *src_a,
	                  Kwave::SampleReader *src_b,
	                  const Kwave::ReversePlugin::SliceParams &params);

	/** reverses the content of an array of samples */
	void reverse(Kwave::SampleArray &buffer);

    };
}

#endif /* REVERSE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
