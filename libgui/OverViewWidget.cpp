
#include <qpainter.h>
// #include "../src/MainWidget.h" //this has to be changed sometimes
#include "OverViewWidget.h"

OverViewWidget::OverViewWidget(QWidget *parent, const char *name)
    :QWidget(parent, name)
{
    dir = 0;
//    mparent = 0;
    grabbed = false;
    height = 0;
    this->parent = parent;
    pixmap = 0;
    redraw = false;
    slider_length = 0;
    slider_pos = 0;
    slider_width = 0;
    timer = 0;
    width = 0;
}

////*****************************************************************************
//OverViewWidget::OverViewWidget(MainWidget *parent, const char *name)
//    :QWidget(parent, name)
//{
//    dir = 0;
//    mparent = parent;
//    grabbed = false;
//    height = 0;
//    this->parent = parent;
//    pixmap = 0;
//    redraw = false;
//    slider_length = 0;
//    slider_pos = 0;
//    slider_width = 0;
//    timer = 0;
//    width = 0;
//}

//*****************************************************************************
OverViewWidget::~OverViewWidget()
{
    if (pixmap) delete pixmap;
    if (timer) {
	timer->stop();
	delete timer;
    }
}

//*****************************************************************************
void OverViewWidget::mousePressEvent( QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    int x1 = (slider_length) ?
	(int)(((double)slider_pos) * width / slider_length) :
	0;
    int x2 = (slider_length) ?
	(int)(((double)slider_width) * width / slider_length) :
	0;

    if (x2 < 8) x2 = 8;
    if (e->x() > x1 + x2) {
	dir = slider_width / 2;
	if (timer) {
	    timer->stop();
	    delete timer;
	}
	timer = new QTimer (this);
	ASSERT(timer);
	if (!timer) return;
	
	connect (timer, SIGNAL(timeout()), this, SLOT(increase()));
	timer->start( 50);
    } else
	if (e->x() < x1) {
	    dir = -slider_width / 2;
	    timer = new QTimer (this);
	    connect (timer, SIGNAL(timeout()), this, SLOT(increase()));
	    timer->start( 50);
	} else grabbed = e->x() - x1;
}

//****************************************************************************
void OverViewWidget::increase()
{
    slider_pos += dir;
    if (slider_pos < 0) slider_pos = 0;
    if (slider_pos > slider_length - slider_width)
	slider_pos = slider_length - slider_width;
    repaint (false);
    emit valueChanged (slider_pos);
}

//****************************************************************************
void OverViewWidget::mouseReleaseEvent( QMouseEvent *)
{
    grabbed = false;
    if (timer) {
	timer->stop();
	delete timer;
	timer = 0;
    }
}

//****************************************************************************
void OverViewWidget::mouseMoveEvent( QMouseEvent *e)
{
    ASSERT(e);
    ASSERT(width);
    if (!e) return;
    if (!width) return;

    if (grabbed) {
	int pos = e->x() - grabbed;
	if (pos < 0) pos = 0;
	if (pos > width) pos = width;
	slider_pos = (int)(((double) slider_length) * pos / width);
	if (slider_pos > slider_length - slider_width)
	    slider_pos = slider_length - slider_width;
	repaint (false);
	emit valueChanged (slider_pos);
    }
}

//*****************************************************************************
void OverViewWidget::refresh()
{
    redraw = true;
    repaint (false);
}

//*****************************************************************************
void OverViewWidget::setRange(int new_pos, int new_width, int new_length)
{
    if ( (new_pos != slider_pos) ||
	 (slider_length != new_length) ||
	 (slider_width != new_width) ) {
	if ( (slider_length == new_length) && (slider_width == new_width) ) {
	    slider_pos = new_pos;
	    repaint(false);
	} else {
	    slider_pos = new_pos;
	    slider_width = new_width;
	    slider_length = new_length;
	    refresh();
	}
    }
}

//*****************************************************************************
void OverViewWidget::setValue(int newval)
{
    if (slider_pos != newval) {
	slider_pos = newval;
	repaint (false);
    }
}

//*****************************************************************************
void OverViewWidget::paintEvent (QPaintEvent *)
{
    QPainter p;
    ///if pixmap has to be resized ...
    if ((rect().height() != height) || (rect().width() != width) || redraw) {
	redraw = false;
	height = rect().height();
	width = rect().width();

	if (pixmap) delete pixmap;
	pixmap = new QPixmap (size());
        ASSERT(pixmap);
        if (!pixmap) return;

	pixmap->fill (colorGroup().background());

	p.begin (pixmap);
	p.setPen (colorGroup().midlight());

//	if (mparent) {
//	    unsigned char *overview = mparent->getOverView(width);
//	    if (overview) {
//		for (int i = 0; i < width; i++)
//		    p.drawLine(i, height - (((int)overview[i])*height) / 128,
//		               i, height);
//		delete overview;
//	    }
//	}

	p.end ();
    }
    if (pixmap) bitBlt (this, 0, 0, pixmap);

    p.begin (this);

    int x1 = (slider_length) ?
	(int)(((double)slider_pos) * width / slider_length) :
	0;
    int x2 = (slider_length) ?
	(int)(((double)slider_width) * width / slider_length) :
	0;
    if (x2 < 8) x2 = 8;      //so there is at least something

    p.setPen (colorGroup().light());
    p.drawLine (0, 0, width, 0);
    p.drawLine (0, 0, 0, height);

    p.drawLine (x1, 0, x1 + x2, 0);
    p.drawLine (x1, 0, x1, height);
    p.drawLine (x1 + 1, 0, x1 + 1, height);
    p.setBrush (colorGroup().background());
    p.drawRect (x1, 0, x2, height);
    p.setPen (colorGroup().dark());
    p.drawLine (1, height - 1, width, height - 1);
    p.drawLine (width - 1, 1, width - 1, height - 1);

    p.drawLine (x1 + 1, height - 2, x1 + x2, height - 2);
    p.drawLine (x1 + x2, 1, x1 + x2, height);
    p.drawLine (x1 + x2 - 1, 1, x1 + x2 - 1, height);

    p.end ();
}

//*****************************************************************************
//*****************************************************************************
