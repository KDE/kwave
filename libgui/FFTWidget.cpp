#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <qpixmap.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include <libkwave/Interpolation.h>
#include <libkwave/Curve.h>
#include <libkwave/Signal.h>
#include "Dialog.h"
#include "FFTWidget.h"

__inline void getMaxMinPower (complex *sample, int len, double *max, double *min) {
    double rea, ima;
    double c;
    *min = INT_MAX;
    *max = INT_MIN;
    for (int i = 0; i < len; i++) {
	rea = sample[i].real;
	ima = sample[i].imag;
	c = sqrt(rea * rea + ima * ima);
	if (c > *max) *max = c;
	if (c < *min) *min = c;
    }
}
//****************************************************************************
__inline void getMaxMinPhase (complex *sample, int len, double &max, double &min) {
    double rea, ima;
    double c;
    min = INT_MAX;
    max = INT_MIN;
    for (int i = 0; i < len; i++) {
	rea = sample[i].real;
	ima = sample[i].imag;
	c = (atan(ima / rea) + M_PI / 2) / M_PI;
	if (c > max) max = c;
	if (c < min) min = c;
    }
}
//****************************************************************************
__inline void getLocalMax (complex *sample, int upperLimit, int &nr)
//also contributed by Gerhard Zintel
{
    double rea, ima;
    double c, max;
    int direction = 1;
    rea = sample[nr].real;
    ima = sample[nr++].imag;
    max = rea * rea + ima * ima;
    rea = sample[nr].real;
    ima = sample[nr].imag;
    c = rea * rea + ima * ima;
    if (c < max) {
	direction = -1;
	c = max;
	nr--;
    }
    do {
	max = c;
	nr += direction;
	rea = sample[nr].real;
	ima = sample[nr].imag;
	c = rea * rea + ima * ima;
    } while (c > max && nr > 0 && nr < upperLimit);
    nr -= direction;
}
//****************************************************************************
FFTWidget::FFTWidget(QWidget *parent)
    :QWidget(parent)
{
    autodelete = true;
    cursor = -1;
    data = 0;
    db = false;
    fftsize = 0;
    findLocalMax = true;
    height = -1;
    lmarker = 0;
    max = 0;
    min = 0;
    oldcursor = -1;
    phaseview = false;
    pixmap = 0;
    rate = 0;
    redraw = false;
    redrawcursor = false;
    rmarker = 0;
    width = 0;
    zoom = 0;

    setCursor (crossCursor);
    setBackgroundColor (QColor(black) );
}

//****************************************************************************
FFTWidget::~FFTWidget()
{
    if (pixmap == 0) delete pixmap;
    if (data && autodelete) delete data;
}

//****************************************************************************
void FFTWidget::setAutoDelete(int tr)
{
    autodelete = tr;
}

//****************************************************************************
void FFTWidget::setFreqRange(int min, int max)
{
}

//****************************************************************************
void FFTWidget::togglefindPeak(bool *send)
{
    ASSERT(send);
    findLocalMax = !findLocalMax;
    if (send) *send = findLocalMax;
}

//****************************************************************************
void FFTWidget::findMaxPeak()
{
    double max = 0;
    double rea, ima, c;

    oldcursor = cursor;
    for (int i = 0; i < fftsize / 2; i++) {
	rea = data[i].real;
	ima = data[i].imag;
	c = sqrt(rea * rea + ima * ima);
	if (c > max) max = c, cursor = i;    //set new maximum, set cursor
    }

    redrawcursor = true;
    repaint (false);
}

//****************************************************************************
void FFTWidget::findMinimum()
{
    double min = INT_MAX;
    double rea, ima, c;

    oldcursor = cursor;
    for (int i = 0; i < fftsize / 2; i++) {
	rea = data[i].real;
	ima = data[i].imag;
	c = sqrt(rea * rea + ima * ima);
	if (c < min) min = c, cursor = i;    //set new maximum, set cursor
    }

    redrawcursor = true;
    repaint(false);
}

//****************************************************************************
void FFTWidget::phaseMode()
{
    redraw = true;
    phaseview = true;
    repaint();
}

//****************************************************************************
void FFTWidget::dbMode(int db)
{
    redraw = true;
    phaseview = false;
    this->db = db;
    repaint();
}

//****************************************************************************
void FFTWidget::percentMode()
{
    redraw = true;
    phaseview = false;
    db = false;
    repaint();
}

//****************************************************************************
void FFTWidget::setPhase(complex *data, int size, int rate)
{
    phaseview = true;
    this->data = data;
    this->fftsize = size;
    this->rate = rate;
}

//****************************************************************************
void FFTWidget::getMaxMin()
{
    getMaxMinPower (data, fftsize / 2, &this->max, &this->min);
}

//****************************************************************************
void FFTWidget::setSignal(complex *data, int size, int rate)
{
    phaseview = false;
    this->data = data;
    this->fftsize = size;
    this->rate = rate;
    getMaxMin();
}

//****************************************************************************
void FFTWidget::refresh()
{
    getMaxMin();
    redraw = true;
    repaint();
}

//****************************************************************************
void FFTWidget::formant()
{
    ASSERT(rate != 0);
    if (rate == 0) return;

    Dialog *dialog =
	DynamicLoader::getDialog("formant", new DialogOperation(rate));

    if ((dialog) && (dialog->exec())) {
	int i;
	double mul = 0;
	int size = 5000 * (fftsize) / (rate);
	//formant spectrum is limited to 5khz
	//      double *points=dialog->getPoints (size);
	double *points = 0;

	if (points) {
	    for (i = 0; i < size; i++) {
		//reconvert db scale to linear multiplication factor
		mul = 1 / (pow (2, (points[i] / 6)));

		data[i].real *= mul;
		data[i].imag *= mul;
		data[fftsize - i].real *= mul;
		data[fftsize - i].imag *= mul;
	    }
	    for (; i < fftsize / 2; i++) {
		data[i].real *= mul;
		data[i].imag *= mul;
		data[fftsize - i].real *= mul;
		data[fftsize - i].imag *= mul;
	    }
	    delete points;
	}

	//      delete dialog;
	refresh ();
    }
}

//****************************************************************************
void FFTWidget::smooth()
{
    Dialog *dialog =
	DynamicLoader::getDialog ("movingaverage", new DialogOperation(rate));

    if ((dialog) && (dialog->exec())) {
	int average = atoi(dialog->getCommand());
	int b = average / 2;
	double rea, ima, abs, old;
	int i, j;

	complex *newdata = new complex [fftsize];
	ASSERT(newdata);
	if (newdata) {
	    for (i = 0; i < fftsize; i++) {
		abs = 0;

		if ((i - b < 0) || (i + b >= fftsize))
		    for (j = -b; j < b; j++) {
			if ((i + b >= 0) && (i + j < fftsize)) {
			    rea = data[i + j].real;
			    ima = data[i + j].imag;
			    abs += sqrt(rea * rea + ima * ima);
			}
		    }
		else
		    for (j = -b; j < b; j++) {
			rea = data[i + j].real;
			ima = data[i + j].imag;
			abs += sqrt(rea * rea + ima * ima);
		    }

		abs /= average;
		rea = data[i].real;
		ima = data[i].imag;
		old = sqrt (rea * rea + ima * ima);

		abs = abs / old;

		newdata[i].real = data[i].real * abs;
		newdata[i].imag = data[i].imag * abs;
	    }
	    delete data;
	    data = newdata;

	    refresh ();
	}
    }
}
//****************************************************************************
void FFTWidget::amplify()
{
    Dialog *dialog =
	DynamicLoader::getDialog ("movingaverage", new DialogOperation(rate));

    if ((dialog) && (dialog->exec())) {
	Parser parser(dialog->getCommand());
	Interpolation interpolation;

	Curve *points = new Curve(parser.getFirstParam());

	double *y = interpolation.getInterpolation (points, fftsize / 2);

	for (int i = 0; i < fftsize / 2; i++) {
	    data[i].real *= 2 * y[i];
	    data[i].imag *= 2 * y[i];
	    data[fftsize - i].real *= 2 * y[i];
	    data[fftsize - i].imag *= 2 * y[i];
	}

	refresh ();

	delete dialog;
    }
}

//****************************************************************************
void FFTWidget::killPhase()
{
    double rea, ima;
    for (int i = 0; i < fftsize; i++) {
	rea = data[i].real;
	ima = data[i].imag;
	data[i].real = sqrt(rea * rea + ima * ima);
	data[i].imag = 0;
    }

    redraw = true;
    repaint();
}

//****************************************************************************
void FFTWidget::iFFT()
{
    complex *data = new complex [fftsize];
    ASSERT(data);
    if (data) {
	for (int i = 0; i < fftsize; i++) data[i] = this->data[i];

	gsl_fft_complex_wavetable table;

	gsl_fft_complex_wavetable_alloc (fftsize, &table);

	gsl_fft_complex_init (fftsize, &table);

	gsl_fft_complex_inverse (data, fftsize, &table);
	gsl_fft_complex_wavetable_free (&table);

	//      TopWidget *win=new TopWidget ();

	if (0 == 1) {
	    Signal *newsig = new Signal (fftsize, rate);

	    //    win->show();
	    if (newsig) {
		int *sam = newsig->getSample();
		if (sam) {
		    for (int i = 0; i < fftsize; i++)
			sam[i] = (int)(data[i].real * ((1 << 23)-1));
		}
		//            win->setSignal (new SignalManager(newsig));
	    }
	}
	delete data;
    }
}

//****************************************************************************
void FFTWidget::mousePressEvent( QMouseEvent *e)
{
    mouseMoveEvent (e);
}

//****************************************************************************
void FFTWidget::mouseReleaseEvent( QMouseEvent *)
{
}

//****************************************************************************
void FFTWidget::mouseMoveEvent( QMouseEvent *e )
{
    ASSERT(e);
    ASSERT(height);
    if (!e) return;
    if (!height) return;

    int x = e->pos().x();
    if ((x < width) && (x >= 0)) {
	int y = e->pos().y();

	if (db)
	    emit dbInfo ((height - y)*db / (height), 0);
	else
	    if ((y >= 0) && (y < height))
		emit ampInfo ((height - y)*100 / (height), (int) floor(100 / (double)height + .5));

	if (cursor != x) //update cursor
	{
	    oldcursor = cursor;
	    cursor = (((long int) x) * fftsize / (2 * width));
	    redrawcursor = true;
	    repaint (false);
	}
    }
}

//****************************************************************************
void FFTWidget::drawOverviewPhase()
{
    if (fftsize) {
	int step;
	double max = 0, min = 0;

	p.setPen (white);

	for (int i = 0; i < width; i++) {
	    step = (int) (zoom * i);
	    getMaxMinPhase (&data[step], (int)((zoom) + 1), max, min);
	    max = (max) * height;
	    min = (min) * height;
	    p.drawLine (i, -(int)max, i, -(int)min);
	}
    }
}

//****************************************************************************
void FFTWidget::drawOverviewFFT()
{
    if (fftsize) {
	int step;
	double max = 0, min = 0;

	p.setPen (white);

	for (int i = 0; i < width; i++) {
	    step = (int) (zoom * i);
	    getMaxMinPower (&data[step], (int)((zoom) + 1), &max, &min);
	    max = (max) * height / this->max;
	    min = (min) * height / this->max;
	    p.drawLine (i, -(int)max, i, -(int)min);
	}
    }
}

//****************************************************************************
void FFTWidget::drawInterpolatedDB ()
{
    if (fftsize && db) {
	int lx, ly, y, x;
	double rea, ima;

	lx = 0;
	rea = data[0].real;
	ima = data[0].imag;
	rea = sqrt(rea * rea + ima * ima);

	rea /= max;
	if (rea != 0) rea = -6 * (1 / log10(2)) * log10(rea) / db;
	ly = (int)(rea - 1) * height;

	for (int i = 0; i < fftsize / 2; i++) {
	    x = (int) (i / zoom);

	    rea = data[i].real;
	    ima = data[i].imag;
	    rea = sqrt(rea * rea + ima * ima);
	    rea /= max;
	    if (rea != 0) rea = -6 * (1 / log10(2)) * log10(rea) / db;
	    else rea = 2;
	    y = (int)(rea - 1) * height;

	    p.drawLine (lx, ly, x, y);
	    lx = x;
	    ly = y;
	}
    }
}

//****************************************************************************
__inline double getDB (double max, double x, double db)
{
    x /= max;
    x = -6 * (1 / log10(2)) * log10(x) / db;
    x--;
    return x;
}

//****************************************************************************
void FFTWidget::drawOverviewDB()
{
    if (fftsize) {
	int step;
	double stepmax = 0, stepmin = 0;

	p.setPen (white);

	for (int i = 0; i < width; i++) {
	    step = (int) (zoom * i);
	    getMaxMinPower (&data[step], (int)((zoom) + 1), &stepmax, &stepmin);

	    if (stepmax != 0) stepmax = getDB (max, stepmax, db);
	    else stepmax = 1;
	    if (stepmin != 0) stepmin = getDB (max, stepmin, db);
	    else stepmin = 1;

	    stepmax = stepmax * height;
	    stepmin = stepmin * height;
	    p.drawLine (i, (int)stepmax, i, (int)stepmin);
	}
    }
}

//****************************************************************************
void FFTWidget::drawInterpolatedPhase()
{
    if (fftsize) {
	int lx, ly, y, x;
	double rea, ima;

	lx = 0;
	rea = data[0].real;
	ima = data[0].imag;
	rea = (atan(ima / rea) + M_PI / 2) / M_PI;
	ly = (int)(rea * height);

	for (int i = 0; i < fftsize / 2; i++) {
	    x = (int) (i / zoom);

	    rea = data[i].real;
	    ima = data[i].imag;
	    rea = (atan(ima / rea) + M_PI / 2) / M_PI;
	    y = (int)(rea * height);

	    p.drawLine (lx, -ly, x, -y);
	    lx = x;
	    ly = y;
	}
    }
}

//****************************************************************************
void FFTWidget::drawInterpolatedFFT()
{
    if (fftsize) {
	int lx, ly, y, x;
	double rea, ima;

	lx = 0;
	rea = data[0].real;
	ima = data[0].imag;
	rea = sqrt(rea * rea + ima * ima);
	ly = (int)((rea / this->max) * height);

	for (int i = 0; i < fftsize / 2; i++) {
	    x = (int) (i / zoom);

	    rea = data[i].real;
	    ima = data[i].imag;
	    rea = sqrt(rea * rea + ima * ima);
	    y = (int)((rea / this->max) * height);

	    p.drawLine (lx, -ly, x, -y);
	    lx = x;
	    ly = y;
	}
    }
}

//****************************************************************************
void FFTWidget::paintEvent (QPaintEvent *)
{
    ///if pixmap has to be resized or updated
    if ((rect().height() != height) || (rect().width() != width) || redraw) {
	redraw = false;
	height = rect().height();
	width = rect().width();

	if (pixmap) delete pixmap;
	pixmap = new QPixmap (size());
        ASSERT(pixmap);
        if (!pixmap) return;

	pixmap->fill (black);
	p.begin (pixmap);
	p.translate (0, height);

	p.setPen (white);
	if (fftsize != 0) {
	    zoom = ((double)fftsize) / (width * 2);

	    if (phaseview) {
		if (zoom > 1) drawOverviewPhase();
		else drawInterpolatedPhase();
	    } else {
		if (db) {
		    if (zoom > 1) drawOverviewDB();
		    else drawInterpolatedDB();
		} else {
		    if (zoom > 1) drawOverviewFFT();
		    else drawInterpolatedFFT();
		}
	    }

	}
	p.end();
	oldcursor = -1;
	redrawcursor = true;
    }

    if (redrawcursor && (cursor != -1) && fftsize) {
	int f = (int)((cursor * rate) / fftsize);

	if (findLocalMax) {
	    int nr = (int)((f * (fftsize - 1)) / rate);
	    getLocalMax (data, fftsize / 2, nr);
	    f = (nr * rate) / (fftsize - 1);
	    cursor = (int) ((((float)f) * fftsize) / rate + 0.5);
	}
	int x = (((long int)cursor) * width / (fftsize / 2));
	int oldx = (((long int)oldcursor) * width / (fftsize / 2));
	p.begin (pixmap);
	p.translate (0, height);
	p.setPen (green);
	p.setRasterOp (XorROP);

	if (oldcursor != -1) p.drawLine (oldx, 0, oldx, -height);
	p.drawLine (x, 0, x, -height);
	p.end();
	//      emit freqInfo ((cursor*rate)/fftsize,(rate/4)/width);
	//      emit noteInfo ((cursor*rate)/fftsize,0);
	emit freqInfo (f, (rate / 4) / width);
	emit noteInfo (f, 0);
	redrawcursor = false;
    }

    //blit pixmap to window
    if (pixmap) bitBlt (this, 0, 0, pixmap);
}
//****************************************************************************
