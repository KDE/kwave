
#include "config.h"
#include <math.h>
#include <limits.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <kapp.h>
#include <kmsgbox.h>

#include <libkwave/Signal.h>
#include <libkwave/WindowFunction.h>

#include "libgui/OverViewWidget.h"
#include "libgui/ScaleWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "../../../src/KwaveApp.h"
#include "../../../src/ProgressDialog.h"
#include "SonagramWindow.h"
#include "../../../src/ImageView.h"
#include "../../../src/SignalManager.h"
#include "SonagramContainer.h"
#include "../../../src/TopWidget.h"

//#include <qtimer.h>
//#include <qpushbt.h>
//#include <qstring.h>
//#include <qpainter.h>
//#include <qpixmap.h>
//#include <kmenubar.h>
//#include <kstatusbar.h>

//extern KApplication *app;
//extern char *mstotimec (int ms);

//****************************************************************************
SonagramWindow::SonagramWindow(const QString &name)
:KTMainWindow()
{
    debug("SonagramWindow::SonagramWindow(): start"); // ###
    corner = 0;
    data = 0;
    image = 0;
    length = 0;
    mainwidget = 0;
    max = 0;
    overview = 0;
    points = 0;
    rate = 0;
    status = 0;
    view = 0;
    x = 0;
    xscale = 0;
    y = 0;
    yscale = 0;

    KMenuBar *bar = new KMenuBar(this);
    ASSERT(bar);
    if (!bar) return ;

    QPopupMenu *spectral = new QPopupMenu();
    ASSERT(spectral);
    if (!spectral) return ;

    QPopupMenu *file = new QPopupMenu ();
    ASSERT(file);
    if (!file) return ;

    bar->insertItem(i18n("&File"), file);
    bar->insertItem(i18n("&Spectral Data"), spectral);

    file->insertItem(i18n("&Import from Bitmap ..."), this, SLOT(load()));
    file->insertItem(i18n("&Export to Bitmap ..."), this, SLOT(save()));
    spectral->insertItem (i18n("&reTransform to signal"), this, SLOT(toSignal()));

    status = new KStatusBar (this, i18n("Frequencies Status Bar"));
    ASSERT(status);
    if (!status) return ;

    status->insertItem(i18n("Time:          0 ms     "), 1);
    status->insertItem(i18n("Frequency:          0 Hz     "), 2);
    status->insertItem(i18n("Amplitude:    0 %      "), 3);
    status->insertItem(i18n("Phase:    0        "), 4);

    mainwidget = new SonagramContainer(this);
    ASSERT(mainwidget);
    if (!mainwidget) return ;

    view = new ImageView(mainwidget);
    ASSERT(view);
    if (!view) return ;

    connect(view, SIGNAL(info (double, double)),
	    this, SLOT(setInfo(double, double)));
    xscale = new ScaleWidget (mainwidget, 0, 100, "ms");
    ASSERT(xscale);
    if (!xscale) return ;

    yscale = new ScaleWidget (mainwidget, 100, 0, "Hz");
    ASSERT(yscale);
    if (!yscale) return ;

    corner = new CornerPatchWidget (mainwidget);
    ASSERT(corner);
    if (!corner) return ;

    overview = new OverViewWidget (mainwidget);
    ASSERT(overview);
    if (!overview) return ;

    QObject::connect (overview, SIGNAL(valueChanged(int)),
		      view, SLOT(setOffset(int)));
    QObject::connect (view, SIGNAL(viewInfo(int, int, int)),
		      this, SLOT(setRange(int, int, int)));
    QObject::connect (view, SIGNAL(viewInfo(int, int, int)),
		      overview, SLOT(setRange(int, int, int)));

    mainwidget->setObjects (view, xscale, yscale, corner, overview);
    setView(mainwidget);
    setStatusBar(status);
    setMenu(bar);

    QString windowname(i18n("Sonagram of "));
    windowname += name;

    setCaption(windowname.data());
    resize (480, 300);
    debug("SonagramWindow::SonagramWindow(): end"); // ###
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

//****************************************************************************
void SonagramWindow::setSignal(double *input, int size, int points,
			       int windowtype, int rate)
{
//    ASSERT(points);
//    ASSERT(rate);
//    if (!points) return ;
//    if (!rate) return ;
//
//    double rea, ima;
//
//    debug("SonagramWindow::setSignal: size %d points %d windowtype %d",
//	size,points,windowtype);
//
//    this->length = size;
//    this->x = (size / points);
//    this->points = points;
//    this->rate = rate;
//
//    yscale ->setMaxMin (0, rate / 2);
//    xscale ->setMaxMin ((int)(((double)size) / rate*1000), 0);
//
//    data = new complex *[x];
//    ASSERT(data);
//
//    WindowFunction func (windowtype);
//    image = new QImage (x, points / 2, 8, 256);
//    ASSERT(image);
//
//    double* windowfunction = func.getFunction(points);
//    ASSERT(windowfunction);
//
//    if ((data) && (image) && windowfunction) {
//	char buf[256];
//	snprintf(buf, sizeof(buf),
//		 "doing %d %d-point mixed radix fft\'s\n", x, points);
//	ProgressDialog *dialog = new ProgressDialog (x, buf);
//
//	if (dialog) {
//	    dialog->show();
//
//	    gsl_fft_complex_wavetable table;
//
//	    gsl_fft_complex_wavetable_alloc (points, &table);
//	    gsl_fft_complex_init (points, &table);
//
//	    for (int i = 0; i < x; i++) {
//		complex *output = new complex[points];
//		if (output) {
//		    for (int j = 0; j < points; j++) {
//			output[j].real = windowfunction[j] * input[i * points + j];
//			//copy data into complex array
//			output[j].imag = 0;
//		    }
//
//		    gsl_fft_complex_forward (output, points, &table);
//
//		    for (int k = 0; k < points; k++) {
//			rea = output[k].real;
//			ima = output[k].imag;
//			rea = sqrt(rea * rea + ima * ima);
//			//get amplitude
//			if (max < rea) max = rea;
//			//and set maximum for display..
//		    }
//
//		    dialog->setProgress (i);
//		}
//		data[i] = output;
//		//put single spectrum into array of spectra
//	    }
//
//
//	    gsl_fft_complex_wavetable_free (&table);
//	}
//	delete dialog;
//
//	createPalette ();
//	createImage ();
//	view->setImage (image);
//    } else {
//	KMsgBox::message (this, "Info", "Out of memory !", 2);
//	if (data) delete[] data;
//	if (image) delete image;
//    }
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
void SonagramWindow::createPalette()
{
//    ASSERT(image);
//    if (!image) return ;
//
//    for (int i = 0; i < 256; i++)
//	image->setColor(i, qRgb(i, i, i) );  //create grayscale palette
}

//****************************************************************************

void SonagramWindow::createImage()
{
//    ASSERT(image);
//    ASSERT(data);
//
//    double rea, ima;
//    if (image && data) {
//	for (int i = 0; i < x; i++)
//	    if (data[i])
//		for (int j = 0; j < points / 2; j++) {
//		    rea = data[i][j].real;
//		    ima = data[i][j].imag;
//		    rea = sqrt(rea * rea + ima * ima) / max;
//		    //get amplitude and scale to 1
//		    rea = 1 - ((1 - rea) * (1 - rea));
//		    *(image->scanLine((points / 2 - 1)-j) + i) = 255-(int)(rea * 255);
//		}
//    }
}

//****************************************************************************
SonagramWindow::~SonagramWindow()
{
    debug("SonagramWindow::~SonagramWindow()"); // ###

//    if (data) {
//	for (int i = 0; i < x; i++)
//	    if (data[i]) delete data[i];
//	delete[] data;
//    }
//    if (image) delete image;
}

//****************************************************************************
void SonagramWindow::setInfo(double x, double y)
{
//    ASSERT(data);
//    ASSERT(status);
//    ASSERT(view);
//    if (!data) return ;
//    if (!status) return ;
//    if (!view) return ;
//
//    char buf[128];
//    int col;
//
//    debug("SonagramWindow::setInfo(%3.3f,%3.3f)",x,y); // ###
//
//    if (view->getWidth() > this->x)
//	col = (int)(x * (this->x - 1));
//    else
//	col = (int)(view->getOffset() + x * view->getWidth());
//
//    snprintf(buf, sizeof(buf), i18n("Time: %s"),
//	     mstotimec ((int)(((double) col)*points*10000 / rate)));
//    status->changeItem (buf, 1);
//    snprintf(buf, sizeof(buf), i18n("Frequency: %d Hz"),
//	     (int)(y*rate / 2));
//    status->changeItem (buf, 2);
//    if (data [(int)(x*this->x)]) {
//	double rea = data [col][(int)(y * points / 2)].real;
//	double ima = data [col][(int)(y * points / 2)].imag;
//
//	snprintf(buf, sizeof(buf), i18n("Amplitude: %d %%"),
//		 (int)(sqrt(rea*rea + ima*ima) / max*100));
//    } else snprintf(buf, sizeof(buf), i18n("Memory Leak !"));
//    status->changeItem (buf, 3);
//    if (data [(int)(x*this->x)]) {
//	double rea = data [col][(int)(y * points / 2)].real;
//	double ima = data [col][(int)(y * points / 2)].imag;
//	snprintf(buf, sizeof(buf), i18n("Phase: %d degree"),
//		 (int) (atan(ima / rea)*360 / M_PI));
//    } else snprintf(buf, sizeof(buf), i18n("Memory Leak !"));
//
//    status->changeItem (buf, 4);
}

//****************************************************************************
void SonagramWindow::setRange (int offset, int width, int)
{
//    ASSERT(rate);
//    if (rate) return ;
//
//    xscale->setMaxMin ((int)(((double)offset + width)*points*1000 / rate),
//		       (int)(((double)offset)*points*1000 / rate));
}

//****************************************************************************
//****************************************************************************
