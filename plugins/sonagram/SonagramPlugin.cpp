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

#include <QtGui/QColor>
#include <QtCore/QString>
#include <QtGui/QImage>

#include "libkwave/Plugin.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/WindowFunction.h"

#include "libgui/OverViewCache.h"

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(Kwave::SonagramPlugin, "sonagram", "2.1",
             I18N_NOOP("Sonagram"), "Thomas Eschenbacher");

#define MAX_QUEUE_USAGE 256

/**
 * simple private container class for stripe number and data (TSS-safe)
 */
namespace Kwave {
    class StripeInfoPrivate
    {
    public:
	StripeInfoPrivate(unsigned int nr, const QByteArray &data)
	    :m_nr(nr), m_data(data)
	    {};
	StripeInfoPrivate(const StripeInfoPrivate &copy)
	    :m_nr(copy.nr()), m_data(copy.data())
	    {};
	virtual ~StripeInfoPrivate() {} ;
	unsigned int nr() const { return m_nr; };
	const QByteArray &data() const { return m_data; };
    private:
	unsigned int m_nr;
	QByteArray m_data;
    };
}

//***************************************************************************
Kwave::SonagramPlugin::SonagramPlugin(const Kwave::PluginContext &c)
    :Kwave::Plugin(c), m_sonagram_window(0), m_selected_channels(),
     m_first_sample(0), m_last_sample(0), m_stripes(0), m_fft_points(0),
     m_window_type(Kwave::WINDOW_FUNC_NONE), m_color(true),
     m_track_changes(true), m_follow_selection(false), m_image(),
     m_overview_cache(0)
{
    connect(this, SIGNAL(stripeAvailable(Kwave::StripeInfoPrivate *)),
            this, SLOT(insertStripe(Kwave::StripeInfoPrivate *)),
            Qt::BlockingQueuedConnection);
    i18n("Sonagram");
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
    if (m_fft_points > 32767) m_fft_points = 32767;

    param = params[1];
    m_window_type = Kwave::WindowFunction::findFromName(param);
    if (!ok) return -EINVAL;

    param = params[2];
    m_color = (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

    param = params[3];
    m_track_changes = false; // (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

    param = params[4];
    m_follow_selection = false; // (param.toUInt(&ok) != 0);
    if (!ok) return -EINVAL;

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

    // calculate the number of stripes (width of image)
    m_stripes = static_cast<unsigned int>
	(ceil(static_cast<double>(input_length) /
	      static_cast<double>(m_fft_points)));
    if (m_stripes > 32767) m_stripes = 32767;

    // create a new empty image
    createNewImage(m_stripes, m_fft_points/2);

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
void Kwave::SonagramPlugin::run(QStringList /* params */)
{
//    qDebug("SonagramPlugin::run()");

    if (m_fft_points < 4)
	return;

    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), selectedTracks(),
	m_first_sample, m_last_sample);
//    qDebug("SonagramPlugin::run(), first=%u, last=%u",m_first_sample,m_last_sample);

    QByteArray stripe_data(m_fft_points / 2, 0x00);
    unsigned int stripe_nr;
    for (stripe_nr = 0; stripe_nr < m_stripes; stripe_nr++) {
// 	qDebug("SonagramPlugin::run(): calculating stripe %d of %d",
// 	    stripe_nr,m_stripes);

	// calculate one stripe
	calculateStripe(source, m_fft_points, stripe_data);

	Kwave::StripeInfoPrivate *stripe_info = new
	    Kwave::StripeInfoPrivate(stripe_nr, stripe_data);
	if (!stripe_info) break; // out of memory

	// emit the stripe data to be synchronously inserted into
	// the current image
	emit stripeAvailable(stripe_info);
	delete stripe_info;

	if (shouldStop()) break;
    }

//    qDebug("SonagramPlugin::run(): done.");
}

//***************************************************************************
void Kwave::SonagramPlugin::insertStripe(Kwave::StripeInfoPrivate *stripe_info)
{
    Q_ASSERT(stripe_info);
    if (!stripe_info) return;

    unsigned int stripe_nr  = stripe_info->nr();
    const QByteArray stripe = stripe_info->data();

    // forward the stripe to the window to display it
    if (m_sonagram_window) m_sonagram_window->insertStripe(
	stripe_nr, stripe);
}

//***************************************************************************
void Kwave::SonagramPlugin::calculateStripe(Kwave::MultiTrackReader &source,
	const int points, QByteArray &output)
{
    double *in  = 0;
    fftw_complex *out = 0;
    fftw_plan p;

    // first initialize the output to zeroes in case of errors
    output.fill(0);

    Kwave::WindowFunction func(m_window_type);
    QVector<double> windowfunction = func.points(points);
    Q_ASSERT(windowfunction.count() == points);
    if (windowfunction.count() != points) return;

    // initialize the tables for fft
    in = static_cast<double *>(fftw_malloc(points * sizeof(double)));
    Q_ASSERT(in);
    if (!in) return;

    out = static_cast<fftw_complex *>(fftw_malloc(
	((points / 2) + 1) * sizeof(fftw_complex)));
    Q_ASSERT(out);
    if (!out){
	fftw_free(in);
	return;
    }

    // prepare for a 1-dimensional real-to-complex DFT
    p = fftw_plan_dft_r2c_1d(points, in, out, FFTW_ESTIMATE);

    // copy normed signal data into the real array
    for (int j = 0; j < points; j++) {
	double value = 0.0;
	if (!(source.eof())) {
	    unsigned int count = source.tracks();
	    Q_ASSERT(count);
	    if (!count) return;

	    unsigned int t;
	    for (t=0; t < count; t++) {
		sample_t s = 0;
		Kwave::SampleReader *reader = source[t];
		Q_ASSERT(reader);
		if (reader) *reader >> s;
		value += static_cast<double>(s);
	    }
	    value /= static_cast<double>(1 << (SAMPLE_BITS-1)) *
		static_cast<double>(count);
	}
	in[j] = value;
    }

    // calculate the fft
    fftw_execute(p);

//    double max = 0.0;
//    for (int k = 0; k < points/2; k++) {
//	rea = data[k].real;
//	ima = data[k].imag;
//
//	//get amplitude
//	rea = sqrt(rea * rea + ima * ima);
//
//	//and set maximum for display..
//	if (rea > max) max = rea;
//    }
//
//     double max = m_fft_points/2;

    // norm all values to [0...254] and use them as pixel value
    for (int j = 0; j < points / 2; j++) {
	double rea = out[j][0];
	double ima = out[j][1];
	double a = sqrt((rea * rea) + (ima * ima)) /
	    static_cast<double>(points);

	// get amplitude and scale to 1
	a = 1 - ((1 - a) * (1 - a));

	output[j] = 0xFE - static_cast<int>(a * 0xFE );
    }

    // free the the allocated FFT resources
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}

//***************************************************************************
void Kwave::SonagramPlugin::createNewImage(const unsigned int width,
	const unsigned int height)
{
//    qDebug("SonagramPlugin::createNewImage()");

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

//     qDebug("SonagramPlugin::createNewImage(): settings ok, creating image");

    // create the new image object
    m_image = QImage(width, height, QImage::Format_Indexed8);
    Q_ASSERT(!m_image.isNull());
    if (m_image.isNull()) return;

    // initialize the image's palette with transparecy
    m_image.setNumColors(256);
    for (int i=0; i < 256; i++) {
	m_image.setColor(i, 0x00000000);
    }

    // fill the image with "empty"
    m_image.fill(0xFF);

//     qDebug("SonagramPlugin::createNewImage(): done.");
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
