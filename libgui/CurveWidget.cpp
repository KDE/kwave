/***************************************************************************
        CurveWidget.cpp  -  widget for editing an interpolated curve
			     -------------------
    begin                : Sep 16 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include <qaccel.h>
#include <qarray.h>
#include <qcursor.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qkeycode.h>
#include <qlist.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kstddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kstddirs.h>

#include "libkwave/Interpolation.h"
#include "libkwave/FileLoader.h"
#include "libkwave/Curve.h"

#include "CurveWidget.h"

//****************************************************************************
CurveWidget::CurveWidget(QWidget *parent, const char *name)
    :QWidget(parent, name), m_width(0), m_height(0), m_curve(), m_menu(0),
     m_preset_menu(0), m_current(0), m_last(0), m_down(false), m_knob(0),
     m_selected_knob(0)
{

    // set the default curve
    m_curve.fromCommand("curve(linear,0,0,1,1)");

    setBackgroundColor(black);

    // create the pixmaps for the selected and non-selected knob
    KIconLoader icon_loader;
    m_knob = icon_loader.loadIcon("knob.xpm", KIcon::Small);
    m_selected_knob = icon_loader.loadIcon("selectedknob.xpm", KIcon::Small);

    // set up the context menu for the right mouse button
    m_menu = new QPopupMenu();
    ASSERT(m_menu);
    if (!m_menu) return;

    QPopupMenu *interpolation = new QPopupMenu();
    ASSERT(interpolation);
    if (!interpolation) return;

    QPopupMenu *del = new QPopupMenu();
    ASSERT(del);
    if (!del) return;

    QPopupMenu *transform = new QPopupMenu();
    ASSERT(transform);
    if (!transform) return;

    /* list of presets */
    m_preset_menu = new QPopupMenu();
    ASSERT(m_preset_menu);
    if (!m_preset_menu) return;
    loadPresetList();
    connect(m_preset_menu, SIGNAL(activated(int)), SLOT(loadPreset(int)) );

    transform->insertItem(i18n("Flip Horizontal"),
	                  this, SLOT(HFlip()));
    transform->insertItem(i18n("Flip Vertical"),
                          this, SLOT(VFlip()));
    transform->insertSeparator();
    transform->insertItem(i18n("into 1st half"),
                          this, SLOT(firstHalf()));
    transform->insertItem(i18n("into 2nd half"),
                          this, SLOT(secondHalf()));

    m_menu->insertItem(i18n("Interpolation"), interpolation);
    m_menu->insertSeparator();
    m_menu->insertItem(i18n("Transform"), transform);
    m_menu->insertItem(i18n("Delete"), del);
    m_menu->insertItem(i18n("Fit In"), this, SLOT(scaleFit()));
    m_menu->insertSeparator();
    m_menu->insertItem(i18n("Presets"), m_preset_menu);
    m_menu->insertItem(i18n("Save Preset"), this, SLOT(savePreset()));

    del->insertItem(i18n("recently selected Point"),
	            this, SLOT(deleteLast()));
    del->insertItem(i18n("every 2nd Point"),
	            this, SLOT(deleteSecond()));

    QStringList types = Interpolation::descriptions(true);
    int id = 0;
    for (QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	interpolation->insertItem(*it, id++);
    }

    connect(interpolation, SIGNAL(activated(int)),
	    SLOT(selectInterpolationType(int)) );

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
    if (m_menu) delete m_menu;
}

//****************************************************************************
QString CurveWidget::getCommand()
{
    return m_curve.getCommand();
}

//****************************************************************************
void CurveWidget::setCurve(const QString &command)
{
    m_curve.fromCommand(command);
}

//****************************************************************************
void CurveWidget::selectInterpolationType(int index)
{
    m_curve.setInterpolationType(Interpolation::findByIndex(index));
    repaint();
}

//****************************************************************************
void CurveWidget::savePreset()
{
    KStandardDirs stddirs;
    stddirs.addResourceType("curves", (QString)"presets" +
	QDir::separator() + (QString)"curves");

    QDir presetDir = stddirs.saveLocation("curves", 0, true);
    QString name = KFileDialog::getSaveFileName(
		       presetDir.path(), "*.curve", this);

    // append the extension if not given
    if (name.find (".curve") == -1) name.append(".curve");

    QFile out(name);
    out.open(IO_WriteOnly);
    QString cmd = m_curve.getCommand();
    out.writeBlock(cmd.data(), cmd.length()+1);

    // reload the list of known presets
    loadPresetList();
}

//****************************************************************************
void CurveWidget::loadPresetList()
{
    KStandardDirs stddirs;
    stddirs.addResourceType("curves", (QString)"presets" +
	QDir::separator() + (QString)"curves");

    QStringList files = stddirs.findAllResources("curves",
	    "*.curve", false, true);
    files.sort();

    m_preset_menu->clear();
    for (unsigned int i=0; i < files.count(); i++) {
	QFileInfo fi(files[i]);
	QString name = fi.fileName();
	m_preset_menu->insertItem(name.left(name.length()-strlen(".curve")));
    }
}

//****************************************************************************
void CurveWidget::loadPreset(int id)
{
    ASSERT(m_preset_menu);
    if (!m_preset_menu) return;

    // invalidate the current selection
    m_current = 0;
    m_last = 0;

    KStandardDirs stddirs;
    stddirs.addResourceType("curves", (QString)"presets" +
	QDir::separator() + (QString)"curves");

    // get the path of the file
    QString filename = m_preset_menu->text(id);
    QString path = stddirs.findResource("curves", filename + ".curve");

    // load the file
    FileLoader loader(path);
    m_curve.fromCommand(loader.buffer());

    repaint();
}

//****************************************************************************
void CurveWidget::secondHalf()
{
    m_curve.secondHalf ();
    m_last = 0;
    repaint();
}

//****************************************************************************
void CurveWidget::firstHalf()
{
    m_curve.firstHalf ();
    m_last = 0;
    repaint();
}

//****************************************************************************
void CurveWidget::deleteSecond()
{
    m_curve.deleteSecondPoint();
    m_last = 0;
    repaint ();
}

//****************************************************************************
void CurveWidget::deleteLast()
{
    if (m_last) {
	m_curve.deletePoint(m_last, true);
	m_last = 0;
	repaint();
    }
}

//****************************************************************************
void CurveWidget::HFlip()
{
    m_curve.HFlip();
    repaint();
}

//****************************************************************************
void CurveWidget::VFlip()
{
    m_curve.VFlip();
    repaint();
}

//****************************************************************************
void CurveWidget::scaleFit()
{
    m_curve.scaleFit();
    repaint();
}

//****************************************************************************
void CurveWidget::addPoint(double newx, double newy)
{
    m_curve.insert(newx, newy);
    m_last = 0;
    repaint();
}

//****************************************************************************
Point *CurveWidget::findPoint(int sx, int sy)
// checks, if given coordinates fit to a control point in the list...
{
    ASSERT(m_width > 1);
    ASSERT(m_height > 1);
    if ((m_width <= 1) || (m_width <= 1)) return 0;

    return m_curve.findPoint(((double)sx) / (m_width-1),
                             ((double)m_height - sy) / (m_height-1));
}

//****************************************************************************
void CurveWidget::mousePressEvent(QMouseEvent *e)
{
    ASSERT(e);
    ASSERT(m_width > 1);
    ASSERT(m_height > 1);
    if (!e || (m_width <= 1) || (m_width <= 1)) return;

    if (e->button() == RightButton) {
	// right mouse button -> context menu
	QPoint popup = QCursor::pos();
	if (m_menu) m_menu->popup(popup);
    } else if (e->button() == LeftButton) {
	// left mouse button -> select existing or create new point
        m_down=true;
	m_current = findPoint(e->pos().x(), e->pos().y());
	if (m_current == 0) {
	    //so, no matching point is found -> generate a new one !
	    addPoint((double) (e->pos().x()) / (m_width-1),
		     (double) (m_height - e->pos().y()) / (m_height-1));
	    m_current = findPoint(e->pos().x(), e->pos().y());
	}
	repaint();
    }
}

//****************************************************************************
void CurveWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_last = m_current;
    m_current = 0;
    m_down = false;
    repaint();
}

//****************************************************************************
void CurveWidget::mouseMoveEvent(QMouseEvent *e )
{
    ASSERT(e);
    ASSERT(m_width > 1);
    ASSERT(m_height > 1);
    if (!e || (m_width <= 1) || (m_width <= 1)) return;

    int x = e->pos().x();
    int y = e->pos().y();

    // if a point is selected...
    if (m_current) {
	m_current->x = (double) (x) / (m_width-1);
	m_current->y = (double) (m_height - y) / (m_height-1);

	if (m_current->x < 0.0) m_current->x = 0.0;
	if (m_current->y < 0.0) m_current->y = 0.0;
	if (m_current->x > 1.0) m_current->x = 1.0;
	if (m_current->y > 1.0) m_current->y = 1.0;

	Point *prev = m_curve.previous(m_current);
	Point *next = m_curve.next(m_current);

	if (prev) {
	    if (m_current->x < prev->x)
		m_current->x = prev->x + (1 / (double)(m_width-1));
	}
	if (next) {
	    if (m_current->x > next->x)
		m_current->x = next->x - (1 / (double)(m_width-1));
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
    QPainter p;
    int lx, ly;

    m_height = rect().height();
    m_width  = rect().width();

    if (!m_curve.count()) return; // nothing to draw

    const int kw = m_knob.width();
    const int kh = m_knob.height();

    QArray<double> y = m_curve.interpolation(m_width);
    ASSERT(static_cast<int>(y.count()) == m_width);
    if (static_cast<int>(y.count()) < m_width) {
	warning("CurveWidget: unable to get interpolation !");
	return;
    }

    p.begin(this);
    p.setPen(white);

    // draw the lines
    int ay;
    ly = (m_height-1) - (int)(y[0] * (m_height-1));
    for (int i=1; i < m_width; i++) {
	ay = (m_height-1) - (int)(y[i] * (m_height-1));
	p.drawLine (i-1, ly, i, ay);
	ly = ay;
    }

    // draw the points (knobs)
    Point *pt;
    for (pt = m_curve.first(); (pt); pt = m_curve.next(pt)) {
	lx = (int)(pt->x * (m_width-1));
	ly = (m_height-1) - (int)(pt->y * (m_height-1));
	
	if ((pt == m_current) || (!m_down && (pt == m_last)) )
	    bitBlt(this, lx - (kw >> 1), ly - (kh >> 1),
	           &m_selected_knob, 0, 0, kw, kh);
	else bitBlt(this, lx - (kw >> 1), ly - (kh >> 1),
	            &m_knob, 0, 0, kw, kh);
    }
    p.end();

}

//****************************************************************************
//****************************************************************************
