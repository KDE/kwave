
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include <qstring.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qimage.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qfiledlg.h>
#include <qpopupmenu.h>

#include <kapp.h>
#include <kiconloader.h>

#include <libkwave/Interpolation.h>
#include <libkwave/FileLoader.h>
#include <libkwave/Curve.h>
#include <libkwave/Global.h>
#include <libkwave/String.h>

#include "CurveWidget.h"

#include <qwidget.h>
#include <libkwave/Curve.h>

extern Global globals;

//****************************************************************************
CurveWidget::CurveWidget(QWidget *parent, const char *init, int keepborder)
    :QWidget(parent)
{
    act = 0;
    down = false;
    height = 0;
    interpolationtype = 0;
    keepborder = 0;
    knob = 0;
    knobcount = 1;
    last = 0;
    menu = 0;
    namelist = 0;
    pixmap = 0;
    points = 0;
    presetDir = 0;
    selectedknob = 0;
    width = 0;

    this->keepborder = keepborder;

    if (init) points = new Curve(init);
    else points = new Curve("curve (linear,0,0,1,1)");
    ASSERT(points);
    if (!points) return;

    setBackgroundColor(black);

    const QString dirname=KApplication::getKApplication()->kde_datadir();
    QDir dir (dirname.data());
    dir.cd ("kwave");
    dir.cd ("pics");

    knob = new QPixmap(dir.filePath("knob.xpm"));
    ASSERT(knob);
    if (!knob) return;

    selectedknob = new QPixmap(dir.filePath("selectedknob.xpm"));
    ASSERT(selectedknob);
    if (!selectedknob) return;

    menu = new QPopupMenu ();
    ASSERT(menu);
    if (!menu) return;

    QPopupMenu *interpolation = new QPopupMenu ();
    ASSERT(interpolation);
    if (!interpolation) return;

    QPopupMenu *del = new QPopupMenu ();
    ASSERT(del);
    if (!del) return;

    QPopupMenu *transform = new QPopupMenu();
    ASSERT(transform);
    if (!transform) return;

    QPopupMenu *presets = new QPopupMenu();
    ASSERT(presets);
    if (!presets) return;

    transform->insertItem(i18n("Flip Horizontal"),
	                  this, SLOT(HFlip()));
    transform->insertItem(i18n("Flip Vertical"),
                          this, SLOT(VFlip()));
    transform->insertSeparator();
    transform->insertItem(i18n("into 1st half"),
                          this, SLOT(firstHalf()));
    transform->insertItem(i18n("into 2nd half"),
                          this, SLOT(secondHalf()));

    menu->insertItem(i18n("Interpolation"), interpolation);
    menu->insertSeparator();
    menu->insertItem(i18n("Transform"), transform);
    menu->insertItem(i18n("Delete"), del);
    menu->insertItem(i18n("Fit In"), this, SLOT(scaleFit()));
    menu->insertSeparator();
    menu->insertItem(i18n("Presets"), presets);
    menu->insertItem(i18n("Save Preset"), this, SLOT(savePreset()));

    del->insertItem(i18n("recently selected Point"),
	            this, SLOT(deleteLast()));
    del->insertItem(i18n("every 2nd Point"),
	            this, SLOT(deleteSecond()));

    presetDir = new QDir(globals.localconfigDir);
    ASSERT(presetDir);
    if (!presetDir) return;

    if (!presetDir->cd ("presets")) {
	presetDir->mkdir ("presets");
	presetDir->cd ("presets");
    }
    if (!presetDir->cd ("curves")) {
	presetDir->mkdir ("curves");
	presetDir->cd ("curves");
    }

    presetDir->setNameFilter ("*.curve");

    namelist = (QStrList *)presetDir->entryList();
    ASSERT(namelist);
    if (!namelist) return;

    for (char *tmp = namelist->first(); tmp; tmp = namelist->next()) {
	char buf[strlen(tmp) - 5];
	strncpy (buf, tmp, strlen(tmp) - 6);
	buf[strlen(tmp) - 6] = 0;
	presets->insertItem (buf);
    }
    connect( presets, SIGNAL(activated(int)), SLOT(loadPreset(int)) );

    char **names = (char **) Interpolation::getTypes();
    int i = 0;
    ASSERT(names);
    if (!names) return;

    while (names[i]) {
	interpolation->insertItem(i18n(names[i++]));
    }

    connect(interpolation, SIGNAL(activated(int)),
	    SLOT(setType(int)) );

    setMouseTracking(true);

    QAccel *delkey = new QAccel (this);
    ASSERT(delkey);
    if (!delkey) return;

    delkey->connectItem(delkey->insertItem(Key_Delete),
                        this, SLOT (deleteLast()));

}

//****************************************************************************
CurveWidget::~CurveWidget()
{
    if (presetDir) delete[] presetDir;
    if (points) delete points;
    if (knob) delete knob;
    if (pixmap) delete pixmap;
    if (selectedknob) delete selectedknob;
    if (menu) delete menu;
}

//****************************************************************************
const char *CurveWidget::getCommand()
{
    ASSERT(points);
    return (points) ? points->getCommand() : 0;
}

//****************************************************************************
void CurveWidget::setCurve(const char *next)
{
    if (points) delete points;
    points = new Curve(next);
    ASSERT(points);
}

//****************************************************************************
void CurveWidget::setType(int type)
{
    ASSERT(points);
    if (!points) return;

    points->setInterpolationType (Interpolation::getTypes()[type]);
    repaint();
}

//****************************************************************************
void CurveWidget::savePreset()
{
    ASSERT(points);
    ASSERT(presetDir);
    if (!points) return;
    if (!presetDir) return;

    QString name = QFileDialog::getSaveFileName(
		       presetDir->path(), "*.curve", this);

    if (name.find (".curve") == -1) name.append(".curve");
    QFile out(name.data());
    out.open (IO_WriteOnly);
    const char *buf = points->getCommand();
    out.writeBlock (buf, strlen(buf));
}

//****************************************************************************
void CurveWidget::loadPreset(int num)
{
    ASSERT(namelist);
    ASSERT(presetDir);
    if (!namelist) return;
    if (!presetDir) return;

    char *name = namelist->at(num);

    FileLoader loader (presetDir->absFilePath(name));

    if (points) delete points;
    points = new Curve (loader.getMem());
    ASSERT(points);

    repaint();
}

//****************************************************************************
void CurveWidget::secondHalf()
{
    ASSERT(points);
    if (!points) return;

    points->secondHalf ();
    last = 0;
    repaint();
}

//****************************************************************************
void CurveWidget::firstHalf()
{
    ASSERT(points);
    if (!points) return;

    points->firstHalf ();
    last = 0;
    repaint();
}

//****************************************************************************
void CurveWidget::deleteSecond()
{
    ASSERT(points);
    if (!points) return;

    points->deleteSecondPoint();
    last = 0;
    repaint ();
}

//****************************************************************************
void CurveWidget::deleteLast()
{
    ASSERT(points);
    if (!points) return;

    if (last) {
	points->deletePoint(last, false);
	last = 0;
	repaint ();
    }
}

//****************************************************************************
void CurveWidget::HFlip()
{
    ASSERT(points);
    if (!points) return;

    points->HFlip();
    repaint ();
}

//****************************************************************************
void CurveWidget::VFlip()
{
    ASSERT(points);
    if (!points) return;

    points->VFlip();
    repaint ();
}

//****************************************************************************
void CurveWidget::scaleFit()
{
    ASSERT(points);
    if (!points) return;

    points->scaleFit ();
    repaint ();
}

//****************************************************************************
void CurveWidget::addPoint(double newx, double newy)
{
    ASSERT(points);
    if (!points) return;

    points->addPoint(newx, newy);
    last = 0;
    repaint();
}

//****************************************************************************
Point *CurveWidget::findPoint(int sx, int sy)
// checks, if given coordinates fit to a control point in the list...
{
    ASSERT(points);
    if (!points) return 0;

    return points->findPoint(((double)sx) / width,
                             ((double)height - sy) / height);
}

//****************************************************************************
void CurveWidget::mousePressEvent( QMouseEvent *e)
{
    ASSERT(e);
    if (!e) return;

    if (e->button() == RightButton) {
	QPoint popup = QCursor::pos();
	if (menu) menu->popup(popup);
    } else {
        down=true;
	act = findPoint(e->pos().x(), e->pos().y());
	if (act == 0) {
	    //so, no matching point is found -> generate a new one !
	    addPoint((double) (e->pos().x()) / width,
		     (double) (height - e->pos().y()) / height);
	    act = findPoint(e->pos().x(), e->pos().y());
	}
	repaint();
    }
}

//****************************************************************************
void CurveWidget::mouseReleaseEvent( QMouseEvent *)
{
    last = act;
    act = 0;
    down = false;
    repaint();
}

//****************************************************************************
void CurveWidget::mouseMoveEvent( QMouseEvent *e )
{
    ASSERT(points);
    ASSERT(e);
    if (!e) return;
    if (!points) return;

    int x = e->pos().x();
    int y = e->pos().y();

    // if a point is selected...
    if (act) {
	act->x = (double) (x) / width;
	act->y = (double) (height - y) / height;

	if (act->x < 0) act->x = 0;
	if (act->y < 0) act->y = 0;
	if (act->x > 1) act->x = 1;
	if (act->y > 1) act->y = 1;

	Point *prev = points->previous(act);
	Point *next = points->next(act);

	if (prev) {
	    if (act->x < prev->x)
		act->x = prev->x + (1 / (double) width);
	}
	if (next) {
	    if (act->x > next->x)
		act->x = next->x - (1 / (double) width);
	}
	repaint ();
    } else {
	if (findPoint(x, y)) setCursor(sizeAllCursor);
	else setCursor(arrowCursor);
    }
}

//****************************************************************************
void CurveWidget::paintEvent(QPaintEvent *)
{
//    debug("CurveWidget::paintEvent (QPaintEvent *)");
    ASSERT(points);
    if (!points) return;

    Point *tmp;

    height = rect().height();
    width = rect().width();

    int kw = (knob) ? knob->width() : 0;
    int kh = (knob) ? knob->height() : 0;
    int lx, ly, ay;

    double *y = points->getInterpolation (width);
    ASSERT(y);
    if (!y) {
	warning("CurveWidget: could not get Interpolation !\n");
	return;
    }

    p.begin (this);
    p.setPen (white);
    ly = height - (int)(y[0] * height);

    for (int i = 1; i < width; i++) {
	ay = height - (int)(y[i] * height);
	p.drawLine (i - 1, ly, i, ay);
	ly = ay;
    }

    for ( tmp = points->first(); tmp ; tmp = points->next(tmp) ) {
	lx = (int)(tmp->x * width);
	ly = height - (int)(tmp->y * height);

	if ((tmp == act) || (!down && (tmp == last)) )
	    bitBlt(this, lx - kw / 2, ly - kh / 2,
	           selectedknob, 0, 0, kw, kh);
	else bitBlt(this, lx - kw / 2, ly - kh / 2,
	            knob, 0, 0, kw, kh);
    }
    p.end();
}

//****************************************************************************
//****************************************************************************
