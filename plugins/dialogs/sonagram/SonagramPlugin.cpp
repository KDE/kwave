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

#include <libkwave/WindowFunction.h>

#include <libgui/KwavePlugin.h>
#include <libgui/SignalProxy.h>

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(SonagramPlugin,"sonagram","Martin Wilz");

#define MAX_QUEUE_USAGE 16

//***************************************************************************
SonagramPlugin::SonagramPlugin(PluginContext &c)
    :KwavePlugin(c)
{
    color = true;
    fft_points = 0;
    first_sample = 0;
    image = 0;
    last_sample = 0;
    sonagram_window = 0;
    spx_insert_stripe = 0;
    stripes = 0;
}

//***************************************************************************
SonagramPlugin::~SonagramPlugin()
{
    if (spx_insert_stripe) delete spx_insert_stripe;
    spx_insert_stripe = 0;

    if (sonagram_window) delete sonagram_window;
    sonagram_window = 0;

    if (image) delete image;
    image = 0;
}

//***************************************************************************
QStrList *SonagramPlugin::setup(QStrList *previous_params)
{
    QStrList *result = 0;

    SonagramDialog *dlg = new SonagramDialog(*this);
    ASSERT(dlg);
    if (!dlg) return 0;

    if (dlg->exec() == QDialog::Accepted) {
	result = new QStrList();
	ASSERT(result);
	if (result) dlg->getParameters(*result);
    };

    delete dlg;
    return result;
}

//***************************************************************************
int SonagramPlugin::start(QStrList &params)
{
    debug("--- SonagramPlugin::start() ---");

    // evaluate the parameter list
    ASSERT(params.count() == 3);
    if (params.count() != 3) {
	debug("SonagramPlugin::start(): params.count()=%d",params.count());
	return -EINVAL;
    }

    bool ok;
    QString param;
	
    param = params.at(0);
    fft_points = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;
    ASSERT(fft_points <= 32767);
    if (fft_points > 32767) fft_points=32767;
	
    param = params.at(1);
    window_type = param.toUInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params.at(2);
    color = (param.toUInt(&ok) != 0);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // create an empty sonagram window
    sonagram_window = new SonagramWindow(getSignalName());
    ASSERT(sonagram_window);
    if (!sonagram_window) return -ENOMEM;

    unsigned int input_length = getSelection(&first_sample, &last_sample);
    if (first_sample == last_sample) {
	input_length = getSignalLength()-1;
	first_sample = 0;
	last_sample = input_length-1;
    }

    // calculate the number of stripes (width of image)
    stripes = (unsigned int)
	(ceil((double)input_length/(double)fft_points));
    ASSERT(stripes <= 32767);
    if (stripes > 32767) stripes = 32767;

    // create a new empty image
    createNewImage(stripes, fft_points/2, color);

    // activate the window with an initial image
    sonagram_window->setImage(image);
    sonagram_window->show();

    // connect all needed signals
    connect(sonagram_window, SIGNAL(destroyed()),
	    this, SLOT(windowClosed()));

    QObject::connect((QObject*)&(manager()),
	SIGNAL(sigSignalNameChanged(const QString &)),
	sonagram_window, SLOT(setName(const QString &)));

    debug("SonagramPlugin::start() done.");
    return 0;
}

//***************************************************************************
void SonagramPlugin::run(QStrList params)
{
    debug("SonagramPlugin::run()");

    if (!spx_insert_stripe) spx_insert_stripe =
	new SignalProxy1< pair<unsigned int, QByteArray> >
	(this, SLOT(insertStripe()));

    bool done = false;
    while (!done) {
	QByteArray *stripe_data;
	unsigned int stripe_nr;
	for (stripe_nr = 0; stripe_nr < stripes; stripe_nr++) {
	    debug("SonagramPlugin::run(): calculating stripe %d of %d",stripe_nr,stripes);
	
	    // create a new stripe data array
	    stripe_data = new QByteArray(fft_points/2);
	    ASSERT(stripe_data);
	    if (!stripe_data) continue;

	    // calculate one stripe
	    calculateStripe(first_sample+stripe_nr*fft_points, fft_points,
	    	*stripe_data);
	
	    pair<unsigned int, QByteArray> stripe_info(stripe_nr, *stripe_data);
	
	    // emit the stripe data to be synchronously inserted into
	    // the current image
	    spx_insert_stripe->enqueue(stripe_info);
	
	    while (spx_insert_stripe->count() >= MAX_QUEUE_USAGE)
		yield();
	
	    delete stripe_data;
	}

	done = true;
    }
}

//***************************************************************************
void SonagramPlugin::insertStripe()
{
    debug("SonagramPlugin::insertStripe()");
    ASSERT(spx_insert_stripe);
    if (!spx_insert_stripe) return;

    pair<unsigned int,QByteArray> *stripe_info = spx_insert_stripe->dequeue();
    ASSERT(stripe_info);
    if (!stripe_info) return;

    unsigned int stripe_nr = stripe_info->first;
    debug("SonagramPlugin::insertStripe(): stripe nr = %d",stripe_nr);
    QByteArray stripe = stripe_info->second;
    ASSERT(stripe);
    if (!stripe) return;

    // forward the stripe to the window to display it
    ASSERT(sonagram_window);
    if (sonagram_window) sonagram_window->insertStripe(
	stripe_nr, stripe);

    // remove the stripe data, it's our own copy
    delete stripe;
}

//***************************************************************************
void SonagramPlugin::calculateStripe(const unsigned int start,
	const int points, QByteArray &output)
{
    // first initialize the output to zeroes in case of errors
    output.fill(0);

    ASSERT(points);
    if (!points) return;

    WindowFunction func(window_type);
    double *windowfunction = func.getFunction(points);
    ASSERT(windowfunction);
    if (!windowfunction) return;

    complex *data = new complex[points];
    ASSERT(data);
    if (!data) return;

    // initialize the table for fft
    gsl_fft_complex_wavetable table;
    gsl_fft_complex_wavetable_alloc(points, &table);
    gsl_fft_complex_init(points, &table);
    	
    // copy signal data into complex array
    for (int j = 0; j < points; j++) {
	unsigned int sample_nr = start + j;
	double value;

	value = (sample_nr < last_sample) ?
	    (double)getSingleSample(0, sample_nr)/(double)(1<<23): 0.0;

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
    double max = fft_points/2;

    // norm all values to [0...255] and use them as pixel value
    if (max != 0) {
	for (int j = 0; j < points/2; j++) {
	    rea = data[j].real;
	    ima = data[j].imag;
	    rea = sqrt(rea * rea + ima * ima) / max;

	    // get amplitude and scale to 1
	    rea = 1 - ((1 - rea) * (1 - rea));

	    output[j] = 255-(int)(rea*255);
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
    if (image) delete image;
    image = 0;

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
    image = new QImage(width, height, 8, 256);
    ASSERT(image);
    if (!image) return;

    // set the image's palette
    debug("SonagramPlugin::createNewImage(): setting palette");
    QColor c;
    for (int i=0; i < 256; i++) {
	if (color) {
	    // rainbow effect
	    c.setHsv( (i*255)/256, 255, 255 );
	} else {
	    // grayscale palette
	    c.setRgb(i, i, i);
	}
	image->setColor(i, c.rgb());
    }

    // fill the image with "empty"
    image->fill(0xFF);

    debug("SonagramPlugin::createNewImage(): done.");
}

//***************************************************************************
void SonagramPlugin::windowClosed()
{
    // the SonagramWindow closes itself !
    sonagram_window = 0;

    // close the plugin too
    close();
}

//***************************************************************************
//***************************************************************************
