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

#include <QString>
#include <QImage>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_fft.h>
#include <gsl/gsl_fft_complex.h>

#include "libkwave/KwavePlugin.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/WindowFunction.h"
#include "libgui/OverViewCache.h"
#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
#include "kwave/TopWidget.h"

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(SonagramPlugin,"sonagram","Thomas Eschenbacher");

#define MAX_QUEUE_USAGE 256

/**
 * simple private container class for stripe number and data (TSS-safe)
 */
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

//***************************************************************************
SonagramPlugin::SonagramPlugin(const PluginContext &c)
    :KwavePlugin(c), m_sonagram_window(0), m_selected_channels(),
     m_first_sample(0), m_last_sample(0), m_stripes(0), m_fft_points(0),
     m_window_type(WINDOW_FUNC_NONE), m_color(true), m_track_changes(true),
     m_follow_selection(false), m_image(0), m_overview_cache(0),
     m_cmd_shutdown(false)
{
    connect(this, SIGNAL(stripeAvailable(StripeInfoPrivate *)),
            this, SLOT(insertStripe(StripeInfoPrivate *)),
            Qt::BlockingQueuedConnection);
    i18n("sonagram");
}

//***************************************************************************
SonagramPlugin::~SonagramPlugin()
{
    if (m_sonagram_window) delete m_sonagram_window;
    m_sonagram_window = 0;

    if (m_image) delete m_image;
    m_image = 0;
}

//***************************************************************************
QStringList *SonagramPlugin::setup(QStringList &previous_params)
{
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    SonagramDialog *dlg = new SonagramDialog(*this);
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
int SonagramPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return -EINVAL;

    param = params[0];
    m_fft_points = param.toUInt(&ok);
    if (!ok) return -EINVAL;
    if (m_fft_points > 32767) m_fft_points=32767;

    param = params[1];
    m_window_type = WindowFunction::findFromName(param);
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
int SonagramPlugin::start(QStringList &params)
{
    // interprete parameter list and abort if it contains invalid data
    int result = interpreteParameters(params);
    if (result) return result;

    // create an empty sonagram window
    m_sonagram_window = new SonagramWindow(signalName());
    Q_ASSERT(m_sonagram_window);
    if (!m_sonagram_window) return -ENOMEM;

    // if the signal closes, close the sonagram window too
    QObject::connect(&manager(), SIGNAL(sigClosed()),
                     m_sonagram_window, SLOT(close()));

    unsigned int input_length = selection(&m_first_sample, &m_last_sample);
    if (m_first_sample == m_last_sample) {
	input_length = signalLength()-1;
	m_first_sample = 0;
	m_last_sample = input_length-1;
    }

    // calculate the number of stripes (width of image)
    m_stripes = (unsigned int)
	(ceil((double)input_length/(double)m_fft_points));
    if (m_stripes > 32767) m_stripes = 32767;

    // create a new empty image
    createNewImage(m_stripes, m_fft_points/2);

    // set the overview
    m_selected_channels = selectedTracks();
    SignalManager &sig_mgr = manager().topWidget().signalManager();
    QList<unsigned int> tracks = sig_mgr.selectedTracks();
    m_overview_cache = new OverViewCache(sig_mgr,
        m_first_sample, m_last_sample-m_first_sample+1,
        tracks.isEmpty() ? 0 : &tracks);
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
    // and all necessary informations
    m_sonagram_window->setColorMode((m_color) ? 1 : 0);
    m_sonagram_window->setImage(m_image);
    m_sonagram_window->setPoints(m_fft_points);
    m_sonagram_window->setRate(signalRate());
    m_sonagram_window->show();

    // connect all needed signals
    connect(m_sonagram_window, SIGNAL(destroyed()),
	    this, SLOT(windowDestroyed()));

    if (m_track_changes) {
	QObject::connect((QObject*)&(manager()),
	    SIGNAL(sigSignalNameChanged(const QString &)),
	    m_sonagram_window, SLOT(setName(const QString &)));
    }

    // increment the usage counter and release the plugin when the
    // sonagram window closed
    use();

    m_cmd_shutdown = false;
    return 0;
}

//***************************************************************************
int SonagramPlugin::stop()
{
   m_cmd_shutdown = true;
   return KwavePlugin::stop();
}

//***************************************************************************
void SonagramPlugin::run(QStringList /* params */)
{
//    qDebug("SonagramPlugin::run()");

    MultiTrackReader source(signalManager(), selectedTracks(),
	m_first_sample, m_last_sample);
//    qDebug("SonagramPlugin::run(), first=%u, last=%u",m_first_sample,m_last_sample);

    QByteArray stripe_data(m_fft_points/2, 0x00);
    while (!m_cmd_shutdown) {
	unsigned int stripe_nr;
	for (stripe_nr = 0; stripe_nr < m_stripes; stripe_nr++) {
//	    qDebug("SonagramPlugin::run(): calculating stripe %d of %d",
//	        stripe_nr,m_stripes);

	    // calculate one stripe
	    calculateStripe(source, m_fft_points, stripe_data);

	    StripeInfoPrivate *stripe_info = new
		StripeInfoPrivate(stripe_nr, stripe_data);
	    if (!stripe_info) break; // out of memory

	    // emit the stripe data to be synchronously inserted into
	    // the current image
	    emit stripeAvailable(stripe_info);
	    delete stripe_info;

	    if (m_cmd_shutdown) break;
	}

	m_cmd_shutdown = true;
    }

//    qDebug("SonagramPlugin::run(): done.");
}

//***************************************************************************
void SonagramPlugin::insertStripe(StripeInfoPrivate *stripe_info)
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
void SonagramPlugin::calculateStripe(MultiTrackReader &source,
	const int points, QByteArray &output)
{
    gsl_fft_complex_wavetable *wavetable = 0;
    gsl_fft_complex_workspace *workspace = 0;

    // first initialize the output to zeroes in case of errors
    output.fill(0);

    WindowFunction func(m_window_type);
    QVector<double> windowfunction = func.points(points);
    Q_ASSERT(windowfunction.count() == points);
    if (windowfunction.count() != points) return;

    QVector<double> input(2*points);
    Q_ASSERT(input.size() == 2*points);
    if (input.size() < 2*points) return;

    // initialize the table for fft
    wavetable = gsl_fft_complex_wavetable_alloc(points);
    Q_ASSERT(wavetable);
    if (!wavetable) return;

    workspace = gsl_fft_complex_workspace_alloc(points);
    Q_ASSERT(workspace);
    if (!workspace) {
	gsl_fft_complex_wavetable_free(wavetable);
	return;
    }

    // copy signal data into complex array
    for (int j = 0; j < points; j++) {
	double value = 0.0;
	if (!(source.eof())) {
	    unsigned int count = source.tracks();
	    unsigned int t;
	    for (t=0; t < count; t++) {
		sample_t s = 0;
		SampleReader *reader = source[t];
		Q_ASSERT(reader);
		if (reader) *reader >> s;
		value += (double)s;
	    }
	    value /= (double)(1 << (SAMPLE_BITS-1)) * (double)count;
	}
	input[j+j + 0] = windowfunction[j] * value;
	input[j+j + 1] = 0;
    }

    // calculate the fft
    gsl_fft_complex_forward(input.data(), 1, points,
                            wavetable, workspace);

    double ima;
    double rea;
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
    double max = m_fft_points/2;

    // norm all values to [0...255] and use them as pixel value
    if (max != 0) {
	for (int j = 0; j < points/2; j++) {
	    rea = input[j+j + 0];
	    ima = input[j+j + 1];
	    rea = sqrt(rea * rea + ima * ima) / max;

	    // get amplitude and scale to 1
	    rea = 1 - ((1 - rea) * (1 - rea));

	    output[j] = 0xFE - (int)(rea * 0xFE );
	}
    }

    // free the intermediate array used for fft
    gsl_fft_complex_wavetable_free(wavetable);
    gsl_fft_complex_workspace_free(workspace);
}

//***************************************************************************
void SonagramPlugin::createNewImage(const unsigned int width,
	const unsigned int height)
{
//    qDebug("SonagramPlugin::createNewImage()");

    // delete the previous image
    if (m_sonagram_window) m_sonagram_window->setImage(0);
    if (m_image) delete m_image;
    m_image = 0;

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
    m_image = new QImage(width, height, QImage::Format_Indexed8);
    Q_ASSERT(m_image);
    if (!m_image) return;

    // initialize the image's palette with transparecy
    m_image->setNumColors(256);
    for (int i=0; i < 256; i++) {
	m_image->setColor(i, 0x00000000);
    }

    // fill the image with "empty"
    m_image->fill(0xFF);

//     qDebug("SonagramPlugin::createNewImage(): done.");
}

//***************************************************************************
void SonagramPlugin::refreshOverview()
{
    if (!m_overview_cache || !m_sonagram_window) return;

    QBitmap overview = m_overview_cache->getOverView(
        2*m_sonagram_window->width(), 40);
    m_sonagram_window->setOverView(&overview);
}

//***************************************************************************
void SonagramPlugin::windowDestroyed()
{
    m_sonagram_window = 0; // closes itself !
    m_cmd_shutdown = true;
    release();
}

//***************************************************************************
#include "SonagramPlugin.moc"
//***************************************************************************
//***************************************************************************
