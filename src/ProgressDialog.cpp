#include <stdio.h>

#include <qpainter.h>
#include <qtimer.h>

#include <kapp.h>

#include <libkwave/TimeOperation.h>
#include "ProgressDialog.h"

//uncomment this to get a rather fancy Progress indicator, which shows that I've to
//much free time
//#define FANCY

#ifdef FANCY
#define M_PI 3.1415926

#include <qpixmap.h>

#endif

//**********************************************************
//KProgresss does flicker a lot, has bugs, that appear when using high
//numeric values, so here follows my own implementation...
//**********************************************************
ProgressDialog::ProgressDialog(TimeOperation *operation, const char *caption)
    :QDialog(0, caption)
//timerbased variant, peeks at address "counter" to gain progress information
{
    act = 0;
    last = 0;
    lastx = 0;
    lasty = 0;
    max = (operation) ? operation->getLength() : 0;
    oldw = 0;
    this->operation = operation;
    timer = 0;

    ASSERT(operation);
    ASSERT(caption);

#ifdef FANCY
    setMinimumSize (128, 128);
    setMaximumSize (128, 128);
    setBackgroundColor (black);
#else
    resize (320, 20);
#endif

    setCaption (caption);

    timer = new QTimer (this);
    ASSERT(timer);
    if (timer) {
	connect( timer, SIGNAL(timeout()), SLOT(timedProgress()));
	timer->start( 200);    //5 times per second should be enough
    }
}

//**********************************************************
//this one needs calls to get updated
ProgressDialog::ProgressDialog(int max, char *caption)
    :QDialog(0, (caption) ? caption : i18n("Progress"))
{
    act = 0;
    last = 0;
    lastx = 0;
    lasty = 0;
    this->max = max;
    oldw = 0;
    operation = 0;
    timer = 0;

#ifdef FANCY
    setMinimumSize (128, 128);
    setMaximumSize (128, 128);
    setBackgroundColor (black);
#else
    resize (320, 20);
#endif

    setCaption (caption);
    setCursor (waitCursor);
}

//**********************************************************
void ProgressDialog::timedProgress ()
{
    if (operation) {
	int prog = operation->getCounter();

	if (prog < 0) {
	    //signal for ending...
	    delete this;
	} else {
	    setProgress (prog);
	}
    }
}

//**********************************************************
void ProgressDialog::setProgress(int x)
{
    act = x;
    repaint(false);
}

//**********************************************************
void ProgressDialog::paintEvent(QPaintEvent *)
{
    ASSERT(max);
    if (!max) return;

    QPainter p;
    int perc = (int)(((double)act) * 100 / max);

#ifdef FANCY
    int width = 128;
    int height = 128;

    p.begin (this);
    int col = (int)((sin(60 * M_PI * (double)act / max) + 1) * 64);
    p.setPen (QPen(QColor(col, col, col), width / 16));
    //  p.setRasterOp (XorROP);
    p.translate (width / 2, height / 2);

    int newx = (int)(sin(64 * M_PI * (double)act / max) * width / sqrt(2) * act / max);
    int newy = (int)(cos(64 * M_PI * (double)act / max) * height / sqrt(2) * act / max);

    p.drawLine (lastx, lasty, newx, newy);

    if (perc > last) {
	char buf[256];
	snprintf(buf, sizeof(buf), "%d %%", perc);
	last = perc;

	p.setRasterOp (CopyROP);

	QFont newfont(font());

	newfont.setPointSize (width / 4);
	newfont.setBold (true);

	p.setPen(black);
	p.setFont(newfont);
	p.drawText(-width / 2 + 3, -height / 2, width, height,
	           AlignCenter, buf, 5);
	p.setFont(newfont);
	p.drawText(-width / 2 - 3, -height / 2, width, height,
	           AlignCenter, buf, 5);
	p.setFont(newfont);
	p.drawText(-width / 2, -height / 2 + 3, width, height,
	           AlignCenter, buf, 5);
	p.setFont(newfont);
	p.drawText(-width / 2, -height / 2 - 3, width, height,
	           AlignCenter, buf, 5);

	newfont.setPointSize (width / 4);
	newfont.setBold (true);

	p.setPen(white);
	p.setFont(newfont);
	p.drawText(-width / 2, -height / 2, width, height,
	           AlignCenter, buf, 5);
    }

    lastx = newx;
    lasty = newy;
    p.end();
#else
    int col = (int)((((double)act) / max) * (512 + 64));
    int w = (int)((((double)act) / max) * (width() - 2));
    if (w != oldw) {
	QPixmap map (width(), height());
	map.fill (colorGroup().background());
	char buf[128];
	snprintf(buf, sizeof(buf), "%d %%", perc);

	p.begin (&map);
	if (col < 256) {
	    p.setPen (QColor(255, col, 0));
	    p.setBrush (QColor(255, col, 0));
	} else
	    if (col < 512) {
		int x = 511-col;
		p.setPen (QColor(x, 255, 0));
		p.setBrush (QColor(x, 255, 0));
	    } else
		if (col < 768) {
		    int x = 767-col;
		    p.setPen (QColor(0, x, 0));
		    p.setBrush (QColor(0, x, 0));
		} else {
		    p.setPen (QColor(0, 0, 0));
		    p.setBrush (QColor(0, 0, 0));
		}

	p.drawRect (1, 1, w, height() - 2);
	p.setPen (colorGroup().light());
	p.drawLine (0, height() - 1, width(), height() - 1);
	p.drawLine (width() - 1, 0, width() - 1, height() - 2);
	p.setPen (colorGroup().dark());
	p.drawLine (0, 0, width(), 0);
	p.drawLine (0, 0, 0, height() - 1);
	p.end ();

	bitBlt (this, 0, 0, &map);

	p.begin (this);
	p.setPen (colorGroup().text());

	p.drawText (0, 0, width(), height(), AlignCenter, buf, 5);
	p.end ();
	w = oldw;
    }
#endif
}

//**********************************************************
ProgressDialog::~ProgressDialog()
{
    setCursor(arrowCursor);

    if (timer) {
	timer->stop();
	delete timer;
    }
    if (operation) delete operation;

    // inform our owner that we are ready
    // ### emit commandDone();
}
