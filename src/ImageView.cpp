
#include "config.h"
#include <qimage.h>
#include <qcursor.h>
#include "ImageView.h"

//****************************************************************************
ImageView::ImageView (QWidget *parent)
    :QWidget(parent)
{
    height = 0;
    image = 0;
    lh = -1;
    lw = -1;
    offset = 0;
    width = 0;

    setCursor (crossCursor);
}

//****************************************************************************
ImageView::~ImageView()
{
}

//****************************************************************************
void ImageView::mouseMoveEvent(QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    int x = e->pos().x();
    if ((x < width) && (x >= 0)) {
	int y = e->pos().y();

	if ((y >= 0) && (y < height))
	    emit info ((double)x / width, (double)(height - y) / height);
    }
}

//****************************************************************************
int ImageView::getOffset()
{
    return offset;
}

//****************************************************************************
int ImageView::getWidth()
{
    return width;
}

//****************************************************************************
void ImageView::setImage(const QImage *image)
{
    this->image = image;
    repaint();
}

//****************************************************************************
void ImageView::setOffset(int offset)
{
    if (this->offset != offset) {
	this->offset = offset;
	repaint();
    }
}

//****************************************************************************
void ImageView::paintEvent(QPaintEvent *)
{
    height = rect().height();
    width = rect().width();

    if (!image) return;

    ASSERT(image->width());
    ASSERT(image->height());
    if (!image->width()) return;
    if (!image->height()) return;
	
    if (offset > image->width() - width)
	offset = image->width() - width;
		
    QWMatrix matrix;
    QPixmap newmap;
//    newmap.convertFromImage(*image, 0);
    newmap.convertFromImage(*image, ColorOnly|ThresholdDither|AvoidDither);

    if (image->width() < width) {
	offset = 0;
	matrix.scale(((float)width) / image->width(),
	            ((float)height) / image->height());
    } else
	matrix.scale (1, ((float)height) / image->height());

    map = (newmap.xForm (matrix));

    bitBlt (this, 0, 0, &map, offset, 0, width, height);

    if ( (lh != height) || (lw != width)) {
	emit viewInfo (offset, width, image->width());
    }

    lh = height;
    lw = width;
}

//****************************************************************************
//****************************************************************************
