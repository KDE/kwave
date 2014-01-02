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

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QQueue>
#include <QtCore/QReadWriteLock>
#include <QtCore/QString>

#include "libkwave/FixedPool.h"
#include "libkwave/Plugin.h"
#include "libkwave/WindowFunction.h"

class QImage;
class QStringList;

/** maximum number of FFT points */
#define MAX_FFT_POINTS 32767

/** maximum number of concurrent FFT jobs */
#define MAX_FFT_JOBS 256

namespace Kwave
{
    class MultiTrackReader;
    class OverViewCache;
    class PluginContext;
    class SonagramWindow;

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
	 * Runns once until all slices of the sonagram are
	 * calculated.
	 * @param params list of strings with parameters
	 * @see Kwave::Plugin::run()
	 */
	virtual void run(QStringList params);

    private:

	/** internal data of a slice */
	typedef struct {
	    /** index of the slice */
	    unsigned int m_index;

	    /** array with input samples */
	    double m_input[MAX_FFT_POINTS];

	    /** FFT output data */
	    fftw_complex m_output[MAX_FFT_POINTS];

	    /** rendered FFT result data */
	    char m_result[MAX_FFT_POINTS];
	} Slice;

    signals:

	/**
	 * emitted when a new slice has been calculated in calculateSlice()
	 * @param slice the slice data container, including result
	 */
	void sliceAvailable(Kwave::SonagramPlugin::Slice *slice);

    private slots:

	/**
	 * Connected to the SonagramWindow's "destroyed()" signal.
	 * @see class SonagramWindow
	 */
	void windowDestroyed();

	/**
	 * Internally used to synchronously insert the data of one
	 * sonagram slice int the current image and refresh the
	 * display.
	 * @note DO NOT CALL DIRECTLY!
	 * @param slice a slice data container, including result
	 */
	void insertSlice(Kwave::SonagramPlugin::Slice *slice);

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
	 * do the FFT calculation on a slice
	 * @param slice structure with the input data and output buffer
	 * @return a byte array with rendered FFT data
	 */
	void calculateSlice(Kwave::SonagramPlugin::Slice *slice);

	/**
	 * Creates a new image for the current processing.
	 * If an old image exists, it will be deleted first, a new image
	 * will not be created if either width or height is zero.
	 * The image will get 8 bits depth and use a color or a greyscale
	 * palette.
	 * @param width number of horizontal pixels (slices = signal
	 *        length / fft points, rounded up) [1..32767]
	 * @param height number of vertical pixels (= fft points / 2) [1..32767]
	 */
	void createNewImage(const unsigned int width,
	    const unsigned int height);

    private:

	/** the main view of the plugin, a SonagramWindow */
	Kwave::SonagramWindow *m_sonagram_window;

	/** list of selected channels */
	QList<unsigned int> m_selected_channels;

	/** first sample of the selection, inclusive */
	sample_index_t m_first_sample;

	/** last sample of the selection, inclusive */
	sample_index_t m_last_sample;

	/** number of slices (= width of the image in pixels) */
	unsigned int m_slices;

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

	/** pool of slices */
	Kwave::FixedPool<MAX_FFT_JOBS, Slice> m_slice_pool;

	/** lock used for tracking running background jobs */
	QReadWriteLock m_pending_jobs;

    };
}

#endif /* _SONAGRAM_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
