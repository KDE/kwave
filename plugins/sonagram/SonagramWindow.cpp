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
#include <new>

#include <QtGui/QBitmap>
#include <QtGui/QImage>
#include <QtGui/QLayout>
#include <QtCore/QTimer>

#include <kmenubar.h>
#include <kfiledialog.h>
#include <kstatusbar.h>

#include "libkwave/WindowFunction.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"
#include "libgui/ImageView.h"
#include "libgui/ScaleWidget.h"

#include "SonagramWindow.h"

/**
 * delay between two screen updates [ms]
 */
#define REFRESH_DELAY 100

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
Kwave::SonagramWindow::SonagramWindow(const QString &name)
    :KMainWindow(), m_image(), m_color_mode(0), m_view(0), m_overview(0),
     m_points(0), m_rate(0), m_xscale(0), m_yscale(0), m_refresh_timer()
{
    KIconLoader icon_loader;

    for (int i = 0; i < 256; ++i) { m_histogram[i] = 0; }

    QWidget *mainwidget = new(std::nothrow) QWidget(this);
    Q_ASSERT(mainwidget);
    if (!mainwidget) return;
    setCentralWidget(mainwidget);

    QGridLayout *top_layout = new(std::nothrow) QGridLayout(mainwidget/*, 3, 2*/);
    Q_ASSERT(top_layout);
    if (!top_layout) return;

    KMenuBar *bar = menuBar();
    Q_ASSERT(bar);
    if (!bar) return ;

//    QMenu *spectral = new QMenu();
//    Q_ASSERT(spectral);
//    if (!spectral) return ;

    QMenu *file = bar->addMenu(i18n("&Sonagram"));
    Q_ASSERT(file);
    if (!file) return ;

//    bar->addAction(i18n("&Spectral Data"), spectral);
//    file->addAction(i18n("&Import from Bitmap..."), this, SLOT(load()));

    file->addAction(
	icon_loader.loadIcon(_("document-export"), KIconLoader::Small),
	i18n("&Export to Bitmap..."),
	this, SLOT(save())
    );
    file->addAction(
	icon_loader.loadIcon(_("dialog-close"), KIconLoader::Small),
	i18n("&Close"),
	this, SLOT(close()),
	QKeySequence::Close
    );

//    spectral->addAction (i18n("&Retransform to Signal"), this, SLOT(toSignal()));

    KStatusBar *status = statusBar();
    Q_ASSERT(status);
    if (!status) return ;

    status->insertItem(i18n("Time: ------ ms"), 1);
    status->insertItem(i18n("Frequency: ------ Hz"), 2);
    status->insertItem(i18n("Amplitude: --- %"), 3);

    m_view = new(std::nothrow) Kwave::ImageView(mainwidget);
    Q_ASSERT(m_view);
    if (!m_view) return;
    top_layout->addWidget(m_view, 0, 1);
    QPalette palette;
    palette.setBrush(m_view->backgroundRole(), QBrush(QImage(background)));
    m_view->setAutoFillBackground(true);
    m_view->setPalette(palette);

    m_xscale = new(std::nothrow)
	Kwave::ScaleWidget(mainwidget, 0, 100, i18n("ms"));
    Q_ASSERT(m_xscale);
    if (!m_xscale) return;
    m_xscale->setFixedHeight(m_xscale->sizeHint().height());
    top_layout->addWidget(m_xscale, 1, 1);

    m_yscale = new(std::nothrow)
	Kwave::ScaleWidget(mainwidget, 0, 100, i18n("Hz"));
    Q_ASSERT(m_yscale);
    if (!m_yscale) return ;
    m_yscale->setFixedWidth(m_yscale->sizeHint().width());
    m_yscale->setMinimumHeight(9*6*5);
    top_layout->addWidget(m_yscale, 0, 0);

    m_overview = new(std::nothrow) Kwave::ImageView(mainwidget);
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
    top_layout->setColumnStretch(0, 0);
    top_layout->setColumnStretch(1, 100);
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
void Kwave::SonagramWindow::close()
{
    QWidget::close();
}

//****************************************************************************
void Kwave::SonagramWindow::save()
{
    if (m_image.isNull()) return;

    Kwave::FileDialog dlg(_("kfiledialog:///kwave_sonagram"), QString(),
        this, true, QString(), _("*.bmp"));
    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setCaption(i18n("Save Sonagram"));
    if (dlg.exec() != QDialog::Accepted) return;
    QString filename = dlg.selectedFile();

    if (!filename.isEmpty()) m_image.save(filename, "BMP");
}

//****************************************************************************
void Kwave::SonagramWindow::load()
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
void Kwave::SonagramWindow::setImage(QImage image)
{
    Q_ASSERT(m_view);
    if (!m_view) return;

    m_image = image;

    // re-initialize histogram over all pixels
    for (unsigned int i = 0; i < 256; i++)
	m_histogram[i] = 0;
    if (!m_image.isNull()) {
	for (int x = 0; x < m_image.width(); x++) {
	    for (int y = 0; y < m_image.height(); y++) {
		quint8 p = static_cast<quint8>(m_image.pixelIndex(x, y));
		m_histogram[p]++;
	    }
	}
    }

    refresh_view();
}

//****************************************************************************
void Kwave::SonagramWindow::setOverView(const QImage &overview)
{
    if (m_overview) m_overview->setImage(overview);
}

//****************************************************************************
void Kwave::SonagramWindow::insertSlice(const unsigned int slice_nr,
                                        const QByteArray &slice)
{
    Q_ASSERT(m_view);
    if (!m_view) return;
    if (m_image.isNull()) return;

    unsigned int image_width  = m_image.width();
    unsigned int image_height = m_image.height();

    // slice is out of range ?
    if (slice_nr >= image_width) return;

    unsigned int y;
    unsigned int size = slice.size();
    for (y = 0; y < size; y++) {
	quint8 p;

	// remove the current pixel from the histogram
	p = static_cast<quint8>(m_image.pixelIndex(slice_nr, y));
	m_histogram[p]--;

	// set the new pixel value
	p = slice[(size - 1) - y];
	m_image.setPixel(slice_nr, y, p);

	// insert the new pixel into the histogram
	m_histogram[p]++;
    }
    while (y < image_height) { // fill the rest with blank
	m_image.setPixel(slice_nr, y++, 0xFE);
	m_histogram[0xFE]++;
    }

    if (!m_refresh_timer.isActive()) {
	m_refresh_timer.setSingleShot(true);
	m_refresh_timer.start(REFRESH_DELAY);
    }
}

//****************************************************************************
void Kwave::SonagramWindow::adjustBrightness()
{
    if (m_image.isNull()) return;

    // get the sum of pixels != 0
    unsigned long int sum = 0;
    for (unsigned int i = 1; i <= 254; i++)
	sum += m_histogram[i];

    // cut off all parts below the cutoff ratio (e.g. 0.1%)
    unsigned int cutoff = Kwave::toUint(sum * COLOR_CUTOFF_RATIO);

    // get the last used color from the histogram
    int last = 254;
    while ((last >= 0) && (m_histogram[last] <= cutoff))
	last--;

    QColor c;
    for (int i = 0; i < 255; i++) {
	int v;

	if (i >= last) {
	    v = 254;
	} else {
	    // map [0...last] to [254...0]
	    v = ((last - i) * 254) / last;
	}

	if (m_color_mode == 1) {
	    // rainbow effect
	    c.setHsv( (v * 255) / 255, 255, 255, 255);
	} else {
	    // greyscale palette
	    c.setRgb(v, v, v, 255);
	}

	m_image.setColor(i, c.rgba());
// 	qDebug("color[%3d] = 0x%08X",i, c.rgba());
    }

    // use color 0xFF for transparency !
    m_image.setColor(0xFF, QColor(0, 0, 0, 0).rgba());
}

//****************************************************************************
void Kwave::SonagramWindow::refresh_view()
{
    Q_ASSERT(m_view);
    if (!m_view) return;
    adjustBrightness();
    m_view->setImage(m_image);
}

//****************************************************************************
void Kwave::SonagramWindow::toSignal()
{
/** @todo needs to be ported to fftw and re-activated */
//    gsl_fft_complex_wavetable table;
//
//    gsl_fft_complex_wavetable_alloc (points, &table);
//    gsl_fft_complex_init (points, &table);
//
//    Kwave::TopWidget *win = new Kwave::TopWidget(...);
//
//    Q_ASSERT(win);
//    if (win) {
//
//	Kwave::Signal *newsig = new Kwave::Signal(length, rate);
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
void Kwave::SonagramWindow::translatePixels2TF(const QPoint p,
                                               double *ms, double *f)
{
    if (ms) {
	// get the time coordinate [0...(N_samples-1)* (1/f_sample) ]
	if (m_rate != 0) {
	    *ms = static_cast<double>(p.x()) *
	          static_cast<double>(m_points) * 1000.0 / m_rate;
	} else {
	    *ms = 0;
	}
    }

    if (f) {
	// get the frequency coordinate
	double py = (m_points >= 2) ? (m_points / 2) - 1 : 0;
	double y = py - p.y();
	if (y < 0) y = 0;
	*f = y / py * (m_rate / 2.0);
    }
}

//***************************************************************************
void Kwave::SonagramWindow::updateScaleWidgets()
{
    double ms;
    double f;

    translatePixels2TF(QPoint(m_image.width() - 1, 0), &ms, &f);

    m_xscale->setMinMax(0, Kwave::toInt(rint(ms)));
    m_yscale->setMinMax(0, Kwave::toInt(rint(f)));
}

//***************************************************************************
Kwave::SonagramWindow::~SonagramWindow()
{
}

//***************************************************************************
void Kwave::SonagramWindow::setColorMode(int mode)
{
    Q_ASSERT(mode >= 0);
    Q_ASSERT(mode <= 1);

    if (mode != m_color_mode) {
	m_color_mode = mode;
	setImage(m_image);
    }
}

//***************************************************************************
void Kwave::SonagramWindow::setName(const QString &name)
{
    setCaption((name.length()) ?
	i18n("Sonagram of %1", name) :
	i18n("Sonagram")
    );
}

//****************************************************************************
void Kwave::SonagramWindow::cursorPosChanged(const QPoint pos)
{
    KStatusBar *status = statusBar();
    Q_ASSERT(status);
    Q_ASSERT(m_points);
    Q_ASSERT(m_rate != 0);
    if (!status) return;
    if (m_image.isNull()) return;
    if (!m_points) return;
    if (m_rate == 0) return;

    double ms;
    double f;
    double a;
    translatePixels2TF(pos, &ms, &f);

    // item 1: time in milliseconds
    status->changeItem(i18n("Time: %1", Kwave::ms2string(ms)), 1);

    // item 2: frequency in Hz
    QString text = i18n("Frequency: %1 Hz", Kwave::toInt(f));
    status->changeItem(text, 2);

    // item 3: amplitude in %
    if (m_image.valid(pos.x(), pos.y())) {
	a = m_image.pixelIndex(pos.x(), pos.y()) * (100.0 / 254.0);
    } else {
	a = 0.0;
    }
    text = i18n("Amplitude: %1%", Kwave::toInt(a));
    status->changeItem(text, 3);
}

//****************************************************************************
void Kwave::SonagramWindow::setPoints(unsigned int points)
{
    m_points = points;
    updateScaleWidgets();
}

//****************************************************************************
void Kwave::SonagramWindow::setRate(double rate)
{
    m_rate = rate;
    updateScaleWidgets();
}

//***************************************************************************
#include "SonagramWindow.moc"
//***************************************************************************
//***************************************************************************
