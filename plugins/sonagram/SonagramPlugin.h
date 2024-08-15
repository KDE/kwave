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

#ifndef SONAGRAM_PLUGIN_H
#define SONAGRAM_PLUGIN_H

#include "config.h"

#include <fftw3.h>

#include <QBitArray>
#include <QByteArray>
#include <QFuture>
#include <QList>
#include <QMutex>
#include <QQueue>
#include <QReadWriteLock>
#include <QRecursiveMutex>
#include <QString>
#include <QTimer>
#include <QUuid>

#include "libkwave/FixedPool.h"
#include "libkwave/Plugin.h"
#include "libkwave/WindowFunction.h"

class QImage;

/** maximum number of FFT points */
#define MAX_FFT_POINTS 32767

/** maximum number of concurrent FFT jobs */
#define MAX_FFT_JOBS 256

/** maximum number of slices (width of the image) */
#define MAX_SLICES 32767

namespace Kwave
{
    class OverViewCache;
    class SelectionTracker;
    class SonagramWindow;

    /**
     * plugin that shows a sonagram window
     */
    class SonagramPlugin: public Kwave::Plugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        SonagramPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        virtual ~SonagramPlugin() override;

        /** @see Kwave::Plugin::setup() */
        virtual QStringList *setup(QStringList &previous_params)
            override;

        /** @see Kwave::Plugin::start() */
        virtual int start(QStringList &params) override;

        /**
         * Runns once until all slices of the sonagram are
         * calculated.
         * @param params list of strings with parameters
         * @see Kwave::Plugin::run()
         */
        virtual void run(QStringList params) override;

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
            unsigned char m_result[MAX_FFT_POINTS];
        } Slice;

    signals:

        /**
         * emitted when a new slice has been calculated in calculateSlice()
         * @param slice the slice data container, including result
         */
        void sliceAvailable(Kwave::SonagramPlugin::Slice *slice);

    private slots:

        /**
         * validate the sonagram by calling makeAllValid in a background thread
         */
        void validate();

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

        /**
         * Connected to the selection tracker's sigTrackInserted.
         * @param track_id unique ID of the track
         * @see SelectionTracker::sigTrackInserted
         */
        void slotTrackInserted(const QUuid &track_id);

        /**
         * Connected to the selection tracker's sigTrackInserted.
         * @param track_id unique ID of the track
         * @see SelectionTracker::sigTrackDeleted
         */
        void slotTrackDeleted(const QUuid &track_id);

        /**
         * Connected to the selection tracker's sigInvalidated.
         * @param track_id UUID of the track or null for "all tracks"
         * @param first index of the first invalidated sample
         * @param last index of the last invalidated sample
         */
        void slotInvalidated(const QUuid *track_id,
                             sample_index_t first,
                             sample_index_t last);

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
         * will be run in a background thread to make all stripes
         * valid.
         */
        void makeAllValid();

        /**
         * Requests an update of the sonagram or portions of it
         */
        void requestValidation();

        /**
         * do the FFT calculation on a slice
         * @param slice structure with the input data and output buffer
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

        /** selection tracker */
        Kwave::SelectionTracker *m_selection;

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

        /** bit field with "is valid" a flag for each stripe */
        QBitArray m_valid;

        /** lock used for tracking running background jobs */
        QReadWriteLock m_pending_jobs;

        /** lock to protect the job list (m_valid) */
        QRecursiveMutex m_lock_job_list;

        /** the currently running background job */
        QFuture<void> m_future;

        /** timer for refreshing the sonagram */
        QTimer m_repaint_timer;
    };
}

#endif /* SONAGRAM_PLUGIN_H */

//***************************************************************************
//***************************************************************************
