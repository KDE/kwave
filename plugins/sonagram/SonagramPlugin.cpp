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

#include <qstring.h>
#include <qimage.h>

#include <kapp.h>

#include <libkwave/gsl_fft.h>
#include <libkwave/WindowFunction.h>

#include <libgui/KwavePlugin.h>

#include <mt/SignalProxy.h>

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(SonagramPlugin,"sonagram","Thomas Eschenbacher");

#define MAX_QUEUE_USAGE 16

/**
 * simple private container class for stripe number and data (TSS-safe)
 */
class StripeInfoPrivate: public TSS_Object
{
public:
    StripeInfoPrivate(unsigned int nr, QByteArray data)
	:TSS_Object(), m_nr(nr), m_data(data)
	{};
    StripeInfoPrivate(StripeInfoPrivate &copy)
	:TSS_Object(), m_nr(copy.nr()), m_data(copy.data())
	{};
    virtual ~StripeInfoPrivate() {} ;
    unsigned int nr() { return m_nr; };
    QByteArray &data() { return m_data; };
private:
    unsigned int m_nr;
    QByteArray m_data;
};

//***************************************************************************
SonagramPlugin::SonagramPlugin(PluginContext &c)
    :KwavePlugin(c)
{
    m_cmd_shutdown = false; // ###
    m_color = true;
    m_fft_points = 0;
    m_first_sample = 0;
    m_follow_selection = false;
    m_image = 0;
    m_last_sample = 0;
    m_sonagram_window = 0;
    m_spx_insert_stripe = 0;
    m_stripes = 0;
    m_spx_insert_stripe = new SignalProxy1< StripeInfoPrivate >
	(this, SLOT(insertStripe()));
    m_track_changes = false;
}

//***************************************************************************
SonagramPlugin::~SonagramPlugin()
{
    if (m_sonagram_window) delete m_sonagram_window;
    m_sonagram_window = 0;

    if (m_spx_insert_stripe) delete m_spx_insert_stripe;
    m_spx_insert_stripe = 0;

    if (m_image) delete m_image;
    m_image = 0;
}

//***************************************************************************
QStrList *SonagramPlugin::setup(QStrList *previous_params)
{
    QStrList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params) interpreteParameters(*previous_params);

    SonagramDialog *dlg = new SonagramDialog(*this);
    ASSERT(dlg);
    if (!dlg) return 0;

    dlg->setWindowFunction(m_window_type);
    dlg->setColorMode(m_color ? 1 : 0);
    dlg->setTrackChanges(m_track_changes);
    dlg->setFollowSelection(m_follow_selection);

    if (dlg->exec() == QDialog::Accepted) {
	result = new QStrList();
	ASSERT(result);
	if (result) dlg->parameters(*result);
    };

    delete dlg;
    return result;
}

//***************************************************************************
int SonagramPlugin::interpreteParameters(QStrList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    ASSERT(params.count() == 5);
    if (params.count() != 5) {
	debug("SonagramPlugin::interpreteParams(): params.count()=%d",
	      params.count());
	return -EINVAL;
    }
	
    param = params.at(0);
    m_fft_points = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;
    ASSERT(m_fft_points <= 32767);
    if (m_fft_points > 32767) m_fft_points=32767;

    param = params.at(1);
    m_window_type = WindowFunction::findFromName(param);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params.at(2);
    m_color = (param.toUInt(&ok) != 0);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params.at(3);
    m_track_changes = (param.toUInt(&ok) != 0);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params.at(4);
    m_follow_selection = (param.toUInt(&ok) != 0);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
int SonagramPlugin::start(QStrList &params)
{
    debug("--- SonagramPlugin::start() ---");

    // interprete parameter list and abort if it contains invalid data
    int result = interpreteParameters(params);
    if (result) return result;

    // create an empty sonagram window
    m_sonagram_window = new SonagramWindow(signalName());
    ASSERT(m_sonagram_window);
    if (!m_sonagram_window) return -ENOMEM;

    unsigned int input_length = selection(&m_first_sample, &m_last_sample);
    if (m_first_sample == m_last_sample) {
	input_length = signalLength()-1;
	m_first_sample = 0;
	m_last_sample = input_length-1;
    }

    // calculate the number of stripes (width of image)
    m_stripes = (unsigned int)
	(ceil((double)input_length/(double)m_fft_points));
    ASSERT(m_stripes <= 32767);
    if (m_stripes > 32767) m_stripes = 32767;

    // create a new empty image
    createNewImage(m_stripes, m_fft_points/2, m_color);

    // activate the window with an initial image
    // and all necessary informations
    m_selected_channels = selectedChannels();
    m_sonagram_window->setOverView(overview(
	2*m_sonagram_window->width(), 40, m_first_sample,
	m_last_sample-m_first_sample+1));
    m_sonagram_window->setColorMode((m_color) ? 1 : 0);
    m_sonagram_window->setImage(m_image);
    m_sonagram_window->setPoints(m_fft_points);
    m_sonagram_window->setRate(signalRate());
    m_sonagram_window->show();

    // connect all needed signals
    connect(m_sonagram_window, SIGNAL(destroyed()),
	    this, SLOT(windowClosed()));

    if (m_track_changes) {
	QObject::connect((QObject*)&(manager()),
	    SIGNAL(sigSignalNameChanged(const QString &)),
	    m_sonagram_window, SLOT(setName(const QString &)));
    }

    m_cmd_shutdown = false; // ###

    debug("SonagramPlugin::start() done.");
    return 0;
}

//***************************************************************************
int SonagramPlugin::stop()
{
   debug("int SonagramPlugin::stop()"); // ###
   m_cmd_shutdown = true;
   return KwavePlugin::stop();
}

//***************************************************************************
void SonagramPlugin::run(QStrList params)
{
    debug("SonagramPlugin::run()");

    ASSERT(m_spx_insert_stripe);
    if (!m_spx_insert_stripe) return;

    while (!m_cmd_shutdown) {
	QByteArray *stripe_data;
	unsigned int stripe_nr;
	for (stripe_nr = 0; stripe_nr < m_stripes; stripe_nr++) {
//	    debug("SonagramPlugin::run(): calculating stripe %d of %d",
//	        stripe_nr,m_stripes);

	    // create a new stripe data array
	    stripe_data = new QByteArray(m_fft_points/2);
	    ASSERT(stripe_data);
	    if (!stripe_data) continue;

	    // calculate one stripe
	    calculateStripe(m_first_sample+stripe_nr*m_fft_points,
	        m_fft_points, *stripe_data);

	    StripeInfoPrivate *stripe_info = new
		StripeInfoPrivate(stripe_nr, *stripe_data);

	    // emit the stripe data to be synchronously inserted into
	    // the current image
	    m_spx_insert_stripe->enqueue(*stripe_info);

	    // don't let the pipe grow too much
	    if (m_spx_insert_stripe->count() >= MAX_QUEUE_USAGE) {
		while (m_spx_insert_stripe->count() >= MAX_QUEUE_USAGE/2) {
		    yield();
		    if (m_cmd_shutdown) break; // ###
		}
	    }
	
	    delete stripe_data;
	    delete stripe_info;
	    yield();
	
	    if (m_cmd_shutdown) break; // ###
	}

	m_cmd_shutdown = true;
    }

    debug("SonagramPlugin::run(): done.");
}

//***************************************************************************
void SonagramPlugin::insertStripe()
{
//    debug("SonagramPlugin::insertStripe()");
    ASSERT(m_spx_insert_stripe);
    if (!m_spx_insert_stripe) return;

    StripeInfoPrivate *stripe_info = m_spx_insert_stripe->dequeue();
    ASSERT(stripe_info);
    if (!stripe_info) return;

    unsigned int stripe_nr = stripe_info->nr();
    QByteArray *stripe = &(stripe_info->data());

    // forward the stripe to the window to display it
    ASSERT(m_sonagram_window);
    if (m_sonagram_window) m_sonagram_window->insertStripe(
	stripe_nr, *stripe);

    // remove the stripe info and stripe data, it's our own copy
    delete stripe_info;
}

//***************************************************************************
void SonagramPlugin::calculateStripe(const unsigned int start,
	const unsigned int points, QByteArray &output)
{
    // first initialize the output to zeroes in case of errors
    output.fill(0);

    ASSERT(points);
    if (!points) return;

    WindowFunction func(m_window_type);
    QArray<double> windowfunction = func.points(points);
    ASSERT(windowfunction.count() == points);
    if (windowfunction.count() != points) return;

    complex *data = new complex[points];
    ASSERT(data);
    if (!data) return;

    // initialize the table for fft
    gsl_fft_complex_wavetable table;
    gsl_fft_complex_wavetable_alloc(points, &table);
    gsl_fft_complex_init(points, &table);
    	
    // copy signal data into complex array
    for (unsigned int j = 0; j < points; j++) {
	unsigned int sample_nr = start + j;
	double value;

	value = (sample_nr < m_last_sample) ?
	    (double)averageSample(sample_nr,
	    &m_selected_channels)/(double)(1<<23): 0.0;

	data[j].real = windowfunction[j] * value;
	data[j].imag = 0;
    }

    // calculate the fft
    gsl_fft_complex_forward(data, points, &table);

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
	for (unsigned int j = 0; j < points/2; j++) {
	    rea = data[j].real;
	    ima = data[j].imag;
	    rea = sqrt(rea * rea + ima * ima) / max;

	    // get amplitude and scale to 1
	    rea = 1 - ((1 - rea) * (1 - rea));

	    output[j] = 0xFE - (int)(rea * 0xFE );
	}
    }

    // free the intermediate array used for fft
    gsl_fft_complex_wavetable_free(&table);
}

//***************************************************************************
void SonagramPlugin::createNewImage(const unsigned int width,
	const unsigned int height, const bool color)
{
    debug("SonagramPlugin::createNewImage()");

    // delete the previous image
    if (m_image) delete m_image;
    m_image = 0;

    // do not create a new image if one dimension is zero!
    ASSERT(width);
    ASSERT(height);
    if (!width || !height) return;

    // also do not create if the image size is out of range
    ASSERT(width <= 32767);
    ASSERT(height <= 32767);
    if ((width >= 32767) || (height >= 32767)) return;

    debug("SonagramPlugin::createNewImage(): settings ok, creating image");

    // create the new image object
    m_image = new QImage(width, height, 8, 256);
    ASSERT(m_image);
    if (!m_image) return;
    m_image->setAlphaBuffer(true);

    // initialize the image's palette with transparecy
    for (int i=0; i < 256; i++) {
	m_image->setColor(i, 0x00000000);
    }

    // fill the image with "empty"
    m_image->fill(0xFF);

    debug("SonagramPlugin::createNewImage(): done.");
}

//***************************************************************************
void SonagramPlugin::windowClosed()
{
    // the SonagramWindow closes itself !
    m_sonagram_window = 0;

    // close the plugin too
    close();
}

//***************************************************************************
//***************************************************************************
