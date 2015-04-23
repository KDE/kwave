/***************************************************************************
    FilterPlugin.h  -  generic class for filter plugins with setup
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef FILTER_PLUGIN_H
#define FILTER_PLUGIN_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/Plugin.h"
#include "libkwave/PluginSetupDialog.h"

class QStringList;
class QWidget;

namespace Kwave
{

    class SampleSource;
    class SampleSink;

    class KDE_EXPORT FilterPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit FilterPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~FilterPlugin();

	/** Reads values from the parameter list */
	virtual int interpreteParameters(QStringList & /* params */) = 0;

	/**
	 * Creates a setup dialog an returns a pointer to it.
	 */
	virtual Kwave::PluginSetupDialog *createDialog(QWidget * /*parent*/) = 0;

	/**
	 * Creates a multi-track filter with the given number of tracks
	 * @param tracks number of tracks that the filter should have
	 * @return pointer to the filter or null if failed
	 */
	virtual Kwave::SampleSource *createFilter(unsigned int tracks) = 0;

	/**
	 * Shows a dialog for setting up the filter plugin
	 * @see Kwave::Plugin::setup
	 */
	virtual QStringList *setup(QStringList &previous_params);

	/**
	 * Does the filter operation and/or pre-listen
	 * @param params list of strings with parameters
	 */
	virtual void run(QStringList params);

	/**
	 * Returns true if the parameters have changed during pre-listen.
	 * @note this default implementation always returns false.
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
	 * The name must already be localized !
	 */
	virtual QString actionName() = 0;

	/**
	 * Returns a text for the progress dialog if enabled.
	 * (should already be localized)
	 */
	virtual QString progressText();

    signals:

	/**
	 * emitted when the user pressed the cancel button
	 * of the progress dialog
	 * @internal
	 */
	void sigCancelPressed();

    protected slots:

	/** Start the pre-listening */
	void startPreListen();

	/** Stop the pre-listening */
	void stopPreListen();

    private:
	/** List of parameters */
	QStringList m_params;

	/** flag for indicating pre-listen mode */
	bool m_listen;

	/** flag for pausing the process */
	bool m_pause;

	/**
	 * a sample sink, used either for pre-listen
	 * or for normal processing
	 */
	Kwave::SampleSink *m_sink;

    };
}

#endif /* FILTER_PLUGIN_H */

//***************************************************************************
//***************************************************************************
