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

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <complex>
#include <fftw3.h>

#include <QtCore/QFutureSynchronizer>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QColor>
#include <QtGui/QImage>

#include "libkwave/GlobalLock.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/WindowFunction.h"

#include "libgui/OverViewCache.h"

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(Kwave::SonagramPlugin, "sonagram", "2.3",
             I18N_NOOP("Sonagram"), "Thomas Eschenbacher");

//***************************************************************************
//***************************************************************************
Kwave::SonagramPlugin::SonagramPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager), m_sonagram_window(0), m_selected_channels(),
     m_first_sample(0), m_last_sample(0), m_slices(0), m_fft_points(0),
     m_window_type(Kwave::WINDOW_FUNC_NONE), m_color(true),
     m_track_changes(true), m_follow_selection(false), m_image(),
     m_overview_cache(0), m_slice_pool(), m_pending_jobs()
{
    i18n("Sonagram");

    // connect the output
    connect(this, SIGNAL(sliceAvailable(Kwave::SonagramPlugin::Slice *)),
            this, SLOT(insertSlice(Kwave::SonagramPlugin::Slice *)),
            Qt::QueuedConnection);
}

//***************************************************************************
Kwave::SonagramPlugin::~SonagramPlugin()
{
    if (m_sonagram_window) delete m_sonagram_window;
    m_sonagram_window = 0;
}

//***************************************************************************
QStringList *Kwave::SonagramPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    Kwave::SonagramDialog *dlg = new Kwave::SonagramDialog(*this);
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

//     param = params[3];
    m_track_changes = false; // (param.toUInt(&ok) != 0);
//     if (!ok) return -EINVAL;

//     param = params[4];
    m_follow_selection = false; // (param.toUInt(&ok) != 0);
//     if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
int Kwave::SonagramPlugin::start(QStringList &params)
{
    // interprete parameter list and abort if it contains invalid data
    int result = interpreteParameters(params);
    if (result) return result;

    // create an empty sonagram window
    m_sonagram_window = new Kwave::SonagramWindow(signalName());
    Q_ASSERT(m_sonagram_window);
    if (!m_sonagram_window) return -ENOMEM;

    // if the signal closes, close the sonagram window too
    QObject::connect(&manager(), SIGNAL(sigClosed()),
                     m_sonagram_window, SLOT(close()));

    unsigned int input_length = selection(&m_selected_channels,
	&m_first_sample, &m_last_sample, true);
    if (!input_length || m_selected_channels.isEmpty())
	return -EINVAL;

    // calculate the number of slices (width of image)
    m_slices = static_cast<unsigned int>
	(ceil(static_cast<double>(input_length) /
	      static_cast<double>(m_fft_points)));
    if (m_slices > 32767) m_slices = 32767;

    // create a new empty image
    createNewImage(m_slices, m_fft_points/2);

    // set the overview
    Kwave::SignalManager &sig_mgr = manager().signalManager();
    m_overview_cache = new Kwave::OverViewCache(sig_mgr,
        m_first_sample, input_length, &m_selected_channels);
    Q_ASSERT(m_overview_cache);
    if (!m_overview_cache) return -ENOMEM;

    refreshOverview();
    if (m_track_changes) {
	// stay informed about changes in the signal
	connect(m_overview_cache, SIGNAL(changed()),
	        this, SLOT(refreshOverview()));
    } else {
	// overview cache is no longer needed
	delete m_overview_cache;
	m_overview_cache = 0;
    }

    // activate the window with an initial image
    // and all necessary information
    m_sonagram_window->setColorMode((m_color) ? 1 : 0);
    m_sonagram_window->setImage(m_image);
    m_sonagram_window->setPoints(m_fft_points);
    m_sonagram_window->setRate(signalRate());
    m_sonagram_window->show();

    // connect all needed signals
    connect(m_sonagram_window, SIGNAL(destroyed()),
	    this, SLOT(windowDestroyed()));

    if (m_track_changes) {
	QObject::connect(static_cast<QObject*>(&(manager())),
	    SIGNAL(sigSignalNameChanged(const QString &)),
	    m_sonagram_window, SLOT(setName(const QString &)));
    }

    // increment the usage counter and release the plugin when the
    // sonagram window closed
    use();

    return 0;
}

//***************************************************************************
void Kwave::SonagramPlugin::run(QStringList params)
{
    qDebug("SonagramPlugin::run()");

    Q_UNUSED(params);

    if (m_fft_points < 4)
	return;

    Kwave::WindowFunction func(m_window_type);
    const QVector<double> windowfunction = func.points(m_fft_points);
    Q_ASSERT(windowfunction.count() == static_cast<int>(m_fft_points));
    if (windowfunction.count() != static_cast<int>(m_fft_points)) return;

    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), selectedTracks(),
	m_first_sample, m_last_sample);
//     qDebug("SonagramPlugin::run(), [%llu..%llu]",m_first_sample,m_last_sample);

    const unsigned int tracks = source.tracks();
    Q_ASSERT(tracks);
    if (!tracks) return;

    QFutureSynchronizer<void> synchronizer;
    for (unsigned int slice_nr = 0; slice_nr < m_slices; slice_nr++) {
// 	qDebug("SonagramPlugin::run(): calculating slice %d of %d",
// 	       slice_nr, m_slices);

	// get a new slice from the pool and initialize it
	Kwave::SonagramPlugin::Slice *slice = m_slice_pool.allocate();
	Q_ASSERT(slice);

	slice->m_index = slice_nr;
	memset(slice->m_input,  0x00, sizeof(slice->m_input));
	memset(slice->m_output, 0x00, sizeof(slice->m_output));
	memset(slice->m_result, 0x00, sizeof(slice->m_result));

	// we have a new slice, now fill it's input buffer
	double *in = slice->m_input;
	for (unsigned int j = 0; j < m_fft_points; j++) {
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

	// a background job is runing soon
	// (for counterpart, see insertSlice(...) below [main thread])
	m_pending_jobs.lockForRead();

	// run the FFT in a background thread
	synchronizer.addFuture(QtConcurrent::run(
	    this, &Kwave::SonagramPlugin::calculateSlice, slice)
	);

	if (shouldStop()) break;
    }

    qDebug("SonagramPlugin::run(): waiting for background jobs...");

    // wait for all worker threads
    synchronizer.waitForFinished();

    // wait for queued signals
    m_pending_jobs.lockForWrite();
    m_pending_jobs.unlock();

    qDebug("SonagramPlugin::run(): done.");
}

//***************************************************************************
void Kwave::SonagramPlugin::calculateSlice(Kwave::SonagramPlugin::Slice *slice)
{
    fftw_plan p;

//     qDebug("SonagramPlugin::calculateSlice(%u)...", slice->m_index);

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
    for (unsigned int j = 0; j < m_fft_points / 2; j++) {
	double rea = slice->m_output[j][0];
	double ima = slice->m_output[j][1];
	double a = sqrt((rea * rea) + (ima * ima)) /
	    static_cast<double>(m_fft_points);

	// get amplitude and scale to 1
	a = 1 - ((1 - a) * (1 - a));

	slice->m_result[j] = 0xFE - static_cast<int>(a * 0xFE);
    }

    // free the the allocated FFT resources
    {
	Kwave::GlobalLock _lock; // libfftw is not threadsafe!
	fftw_destroy_plan(p);
    }

    // emit the slice data to be synchronously inserted into
    // the current image in the context of the main thread
    // (Qt does the queuing for us)
    emit sliceAvailable(slice);

//     qDebug("SonagramPlugin::calculateSlice(%u) done.", slice->m_index);
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
    result.setRawData(&(slice->m_result[0]), m_fft_points / 2);
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
    m_image.setNumColors(256);
    for (int i = 0; i < 256; i++) {
	m_image.setColor(i, 0x00000000);
    }

    // fill the image with "empty"
    m_image.fill(0xFF);
}

//***************************************************************************
void Kwave::SonagramPlugin::refreshOverview()
{
    if (!m_overview_cache || !m_sonagram_window) return;

    QColor fg = m_sonagram_window->palette().light().color();
    QColor bg = m_sonagram_window->palette().mid().color();
    QImage overview = m_overview_cache->getOverView(
       m_sonagram_window->width(), 40, fg, bg);

    m_sonagram_window->setOverView(overview);
}

//***************************************************************************
void Kwave::SonagramPlugin::windowDestroyed()
{
    m_sonagram_window = 0; // closes itself !
    cancel();
    release();
}

//***************************************************************************
#include "SonagramPlugin.moc"
//***************************************************************************
//***************************************************************************
