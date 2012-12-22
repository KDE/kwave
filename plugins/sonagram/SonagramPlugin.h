/***************************************************************************
       SonagramPlugin.h  -  plugin that shows a sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _SONAGRAM_PLUGIN_H_
#define _SONAGRAM_PLUGIN_H_

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QString>

#include "libkwave/Plugin.h"
#include "libkwave/WindowFunction.h"

class QImage;
class QStringList;


namespace Kwave
{
    class MultiTrackReader;
    class OverViewCache;
    class PluginContext;
    class SonagramWindow;
    class StripeInfoPrivate;

    /**
     * plugin that shows a sonagram window
     */
    class SonagramPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:
	/** Constructor */
	SonagramPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~SonagramPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** @see Kwave::Plugin::setup() */
	virtual QStringList *setup(QStringList &previous_params);

	/** @see Kwave::Plugin::start() */
	virtual int start(QStringList &params);

	/**
	 * Runns once until all stripes of the sonagram are
	 * calculated.
	 * @see Kwave::Plugin::run()
	 */
	virtual void run(QStringList params);

    signals:

	void stripeAvailable(Kwave::StripeInfoPrivate *stripe_info);

    private slots:

	/**
	 * Connected to the SonagramWindow's "destroyed()" signal.
	 * @see class SonagramWindow
	 */
	void windowDestroyed();

	/**
	 * Internally used to synchronously insert the data of one
	 * sonagram stripe int the current image and refresh the
	 * display.
	 * DO NOT CALL DIRECTLY!
	 */
	void insertStripe(Kwave::StripeInfoPrivate *stripe_info);

	/**
	 * Updates the overview image under the sonagram
	 */
	void refreshOverview();

    protected:

	/**
	 * interpretes a given parameter list and sets up internal
	 * coordinates accordingly
	 * @param params reference to a QStringList with parameters
	 * @return 0 if ok, or an error code if failed
	 */
	int interpreteParameters(QStringList &params);

    private:

	/**
	 * Creates a new image for the current processing.
	 * If an old image exists, it will be deleted first, a new image
	 * will not be created if either width or height is zero.
	 * The image will get 8 bits depth and use a color or a greyscale
	 * palette.
	 * @param width number of horizontal pixels (stripes = signal
	 *        length / fft points, rounded up) [1..32767]
	 * @param height number of vertical pixels (= fft points / 2) [1..32767]
	 */
	void createNewImage(const unsigned int width,
	    const unsigned int height);

	/**
	 * Calculates the fft for one stripe. If the input data range runns
	 * over the end of the signal or selection, zeroes will be used instead.
	 * @param source a MultiTrackReader with the samples
	 * @param points number of fft points
	 * @param output reference to an array to receive the output
	 */
	void calculateStripe(Kwave::MultiTrackReader &source, const int points,
	    QByteArray &output);

    private:

	/** the main view of the plugin, a SonagramWindow */
	Kwave::SonagramWindow *m_sonagram_window;

	/** list of selected channels */
	QList<unsigned int> m_selected_channels;

	/** first sample of the selection, inclusive */
	sample_index_t m_first_sample;

	/** last sample of the selection, inclusive */
	sample_index_t m_last_sample;

	/** number of stripes (= width of the image in pixels) */
	unsigned int m_stripes;

	/** number of fft points */
	unsigned int m_fft_points;

	/** index of the window function */
	Kwave::window_function_t m_window_type;

	/** if true, use color display, else use greyscale */
	bool m_color;

	/** if true, update the sonagram if the signal changes */
	bool m_track_changes;

	/** if true, update the sonagram if the selection changed */
	bool m_follow_selection;

	/** stores the image that is currently in process */
	QImage m_image;

	/** cache with the current signal overview */
	Kwave::OverViewCache *m_overview_cache;

    };
}

#endif /* _SONAGRAM_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
