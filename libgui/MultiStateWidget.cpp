//provides methods of multistateWidget a Class that switches the image it
// displays on clicking, used for the channel enable/disable lamps...

#include "config.h"
#include <qdir.h>
#include <qimage.h>
#include <qstring.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qpixmap.h>
#include "kstddirs.h"
#include "MultiStateWidget.h"

QList <QPixmap> *pixmaps = 0;
QStringList *pixnames = 0;

//**********************************************************
MultiStateWidget::MultiStateWidget(QWidget *parent, int num, int count)
    :QWidget(parent)
{
    this->act = 0;
    this->count = count;
    this->number = num;
    this->states = new int[count];
    ASSERT(states);
    if (!states) return;

    for (int i = 0; i < count; i++)
    	states[i] = 0;

    if (pixmaps == 0) pixmaps = new QList<QPixmap>();
    ASSERT(pixmaps);

    if (pixnames == 0) pixnames = new QStringList();
    ASSERT(pixnames);

    resize(20, 20);
}

//**********************************************************
void MultiStateWidget::setNumber(int number)
{
    this->number = number;
}

//**********************************************************
int MultiStateWidget::addPixmap(const QString &filename)
{
    ASSERT(pixmaps);
    ASSERT(pixnames);
    if (!pixmaps) return -1;
    if (!pixnames) return -1;

    int result = pixnames->findIndex(filename);
    if (result == -1) {
	QPixmap *newpix = new QPixmap();
	ASSERT(newpix);
	if (!newpix) return -1;
	
	QString file = locate("data", QString("kwave/pics/")+filename);
	QImage *img = new QImage(file);
	ASSERT(img);
	if (!img) return -1;
	
	newpix->convertFromImage(*img);
	pixmaps->append(newpix);
	pixnames->append(filename);
	return pixmaps->at();
    } else
	return result;

    return -1;
}

//**********************************************************
void MultiStateWidget::setStates(int *newstates)
{
    for (int i = 0; i < count; i++)
	states[i] = newstates[i];
}

//**********************************************************
void MultiStateWidget::setState(int newstate)
{
    act = newstate;
    if (act >= count) act = count-1;
    if (act < 0) act = 0;
    repaint();
}

//**********************************************************
void MultiStateWidget::nextState()
{
    act++;
    if (act >= count) act = 0;
    repaint();
}

//**********************************************************
void MultiStateWidget::mouseReleaseEvent( QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    if (e->button() == LeftButton) {
	nextState ();
	emit clicked(number);
    }
}

//**********************************************************
MultiStateWidget::~MultiStateWidget()
{
    if (states) delete[] states;
}

//**********************************************************
void MultiStateWidget::paintEvent (QPaintEvent *)
{
    ASSERT(pixmaps);
    if (!pixmaps) return;

    QPixmap *img;
    img = pixmaps->at(states[act]);

    if (img) bitBlt(this, 0, 0,	img, 0, 0,
                    img->width(), img->height(), CopyROP);
}

//**********************************************************
//**********************************************************
