/***************************************************************************
              SonagramPlugin.cpp  -  plugin that shows a sonagram window
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

#include "config.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>

#include <QApplication>
#include <QColor>
#include <QFutureSynchronizer>
#include <QImage>
#include <QMutexLocker>
#include <QString>
#include <QtConcurrentRun>

#include "libkwave/GlobalLock.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"
#include "libkwave/WindowFunction.h"

#include "libgui/OverViewCache.h"
#include "libgui/SelectionTracker.h"

#include "SonagramDialog.h"
#include "SonagramPlugin.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(sonagram, SonagramPlugin)

/**
 * interval for limiting the number of repaints per second [ms]
 */
#define REPAINT_INTERVAL 500

//***************************************************************************
Kwave::SonagramPlugin::SonagramPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_sonagram_window(0),
     m_selection(0),
     m_slices(0), m_fft_points(0),
     m_window_type(Kwave::WINDOW_FUNC_NONE), m_color(true),
     m_track_changes(true), m_follow_selection(false), m_image(),
     m_overview_cache(0), m_slice_pool(), m_valid(MAX_SLICES, false),
     m_pending_jobs(), m_lock_job_list(QMutex::Recursive), m_future(),
     m_repaint_timer()
{
    i18n("Sonagram");

    // connect the output ouf the sonagram worker thread
    connect(this, SIGNAL(sliceAvailable(Kwave::SonagramPlugin::Slice*)),
            this, SLOT(insertSlice(Kwave::SonagramPlugin::Slice*)),
            Qt::QueuedConnection);

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(validate()));
}

//***************************************************************************
Kwave::SonagramPlugin::~SonagramPlugin()
{
    m_repaint_timer.stop();

    if (m_sonagram_window) delete m_sonagram_window;
    m_sonagram_window = 0;

    if (m_selection) delete m_selection;
    m_selection = 0;
}

//***************************************************************************
QStringList *Kwave::SonagramPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    Kwave::SonagramDialog *dlg = new(std::nothrow) Kwave::SonagramDialog(*this);
    Q_ASSERT(dlg);
    if (!dlg) return 0;

    dlg->setWindowFunction(m_window_type);
    dlg->setColorMode(m_color ? 1 : 0);
    dlg->setTrackChanges(m_track_changes);
    dlg->setFollowSelection(m_follow_selection);

    if (dlg->exec() == QDialog::Accepted) {
	result = new QStringList();
	Q_ASSERT(result);
	if (result) dlg->parameters(*result);
    };

    delete dlg;
    return result;
}

//***************************************************************************
int Kwave::SonagramPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return -EINVAL;

    param = params[0];
    m_fft_points = param.toUInt(&ok);
    if (!ok) return -EINVAL;
    if (m_fft_points > MAX_FFT_POINTS) m_fft_points = MAX_FFT_POINTS;

    param = params[1];
    m_window_type = Kwave::WindowFunction::findFromName(param);
    if (!ok) return -EINVAL;

    param = params[2];
    m_color = (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

    param = params[3];
    m_track_changes = (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

    param = params[4];
    m_follow_selection = (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
int Kwave::SonagramPlugin::start(QStringList &params)
{
    // clean up leftovers from last run
    if (m_sonagram_window) delete m_sonagram_window;
    m_sonagram_window = 0;
    if (m_selection)       delete m_selection;
    m_selection = 0;
    if (m_overview_cache)  delete m_overview_cache;
    m_overview_cache = 0;

    Kwave::SignalManager &sig_mgr = signalManager();

    // interprete parameter list and abort if it contains invalid data
    int result = interpreteParameters(params);
    if (result) return result;

    // create an empty sonagram window
    m_sonagram_window = new(std::nothrow)
	Kwave::SonagramWindow(parentWidget(), signalName());
    Q_ASSERT(m_sonagram_window);
    if (!m_sonagram_window) return -ENOMEM;

    // if the signal closes, close the sonagram window too
    QObject::connect(&manager(), SIGNAL(sigClosed()),
                     m_sonagram_window, SLOT(close()));

    // get the current selection
    QList<unsigned int> selected_channels;
    sample_index_t offset = 0;
    sample_index_t length = 0;
    length = selection(&selected_channels, &offset, 0, true);

    // abort if nothing is selected
    if (!length || selected_channels.isEmpty())
	return -EINVAL;

    // calculate the number of slices (width of image)
    m_slices = Kwave::toUint(ceil(static_cast<double>(length) /
	                          static_cast<double>(m_fft_points)));
    if (m_slices > MAX_SLICES) m_slices = MAX_SLICES;

    /* limit selection to INT_MAX samples (limitation of the cache index) */
    if ((length / m_fft_points) >= INT_MAX) {
	Kwave::MessageBox::error(parentWidget(),
	                         i18n("File or selection too large"));
	return -EFBIG;
    }

    // create a selection tracker
    m_selection = new(std::nothrow) Kwave::SelectionTracker(
	&sig_mgr, offset, length, &selected_channels);
    Q_ASSERT(m_selection);
    if (!m_selection) return -ENOMEM;

    connect(m_selection, SIGNAL(sigTrackInserted(QUuid)),
            this,        SLOT(slotTrackInserted(QUuid)));
    connect(m_selection, SIGNAL(sigTrackDeleted(QUuid)),
            this,        SLOT(slotTrackDeleted(QUuid)));
    connect(
	m_selection,
	SIGNAL(sigInvalidated(const QUuid*,sample_index_t,sample_index_t)),
	this,
	SLOT(slotInvalidated(const QUuid*,sample_index_t,sample_index_t))
    );

    // create a new empty image
    createNewImage(m_slices, m_fft_points / 2);

    // set the overview
    m_overview_cache = new(std::nothrow)
	Kwave::OverViewCache(sig_mgr, offset, length, &selected_channels);
    Q_ASSERT(m_overview_cache);
    if (!m_overview_cache) return -ENOMEM;

    refreshOverview(); // <- this needs the m_overview_cache

    if (m_track_changes) {
	// stay informed about changes in the signal
	connect(m_overview_cache, SIGNAL(changed()),
	        this, SLOT(refreshOverview()));
    } else {
	// overview cache is no longer needed
	delete m_overview_cache;
	m_overview_cache = 0;
    }

    // connect all needed signals
    connect(m_sonagram_window, SIGNAL(destroyed()),
            this, SLOT(windowDestroyed()));

    // activate the window with an initial image
    // and all necessary information
    m_sonagram_window->setColorMode((m_color) ? 1 : 0);
    m_sonagram_window->setImage(m_image);
    m_sonagram_window->setPoints(m_fft_points);
    m_sonagram_window->setRate(signalRate());
    m_sonagram_window->show();

    if (m_track_changes) {
	QObject::connect(static_cast<QObject*>(&(manager())),
	    SIGNAL(sigSignalNameChanged(QString)),
	    m_sonagram_window, SLOT(setName(QString)));
    }

    // increment the usage counter and release the plugin when the
    // sonagram window closed
    use();

    return 0;
}

//***************************************************************************
void Kwave::SonagramPlugin::makeAllValid()
{
    unsigned int             fft_points;
    unsigned int             slices;
    Kwave::window_function_t window_type;
    sample_index_t           first_sample;
    sample_index_t           last_sample;
    QBitArray                valid;
    QList<unsigned int>      track_list;

    {
	QMutexLocker _lock(&m_lock_job_list);

	if (!m_selection) return;
	if (!m_selection->length() || (m_fft_points < 4)) return;

	fft_points   = m_fft_points;
	slices       = m_slices;
	window_type  = m_window_type;
	first_sample = m_selection->first();
	last_sample  = m_selection->last();
	valid        = m_valid;
	m_valid.fill(true);

	const QList<QUuid> selected_tracks(m_selection->allTracks());
	foreach (unsigned int track, signalManager().allTracks())
	    if (selected_tracks.contains(signalManager().uuidOfTrack(track)))
		track_list.append(track);
    }
    const unsigned int tracks = track_list.count();

    Kwave::WindowFunction func(window_type);
    const QVector<double> windowfunction = func.points(fft_points);
    Q_ASSERT(windowfunction.count() == Kwave::toInt(fft_points));
    if (windowfunction.count() != Kwave::toInt(fft_points)) return;

    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), track_list, first_sample, last_sample);

//     qDebug("SonagramPlugin[%p]::makeAllValid() [%llu .. %llu]",
// 	static_cast<void *>(this), first_sample, last_sample);

    QFutureSynchronizer<void> synchronizer;
    for (unsigned int slice_nr = 0; slice_nr < slices; slice_nr++) {
// 	qDebug("SonagramPlugin::run(): calculating slice %d of %d",
// 	       slice_nr, m_slices);

	if (valid[slice_nr]) continue;

	// determine start of the stripe
	sample_index_t pos = first_sample + (slice_nr * fft_points);

	// get a new slice from the pool and initialize it
	Kwave::SonagramPlugin::Slice *slice = m_slice_pool.allocate();
	Q_ASSERT(slice);

	slice->m_index = slice_nr;
	memset(slice->m_input,  0x00, sizeof(slice->m_input));
	memset(slice->m_output, 0x00, sizeof(slice->m_output));

	if ((pos <= last_sample) && (tracks)) {
	    // initialize result with zeroes
	    memset(slice->m_result, 0x00, sizeof(slice->m_result));

	    // seek to the start of the slice
	    source.seek(pos);

	    // we have a new slice, now fill it's input buffer
	    double *in = slice->m_input;
	    for (unsigned int j = 0; j < fft_points; j++) {
		double value = 0.0;
		if (!(source.eof())) {
		    for (unsigned int t = 0; t < tracks; t++) {
			sample_t s = 0;
			Kwave::SampleReader *reader = source[t];
			Q_ASSERT(reader);
			if (reader) *reader >> s;
			value += sample2double(s);
		    }
		    value /= tracks;
		}
		in[j] = value * windowfunction[j];
	    }

	    // a background job is running soon
	    // (for counterpart, see insertSlice(...) below [main thread])
	    m_pending_jobs.lockForRead();

	    // run the FFT in a background thread
	    synchronizer.addFuture(QtConcurrent::run(
		this, &Kwave::SonagramPlugin::calculateSlice, slice)
	    );
	} else {
	    // range has been deleted -> fill with "empty"
	    memset(slice->m_result, 0xFF, sizeof(slice->m_result));
	    m_pending_jobs.lockForRead();
	    emit sliceAvailable(slice);
	}

	if (shouldStop()) break;
    }

//     qDebug("SonagramPlugin::makeAllValid(): waiting for background jobs...");

    // wait for all worker threads
    synchronizer.waitForFinished();

    // wait for queued signals
    m_pending_jobs.lockForWrite();
    m_pending_jobs.unlock();

//     qDebug("SonagramPlugin::makeAllValid(): done.");
}

//***************************************************************************
void Kwave::SonagramPlugin::run(QStringList params)
{
    qDebug("SonagramPlugin::run()");
    Q_UNUSED(params);
    {
	// invalidate all slices
	QMutexLocker _lock(&m_lock_job_list);
	m_valid.fill(false);
    }
    makeAllValid();
}

//***************************************************************************
void Kwave::SonagramPlugin::calculateSlice(Kwave::SonagramPlugin::Slice *slice)
{
    fftw_plan p;

    // prepare for a 1-dimensional real-to-complex DFT
    {
	Kwave::GlobalLock _lock; // libfftw is not threadsafe!
	p = fftw_plan_dft_r2c_1d(
	    m_fft_points,
	    &(slice->m_input[0]),
	    &(slice->m_output[0]),
	    FFTW_ESTIMATE
	);
    }
    Q_ASSERT(p);
    if (!p) return;

    // calculate the fft (according to the specs, this is the one and only
    // libfft function that is threadsafe!)
    fftw_execute(p);

    // norm all values to [0...254] and use them as pixel value
    const double scale = static_cast<double>(m_fft_points) / 254.0;
    for (unsigned int j = 0; j < m_fft_points / 2; j++) {
	// get singal energy and scale to [0 .. 254]
	double rea = slice->m_output[j][0];
	double ima = slice->m_output[j][1];
	double a = ((rea * rea) + (ima * ima)) / scale;

	slice->m_result[j] = static_cast<unsigned char>(qMin(a, double(254.0)));
    }

    // free the allocated FFT resources
    {
	Kwave::GlobalLock _lock; // libfftw is not threadsafe!
	fftw_destroy_plan(p);
    }

    // emit the slice data to be synchronously inserted into
    // the current image in the context of the main thread
    // (Qt does the queuing for us)
    emit sliceAvailable(slice);
}

//***************************************************************************
void Kwave::SonagramPlugin::insertSlice(Kwave::SonagramPlugin::Slice *slice)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(slice);
    if (!slice) return;

    QByteArray result;
    result.setRawData(reinterpret_cast<char *>(&(slice->m_result[0])),
                      m_fft_points / 2);
    unsigned int nr = slice->m_index;

    // forward the slice to the window to display it
    if (m_sonagram_window) m_sonagram_window->insertSlice(nr, result);

    // return the slice into the pool
    m_slice_pool.release(slice);

    // job is done
    m_pending_jobs.unlock();
}

//***************************************************************************
void Kwave::SonagramPlugin::createNewImage(const unsigned int width,
                                           const unsigned int height)
{
    // delete the previous image
    m_image = QImage();
    if (m_sonagram_window) m_sonagram_window->setImage(m_image);

    // do not create a new image if one dimension is zero!
    Q_ASSERT(width);
    Q_ASSERT(height);
    if (!width || !height) return;

    // also do not create if the image size is out of range
    Q_ASSERT(width <= 32767);
    Q_ASSERT(height <= 32767);
    if ((width >= 32767) || (height >= 32767)) return;

    // create the new image object
    m_image = QImage(width, height, QImage::Format_Indexed8);
    Q_ASSERT(!m_image.isNull());
    if (m_image.isNull()) return;

    // initialize the image's palette with transparecy
    m_image.setColorCount(256);
    for (int i = 0; i < 256; i++) {
	m_image.setColor(i, 0x00000000);
    }

    // fill the image with "empty" (transparent)
    m_image.fill(0xFF);
}

//***************************************************************************
void Kwave::SonagramPlugin::refreshOverview()
{
    if (!m_overview_cache || !m_sonagram_window) return;

    QColor fg = m_sonagram_window->palette().light().color();
    QColor bg = m_sonagram_window->palette().mid().color();
    QImage overview = m_overview_cache->getOverView(
       m_sonagram_window->width(), SONAGRAM_OVERVIEW_HEIGHT, fg, bg);

    m_sonagram_window->setOverView(overview);
}

//***************************************************************************
void Kwave::SonagramPlugin::requestValidation()
{
    // only re-start the repaint timer, this hides some GUI update artifacts
    if (!m_repaint_timer.isActive()) {
	m_repaint_timer.stop();
	m_repaint_timer.setSingleShot(true);
	m_repaint_timer.start(REPAINT_INTERVAL);
    }
}

//***************************************************************************
void Kwave::SonagramPlugin::validate()
{
    // wait for previously running jobs to finish
    if (m_future.isRunning()) {
	requestValidation();
	return; // job is still running, come back later...
    }

    // queue a background thread for updates
    m_future = QtConcurrent::run(this, &Kwave::SonagramPlugin::makeAllValid);
}

//***************************************************************************
void Kwave::SonagramPlugin::slotTrackInserted(const QUuid &track_id)
{
    QMutexLocker _lock(&m_lock_job_list);

    Q_UNUSED(track_id);

    // check for "track changes" mode
    if (!m_track_changes) return;

    // invalidate complete signal
    m_valid.fill(false, m_slices);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::slotTrackDeleted(const QUuid &track_id)
{
    QMutexLocker _lock(&m_lock_job_list);

    Q_UNUSED(track_id);

    // check for "track changes" mode
    if (!m_track_changes) return;

    // invalidate complete signal
    m_valid.fill(false, m_slices);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::slotInvalidated(const QUuid *track_id,
                                            sample_index_t first,
                                            sample_index_t last)
{
    QMutexLocker lock(&m_lock_job_list);

    Q_UNUSED(track_id);
//     qDebug("SonagramPlugin[%p]::slotInvalidated(%s, %llu, %llu)",
// 	    static_cast<void *>(this),
// 	   (track_id) ? DBG(track_id->toString()) : "*", first, last);

    // check for "track changes" mode
    if (!m_track_changes) return;

    // adjust offsets, absolute -> relative
    sample_index_t offset = (m_selection) ? m_selection->offset() : 0;
    Q_ASSERT(first >= offset);
    Q_ASSERT(last  >= offset);
    Q_ASSERT(last  >= first);
    first -= offset;
    last  -= offset;

    unsigned int first_idx = Kwave::toUint(first / m_fft_points);
    unsigned int last_idx;
    if (last >= (SAMPLE_INDEX_MAX - (m_fft_points - 1)))
	last_idx = m_slices - 1;
    else
	last_idx = Kwave::toUint(qMin(Kwave::round_up(last,
	    static_cast<sample_index_t>(m_fft_points)) / m_fft_points,
	    static_cast<sample_index_t>(m_slices - 1))
	);

    m_valid.fill(false, first_idx, last_idx + 1);
    requestValidation();
}

//***************************************************************************
void Kwave::SonagramPlugin::windowDestroyed()
{
    cancel();

    m_sonagram_window = 0; // closes itself !

    if (m_selection) delete m_selection;
    m_selection = 0;

    if (m_overview_cache) delete m_overview_cache;
    m_overview_cache = 0;

    release();
}

//***************************************************************************
#include "SonagramPlugin.moc"
//***************************************************************************
//***************************************************************************
