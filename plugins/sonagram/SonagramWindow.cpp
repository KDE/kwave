/***************************************************************************
              SonagramWindow.cpp  -  window for showing a sonagram
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

#include <qbitmap.h>
#include <qimage.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kmenubar.h>
#include <kfiledialog.h>
#include <kstatusbar.h>

#include "libkwave/KwavePlugin.h"
#include "libkwave/WindowFunction.h"
#include "libgui/ScaleWidget.h"

#include "ImageView.h"
#include "SonagramWindow.h"

/**
 * delay between two screen updates [ms]
 */
#define REFRESH_DELAY 100

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

/**
 * Color values below this limit are cut off when adjusting the
 * sonagram image's brightness
 * I found out by experiments that 0.1% seems to be reasonable
 */
#define COLOR_CUTOFF_RATIO (0.1/100.0)

static const char *background[] = {
/* width height num_colors chars_per_pixel */
"     20     20          2               1",
/* colors */
"# c #808080",
". c None",
/* pixels */
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"##########..........",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########",
"..........##########"
};

//****************************************************************************
SonagramWindow::SonagramWindow(const QString &name)
    :KMainWindow()
{
    m_color_mode = 0;
    m_image = 0;
    m_overview = 0;
    m_points = 0;
    m_rate = 0;
    m_view = 0;
    m_xscale = 0;
    m_yscale = 0;
    for (int i=0; i<256; m_histogram[i++] = 0) {}

    QWidget *mainwidget = new QWidget(this);
    Q_ASSERT(mainwidget);
    if (!mainwidget) return;
    setCentralWidget(mainwidget);

    QGridLayout *top_layout = new QGridLayout(mainwidget, 3, 2);
    Q_ASSERT(top_layout);
    if (!top_layout) return;

    KMenuBar *bar = menuBar();
    Q_ASSERT(bar);
    if (!bar) return ;

//    QPopupMenu *spectral = new QPopupMenu();
//    Q_ASSERT(spectral);
//    if (!spectral) return ;

    QPopupMenu *file = new QPopupMenu();
    Q_ASSERT(file);
    if (!file) return ;

    bar->insertItem(i18n("&Sonagram"), file);
//    bar->insertItem(i18n("&Spectral Data"), spectral);
//
//    file->insertItem(i18n("&Import from Bitmap ..."), this, SLOT(load()));
    file->insertItem(i18n("&Export to Bitmap ..."), this, SLOT(save()));
    file->insertItem(i18n("E&xit"), this, SLOT(close()));
//
//    spectral->insertItem (i18n("&reTransform to signal"), this, SLOT(toSignal()));

    KStatusBar *status = statusBar();
    Q_ASSERT(status);
    if (!status) return ;

    status->insertItem(i18n("Time: ------ ms"), 1);
    status->insertItem(i18n("Frequency: ------ Hz"), 2);
    status->insertItem(i18n("Amplitude: --- %"), 3);

    m_view = new ImageView(mainwidget);
    Q_ASSERT(m_view);
    if (!m_view) return;
    top_layout->addWidget(m_view, 0, 1);
    m_view->setBackgroundPixmap(QPixmap(background));

    m_xscale = new ScaleWidget(mainwidget, 0, 100, "ms");
    Q_ASSERT(m_xscale);
    if (!m_xscale) return;
    m_xscale->setFixedHeight(m_xscale->sizeHint().height());
    top_layout->addWidget(m_xscale, 1, 1);

    m_yscale = new ScaleWidget(mainwidget, 0, 100, "Hz");
    Q_ASSERT(m_yscale);
    if (!m_yscale) return ;
    m_yscale->setFixedWidth(m_yscale->sizeHint().width());
    m_yscale->setMinimumHeight(9*6*5);
    top_layout->addWidget(m_yscale, 0, 0);

    m_overview = new ImageView(mainwidget);
    Q_ASSERT(m_overview);
    if (!m_overview) return;
    m_overview->setFixedHeight(30);
    top_layout->addWidget(m_overview, 2, 1);

    connect(m_view, SIGNAL(sigCursorPos(const QPoint)),
	    this, SLOT(cursorPosChanged(const QPoint)));
    connect(&m_refresh_timer, SIGNAL(timeout()),
            this, SLOT(refresh_view()));

    setName(name);

    top_layout->setRowStretch(0, 100);
    top_layout->setRowStretch(1, 0);
    top_layout->setRowStretch(2, 0);
    top_layout->setColStretch(0, 0);
    top_layout->setColStretch(1, 100);
    top_layout->activate();

    status->changeItem(i18n("Time: 0 ms"), 1);
    status->changeItem(i18n("Frequency: 0 Hz"), 2);
    status->changeItem(i18n("Amplitude: 0 %"), 3);

    // try to make 5:3 format (looks best)
    int w = sizeHint().width();
    int h = sizeHint().height();
    if ((w * 3 / 5) < h) w = (h * 5) / 3;
    if ((h * 5 / 3) < w) h = (w * 3) / 5;
    resize(w, h);

    show();
}

//****************************************************************************
void SonagramWindow::close()
{
    QWidget::close();
}

//****************************************************************************
void SonagramWindow::save()
{
    Q_ASSERT(m_image);
    if (!m_image) return;

    QString filename = KFileDialog::getSaveFileName("", "*.bmp", this);
    if (!filename.isEmpty()) m_image->save(filename, "BMP");
}

//****************************************************************************
void SonagramWindow::load()
{
//    if (image) {
//	QString filename = KFileDialog::getOpenFileName("", "*.bmp", this);
//	printf ("loading %s\n", filename.local8Bit().data());
//	if (!filename.isNull()) {
//	    printf ("loading %s\n", filename.local8Bit().data());
//	    QImage *newimage = new QImage (filename);
//	    Q_ASSERT(newimage);
//	    if (newimage) {
//		if ((image->height() == newimage->height())
//		    && (image->width() == newimage->width())) {
//
//		    for (int i = 0; i < x; i++) {
//			for (int j = 0; j < points / 2; j++) {
//			    if (data[i]) {
//				// data[i][j].real;
//			    }
//
//			}
//		    }
//
//		    delete image;
//		    image = newimage;
//		    view->setImage (image);
//		} else {
//		    char buf[128];
//		    delete newimage;
//		    snprintf(buf, sizeof(buf), i18n("Bitmap must be %dx%d"),
//			     image->width(), image->height());
//		    KMsgBox::message (this, "Info", buf, 2);
//		}
//	    } else
//		KMsgBox::message (this, i18n("Error"),
//				  i18n("Could not open Bitmap"), 2);
//	}
//    }
}

//****************************************************************************
void SonagramWindow::setImage(QImage *image)
{
    Q_ASSERT(m_view);
    if (!m_view) return;

    m_image = image;

    // re-initialize histogram over all pixels
    for (int i=0; i<256; i++) m_histogram[i] = 0;
    if (m_image) {
	for (int x=0; x < m_image->width(); x++) {
	    for (int y=0; y < m_image->height(); y++) {
		unsigned char p = m_image->pixelIndex(x, y);
		m_histogram[p]++;
	    }
	}
    }

    m_view->setImage(m_image);
    refresh_view();
}

//****************************************************************************
void SonagramWindow::setOverView(QBitmap *overview)
{
    QImage *image = 0;
    if (!m_overview) return;
    if (overview) {
	image = new QImage(overview->convertToImage());
	Q_ASSERT(image);
    }
    m_overview->setImage(image);
    if (image) delete image;
}

//****************************************************************************
void SonagramWindow::insertStripe(const unsigned int stripe_nr,
	const QByteArray &stripe)
{
    Q_ASSERT(m_view);
    Q_ASSERT(m_image);
    if (!m_view) return;
    if (!m_image) return;

    unsigned int image_width  = m_image->width();
    unsigned int image_height = m_image->height();

    // stripe is out of range ?
    Q_ASSERT(stripe_nr < image_width);
    if ((stripe_nr) >= image_width) return;

    unsigned int y;
    unsigned int size = stripe.size();
    for (y=0; y < size; y++) {
    	unsigned char p;

    	// remove the current pixel from the histogram
    	p = m_image->pixelIndex(stripe_nr, y);
	m_histogram[p]--;

	// set the new pixel value
	p = (unsigned char)stripe[size-y-1];
	m_image->setPixel(stripe_nr, y, p);

    	// insert the new pixel into the histogram
	m_histogram[p]++;
    }
    while (y < image_height) { // fill the rest with blank
	m_image->setPixel(stripe_nr, y++, 0xFE);
	m_histogram[0xFE]++;
    }

    if (!m_refresh_timer.isActive()) {
	m_refresh_timer.start(REFRESH_DELAY, true);
    }
}

//****************************************************************************
void SonagramWindow::adjustBrightness()
{
    Q_ASSERT(m_image);
    if (!m_image) return;

    // get the sum of pixels != 0
    unsigned long int sum = 0;
    for (int i=0; i<254; i++) {
	sum += m_histogram[i];
    }
    // cut off all parts below the cutoff ration (e.g. 0.1%)
    unsigned int cutoff = (int)(sum * COLOR_CUTOFF_RATIO);

    // get the first used color
    unsigned int first=0;
    while ((first < 253) && (m_histogram[first] <= cutoff)) {
	first++;
    }

    QColor c;
    for (unsigned int i=0; i < 255; i++) {
	int v;

	if (i <= first) {
	    v = 0;
	} else {
	    // map [first...254] to [0...254]
	    v = (i-first)*254/(254-first);
	}

	if (m_color_mode == 1) {
	    // rainbow effect
	    c.setHsv( (v*255)/256, 255, 255 );
	} else {
	    // greyscale palette
	    c.setRgb(v, v, v);
	}

	m_image->setColor(i, c.rgb() | 0xFF000000);
    }

    // use color 0xFF for transparency !
    m_image->setColor(0xFF, 0x00000000);

}

//****************************************************************************
void SonagramWindow::refresh_view()
{
    Q_ASSERT(m_view);
    if (!m_view) return;
    if (m_image) adjustBrightness();
    m_view->repaint(false);
}

//****************************************************************************
void SonagramWindow::toSignal()
{
//    gsl_fft_complex_wavetable table;
//
//    gsl_fft_complex_wavetable_alloc (points, &table);
//    gsl_fft_complex_init (points, &table);
//
//    TopWidget *win = new TopWidget(
//	*((KwaveApp*)KApplication::getKApplication()),
//	((KwaveApp*)KApplication::getKApplication())->getRecentFiles()
//    );
//
//    Q_ASSERT(win);
//    if (win) {
//
//	Signal *newsig = new Signal (length, rate);
//	Q_ASSERT(newsig);
//
//	//assure 10 Hz for correction signal, this should not be audible
//	int slopesize = rate / 10;
//
//	double *slope = new double [slopesize];
//
//	if (slope && newsig) {
//	    for (int i = 0; i < slopesize; i++)
//		slope[i] = 0.5 + 0.5 * cos( ((double) i) * M_PI / slopesize);
//
//	    win->show();
//
//	    int *output = newsig->getSample();      //sample data
//	    complex *tmp = new complex [points];   //this window holds the data for ifft and after that part of the signal
//
//	    if (output && tmp && data) {
//		for (int i = 0; i < x; i++) {
//		    if (data[i]) memcpy (tmp, data[i], sizeof(complex)*points);
//		    gsl_fft_complex_inverse (tmp, points, &table);
//
//		    for (int j = 0; j < points; j++)
//			output[i*points + j] = (int)(tmp[j].real * ((1 << 23)-1));
//		}
//		int dif ;
//		int max;
//		for (int i = 1; i < x; i++) //remove gaps between windows
//		{
//		    max = slopesize;
//		    if (max > length - i*points) max = length - i * points;
//		    dif = output[i * points] - output[i * points - 1];
//		    if (dif < 2)
//			for (int j = 0; j < max; j++) output[i*points + j] += (int) (slope[j] * dif );
//		}
//
//		win->setSignal (new SignalManager (newsig));
//
//		if (tmp) delete[] tmp;
//	    } else {
//		if (newsig) delete newsig;
//		if (win) delete win;
//		KMsgBox::message (this, i18n("Error"), i18n("Out of memory !"), 2);
//	    }
//	}
//	if (slope) delete[] slope;
//    }
}

//***************************************************************************
void SonagramWindow::translatePixels2TF(const QPoint p, double *ms, double *f)
{
    if (ms) {
	// get the time coordinate [0...(N_samples-1)* (1/f_sample) ]
	if (m_rate != 0) {
	    *ms = (double)p.x() * (double)m_points * 1000.0 / m_rate;
	} else {
	    *ms = 0;
	}
    }

    if (f) {
	// get the frequency coordinate
	double py = (m_points >= 2) ? (m_points / 2) - 1 : 0;
	double y = py - p.y();
	if (y < 0) y = 0;
	*f = y / py * (m_rate/2.0);
    }
}

//***************************************************************************
void SonagramWindow::updateScaleWidgets()
{
    double ms;
    double f;

    translatePixels2TF(QPoint(m_image->width()-1, 0), &ms, &f);

    m_xscale->setMinMax(0, (int)rint(ms));
    m_yscale->setMinMax(0, (int)rint(f));
}

//***************************************************************************
SonagramWindow::~SonagramWindow()
{
}

//***************************************************************************
void SonagramWindow::setColorMode(int mode)
{
    Q_ASSERT(mode >= 0);
    Q_ASSERT(mode <= 1);

    if (mode != m_color_mode) {
	m_color_mode = mode;
	if (m_image) setImage(m_image);
    }
}

//***************************************************************************
void SonagramWindow::setName(const QString &name)
{
    QString windowname;
    windowname += i18n("Sonagram of ");
    windowname += name.length() ? name : QString(i18n("<nothing>"));

    setCaption(windowname);
}

//****************************************************************************
void SonagramWindow::cursorPosChanged(const QPoint pos)
{
    KStatusBar *status = statusBar();
    Q_ASSERT(status);
    Q_ASSERT(m_image);
    Q_ASSERT(m_points);
    Q_ASSERT(m_rate != 0);
    if (!status) return ;
    if (!m_image) return ;
    if (!m_points) return;
    if (m_rate == 0) return;

    char buf[64];
    double ms;
    double f;
    double a;
    translatePixels2TF(pos, &ms, &f);

    // item 1: time in milliseconds
    status->changeItem(i18n("Time: %1").arg(
	KwavePlugin::ms2string(ms)), 1);

    // item 2: frequency in Hz
    snprintf(buf, sizeof(buf), i18n("Frequency: %d Hz"), (int)f);
    status->changeItem(buf, 2);

    // item 3: amplitude in %
    if (m_image->valid(pos.x(), pos.y())) {
	a = (254.0-m_image->pixelIndex(pos.x(), pos.y())) *
	    (100.0 / 254.0);
    } else {
	a = 0.0;
    }
    snprintf(buf, sizeof(buf), i18n("Amplitude: %d %%"), (int)a);
    status->changeItem(buf, 3);
}

//****************************************************************************
void SonagramWindow::setPoints(unsigned int points)
{
    m_points = points;
    updateScaleWidgets();
}

//****************************************************************************
void SonagramWindow::setRate(double rate)
{
    m_rate = rate;
    updateScaleWidgets();
}

//****************************************************************************
//****************************************************************************
