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

//#include <qdir.h>
//#include <qfiledialog.h>
#include <qimage.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kapp.h>
//#include <kmsgbox.h>

#include <libkwave/WindowFunction.h>

#include "libgui/KwavePlugin.h"
#include "libgui/OverViewWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "../../../src/ImageView.h"

#include "SonagramWindow.h"

//#include <qtimer.h>
//#include <qpushbt.h>
//#include <qstring.h>
//#include <qpainter.h>
//#include <qpixmap.h>
//#include <kmenubar.h>
//#include <kstatusbar.h>

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

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
:KTMainWindow()
{
    m_image = 0;
    m_overview = 0;
    m_points = 0;
    m_rate = 0;
    m_status = 0;
    m_view = 0;
    m_xscale = 0;
    m_yscale = 0;

    QWidget *mainwidget = new QWidget(this);
    ASSERT(mainwidget);
    if (!mainwidget) return;
    setView(mainwidget);

    QGridLayout *top_layout = new QGridLayout(mainwidget, 3, 2);
    ASSERT(top_layout);
    if (!top_layout) return;

    KMenuBar *bar = new KMenuBar(this);
    ASSERT(bar);
    if (!bar) return ;

//    QPopupMenu *spectral = new QPopupMenu();
//    ASSERT(spectral);
//    if (!spectral) return ;

    QPopupMenu *file = new QPopupMenu();
    ASSERT(file);
    if (!file) return ;

    bar->insertItem(i18n("&Sonagram"), file);
//    bar->insertItem(i18n("&Spectral Data"), spectral);
//
//    file->insertItem(i18n("&Import from Bitmap ..."), this, SLOT(load()));
//    file->insertItem(i18n("&Export to Bitmap ..."), this, SLOT(save()));
    file->insertItem(i18n("E&xit"), this, SLOT(close()));
//
//    spectral->insertItem (i18n("&reTransform to signal"), this, SLOT(toSignal()));

    m_status = new KStatusBar (this, i18n("Frequencies Status Bar"));
    ASSERT(m_status);
    if (!m_status) return ;

    m_status->insertItem(i18n("Time: ------ ms"), 1);
    m_status->insertItem(i18n("Frequency: ------ Hz"), 2);
    m_status->insertItem(i18n("Amplitude: --- %"), 3);
//    m_status->insertItem(i18n("Phase: -----"), 4);

    m_view = new ImageView(mainwidget);
    ASSERT(m_view);
    if (!m_view) return;
    top_layout->addWidget(m_view, 0, 1);
    m_view->setBackgroundPixmap(QPixmap(background));

    m_xscale = new ScaleWidget(mainwidget, 0, 100, "ms");
    ASSERT(m_xscale);
    if (!m_xscale) return;
    m_xscale->setFixedHeight(m_xscale->sizeHint().height());
    top_layout->addWidget(m_xscale, 1, 1);

    m_yscale = new ScaleWidget(mainwidget, 100, 0, "Hz");
    ASSERT(m_yscale);
    if (!m_yscale) return ;
    m_yscale->setFixedWidth(m_yscale->sizeHint().width());
    top_layout->addWidget(m_yscale, 0, 0);

    m_overview = new OverViewWidget(mainwidget);
    ASSERT(m_overview);
    if (!m_overview) return;
    m_overview->setFixedHeight(m_overview->sizeHint().height());
    top_layout->addWidget(m_overview, 2, 1);

    connect(m_view, SIGNAL(sigCursorPos(const QPoint)),
	    this, SLOT(cursorPosChanged(const QPoint)));
    connect(m_overview, SIGNAL(valueChanged(int)),
	    m_view, SLOT(setOffset(int)));
    connect(m_view, SIGNAL(viewInfo(int, int, int)),
	    this, SLOT(setRange(int, int, int)));
    connect(m_view, SIGNAL(viewInfo(int, int, int)),
	    m_overview, SLOT(setRange(int, int, int)));
    connect(&m_refresh_timer, SIGNAL(timeout()),
            this, SLOT(refresh_view()));
		
    setStatusBar(m_status);
    setMenu(bar);
    setName(name);

    top_layout->setRowStretch(0, 100);
    top_layout->setRowStretch(1, 0);
    top_layout->setRowStretch(2, 0);
    top_layout->setColStretch(0, 0);
    top_layout->setColStretch(1, 100);
    top_layout->activate();

    m_status->changeItem(i18n("Time: 0 ms"), 1);
    m_status->changeItem(i18n("Frequency: 0 s"), 2);
    m_status->changeItem(i18n("Amplitude: 0 %"), 3);

    resize(max(480,m_status->sizeHint().width()+40), 320);
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
//    if (image) {
//	QString filename = QFileDialog::getSaveFileName("", "*.bmp", this);
//	if ( !filename.isEmpty() ) image->save( filename, "BMP" );
//    }
}

//****************************************************************************
void SonagramWindow::load()
{
//    if (image) {
//	QString filename = QFileDialog::getOpenFileName("", "*.bmp", this);
//	printf ("loading %s\n", filename.data());
//	if (!filename.isNull()) {
//	    printf ("loading %s\n", filename.data());
//	    QImage *newimage = new QImage (filename);
//	    ASSERT(newimage);
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

////****************************************************************************
//void SonagramWindow::setSignal(double *input, int size, int points,
//			       int windowtype, int rate)
//{
//    ASSERT(points);
//    ASSERT(rate);
//    if (!points) return ;
//    if (!rate) return ;
//
//    double rea, ima;
//
//    windowtype=0;
//
//    debug("SonagramWindow::setSignal: size=%d, points=%d, windowtype=%d, rate=%d",
//	size,points,windowtype,rate);
//
//    debug("SonagramWindow::setSignal: size=%d",size);
//    debug("SonagramWindow::setSignal: points=%d",points);
//
//    this->length = size;
//    this->points = points;
//    this->rate = rate;
//    this->image_width = size;
//
//    // use at least 32 pixels for image
//    stripes = max(1, image_width/points);
//
//    debug("SonagramWindow::setSignal:--1--");
//
//    yscale ->setMaxMin (0, rate / 2);
//    xscale ->setMaxMin ((int)(((double)size) / rate*1000), 0);
//
//    debug("SonagramWindow::setSignal:--2--: stripes=%d",stripes);
//
//    data = new complex *[stripes];
//    ASSERT(data);
//
//    debug("SonagramWindow::setSignal:--2a--");
//
//    WindowFunction func(windowtype);
//
//    debug("SonagramWindow::setSignal:--3--");
//
//    double* windowfunction = func.getFunction(points);
//    ASSERT(windowfunction);
//
//    debug("SonagramWindow::setSignal:--4--");
//
//    ASSERT(data);
//    ASSERT(image);
//    ASSERT(windowfunction);
//    if ((data) && (image) && windowfunction) {
//
//	debug("SonagramWindow::setSignal:--5--");
//	gsl_fft_complex_wavetable table;
//
//	debug("SonagramWindow::setSignal:--6--");
//	gsl_fft_complex_wavetable_alloc (points, &table);
//	debug("SonagramWindow::setSignal:--7--");
//	gsl_fft_complex_init (points, &table);
//	debug("SonagramWindow::setSignal:--8--");
//
//	for (int i = 0; i < stripes; i++) {
//	    complex *output = new complex[points];
//	    ASSERT(output);
//	    if (output) {
//		for (int j = 0; j < points; j++) {
//		    output[j].real = windowfunction[j] * input[i * points + j];
//		    //copy data into complex array
//		    output[j].imag = 0;
//       	        }
//		gsl_fft_complex_forward (output, points, &table);
//
//		for (int k = 0; k < points; k++) {
//		    rea = output[k].real;
//		    ima = output[k].imag;
//		    rea = sqrt(rea * rea + ima * ima);
//		    //get amplitude
//		    if (max < rea) max = rea;
//		    //and set maximum for display..
//		}
//	    }
//	    debug("SonagramWindow::setSignal: fft %d of %d",i,stripes);
//	    data[i] = output;
//	}
//	gsl_fft_complex_wavetable_free (&table);
//    }
//
//    debug("SonagramWindow::setSignal:--A--");
//
//    // double rea, ima;
//    for (int i = 0; i < stripes; i++) {
//	if (!data[i]) continue;
//	
//	for (int j = 0; j < points / 2; j++) {
//	    rea = data[i][j].real;
//	    ima = data[i][j].imag;
//	    rea = sqrt(rea * rea + ima * ima) / max;
//
//	    //get amplitude and scale to 1
//	    rea = 1 - ((1 - rea) * (1 - rea));
//	    *(image->scanLine((points / 2 - 1)-j) + i) = 255-(int)(rea * 255);
//	}
//    }
//
////    createImage();
//    view->setImage(image);
//    show();
//}

//****************************************************************************
void SonagramWindow::setImage(QImage *image)
{
    ASSERT(m_view);
    if (!m_view) return;

    m_image = image;
    m_view->setImage(m_image);
    m_view->repaint();
}

//****************************************************************************
void SonagramWindow::insertStripe(const unsigned int stripe_nr,
	const QByteArray &stripe)
{
    ASSERT(m_view);
    ASSERT(m_image);
    if (!m_view) return;
    if (!m_image) return;

    unsigned int image_width  = m_image->width();
    unsigned int image_height = m_image->height();

    // stripe is out of range ?
    ASSERT(stripe_nr < image_width);
    if ((stripe_nr) >= image_width) return;

    unsigned int y;
    unsigned int size = stripe.size();
    for (y=0; y < size; y++) {
	m_image->setPixel(stripe_nr, y, (unsigned char)stripe[size-y-1]);
    }
    while (y < image_height)
	m_image->setPixel(stripe_nr, y++, 0xFE);

    if (!m_refresh_timer.isActive()) {
	m_refresh_timer.start(100, true);
    }
}

//****************************************************************************
void SonagramWindow::refresh_view()
{
    ASSERT(m_view);
    if (m_view) m_view->repaint(false);
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
//    ASSERT(win);
//    if (win) {
//
//	Signal *newsig = new Signal (length, rate);
//	ASSERT(newsig);
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

//****************************************************************************
void SonagramWindow::clear()
{
//    if (data) {
//	for (int i = 0; i < image_width; i++)
//	    if (data[i]) delete data[i];
//	delete[] data;
//    }
//    data = 0;
//
//    if (image) delete image;
//    image = 0;
}

//***************************************************************************
void SonagramWindow::translatePixels2TF(const QPoint p, double *ms, double *f)
{
    if (ms) {
	// get the time coordinate [0...(N_samples-1)* (1/f_sample) ]
	if (m_rate != 0) {
	    *ms = (double)p.x() * (double)m_points * 1000.0 / (double)m_rate;
	} else {
	    *ms = 0;
	}
    }

    if (f) {
	// get the frequency coordinate
	double y = ((m_points/2)-1) - p.y();
	*f = y / (double)(m_points/2-1) * m_rate;
    }
}

//***************************************************************************
void SonagramWindow::updateScaleWidgets()
{
    double ms;
    double f;

    translatePixels2TF(QPoint(m_image->width()-1, 0), &ms, &f);

    m_xscale->setMaxMin(ms, 0);
    m_yscale->setMaxMin(0, f);
}

//***************************************************************************
SonagramWindow::~SonagramWindow()
{
    clear();
}

//***************************************************************************
void SonagramWindow::setName(const QString &name)
{
    QString windowname("Kwave - ");
    windowname += i18n("Sonagram of ");

    setCaption(windowname.data());
    windowname += name.length() ? name : QString(i18n("<nothing>"));

    setCaption(windowname.data());
}

//****************************************************************************
void SonagramWindow::cursorPosChanged(const QPoint pos)
{
    ASSERT(m_status);
    ASSERT(m_image);
    ASSERT(m_points);
    ASSERT(m_rate);
    if (!m_status) return ;
    if (!m_image) return ;
    if (!m_points) return;
    if (!m_rate) return;

    char buf[64];
    double ms;
    double f;
    double a;
    translatePixels2TF(pos, &ms, &f);

    // item 1: time in milliseconds
    char ms_buf[32];
    KwavePlugin::ms2string(ms_buf, sizeof(ms_buf), ms);
    snprintf(buf, sizeof(buf), i18n("Time: %s"), ms_buf);
    m_status->changeItem(buf, 1);

    // item 2: frequency in Hz
    snprintf(buf, sizeof(buf), i18n("Frequency: %d Hz"), (int)f);
    m_status->changeItem(buf, 2);

    // item 3: amplitude in %
    if (m_image->valid(pos.x(), pos.y())) {
	a = (254.0-m_image->pixelIndex(pos.x(), pos.y())) *
	    (100.0 / 254.0);
    } else {
	a = 0.0;
    }
    snprintf(buf, sizeof(buf), i18n("Amplitude: %d %%"), (int)a);
    m_status->changeItem(buf, 3);

//    if (data [(int)(x*image_width)]) {
//	double rea = data [col][(int)(y * points / 2)].real;
//	double ima = data [col][(int)(y * points / 2)].imag;
//
//	snprintf(buf, sizeof(buf), i18n("Amplitude: %d %%"),
//		 (int)(sqrt(rea*rea + ima*ima) / max*100));
//    } else snprintf(buf, sizeof(buf), i18n("Memory Leak !"));
//    status->changeItem (buf, 3);
//
//    if (data [(int)(x*image_width)]) {
//	double rea = data [col][(int)(y * points / 2)].real;
//	double ima = data [col][(int)(y * points / 2)].imag;
//	snprintf(buf, sizeof(buf), i18n("Phase: %d degree"),
//		 (int) (atan(ima / rea)*360 / M_PI));
//    } else snprintf(buf, sizeof(buf), i18n("Memory Leak !"));
//
//    status->changeItem (buf, 4);
}

//****************************************************************************
void SonagramWindow::setPoints(unsigned int points)
{
    m_points = points;
    updateScaleWidgets();
}

//****************************************************************************
void SonagramWindow::setRate(unsigned int rate)
{
    m_rate = rate;
    updateScaleWidgets();
}

//****************************************************************************
//****************************************************************************
