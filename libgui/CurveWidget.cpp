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


#include <math.h>

#include <limits>
#include <new>

#include <QAction>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QPointer>
#include <QShortcut>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <KLocalizedString>
#include <KIconLoader>

#include "libkwave/Interpolation.h"
#include "libkwave/Logger.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/CurveWidget.h"
#include "libgui/FileDialog.h"

//***************************************************************************
Kwave::CurveWidget::CurveWidget(QWidget *parent)
    :QWidget(parent), m_width(0), m_height(0), m_curve(), m_current(Kwave::Curve::NoPoint),
     m_last(Kwave::Curve::NoPoint),
     m_down(false), m_knob(), m_selected_knob()
{
    KIconLoader *icon_loader = KIconLoader::global();

    // set the default curve
    m_curve.fromCommand(_("curve(linear,0,0,1,1)"));

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    // create the pixmaps for the selected and non-selected knob
    if (icon_loader) {
        m_knob = icon_loader->loadIcon(_("knob.xpm"), KIconLoader::Small);
        m_selected_knob = icon_loader->loadIcon(_("selectedknob.xpm"),
                                                KIconLoader::Small);
    }

    // set up the context menu for the right mouse button
    m_menu = new(std::nothrow) QMenu(this);
    Q_ASSERT(m_menu);
    if (!m_menu) return;

    QMenu *interpolation = m_menu->addMenu(i18n("Interpolation"));
    Q_ASSERT(interpolation);
    if (!interpolation) return;

    m_menu->addSeparator();
    QMenu *transform = m_menu->addMenu(i18n("Transform"));
    Q_ASSERT(transform);
    if (!transform) return;
    transform->addAction(i18n("Flip horizontal"),
                         this, SLOT(HFlip()));
    transform->addAction(i18n("Flip vertical"),
                         this, SLOT(VFlip()));
    transform->addSeparator();
    transform->addAction(i18n("Into first half"),
                         this, SLOT(firstHalf()));
    transform->addAction(i18n("Into second half"),
                         this, SLOT(secondHalf()));

    QMenu *del = m_menu->addMenu(i18n("Delete"));
    Q_ASSERT(del);
    if (!del) return;

    m_menu->addAction(i18n("Fit In"), this, SLOT(scaleFit()));
    m_menu->addSeparator();

    /* list of presets */
    m_preset_menu = m_menu->addMenu(i18n("Presets"));
    Q_ASSERT(m_preset_menu);
    if (!m_preset_menu) return;
    loadPresetList();
    connect(m_preset_menu, &QMenu::triggered,
            this, &CurveWidget::loadPreset);

    m_menu->addAction(
        QIcon::fromTheme(_("document-export")),
        i18n("Save Preset"),
        this, SLOT(savePreset()));

    del->addAction(
        QIcon::fromTheme(_("edit-delete")),
        i18n("Currently Selected Point"),
        QKeySequence::Delete,
        this,
        SLOT(deleteLast()));
    del->addAction(i18n("Every Second Point"),
                   this, SLOT(deleteSecond()));

    QStringList types = Kwave::Interpolation::descriptions(true);
    int id = 0;
    foreach (const QString &text, types) {
        QAction *action = new(std::nothrow) QAction(interpolation);
        action->setText(text);
        action->setData(id++);
        interpolation->addAction(action);
    }
    connect(interpolation, SIGNAL(triggered(QAction*)),
            this,          SLOT(selectInterpolationType(QAction*)));

    setMouseTracking(true);

    QShortcut *delkey = new(std::nothrow) QShortcut(this);
    Q_ASSERT(delkey);
    if (!delkey) return;
    delkey->setKey(Qt::Key_Delete);
    connect(delkey, SIGNAL(activated()), this, SLOT (deleteLast()));

}

//***************************************************************************
Kwave::CurveWidget::~CurveWidget()
{
    delete m_menu;
    m_menu = nullptr;
}

//***************************************************************************
QString Kwave::CurveWidget::getCommand()
{
    return m_curve.getCommand();
}

//***************************************************************************
void Kwave::CurveWidget::setCurve(const QString &command)
{
    m_curve.fromCommand(command);
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::selectInterpolationType(QAction *action)
{
    if (!action) return;

    QVariant data = action->data();
    int index = data.toInt();

    m_curve.setInterpolationType(Kwave::Interpolation::findByIndex(index));

    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::savePreset()
{
    QString presetSubDir = _("presets") + QDir::separator() + _("curves");
    QString presetPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) +
        QDir::separator() + presetSubDir;
    if (!QDir(presetPath).exists()) {
        Kwave::Logger::log(this, Logger::Info,
            _("curve preset directory did not exist, creating '%1'").arg(
            presetPath));
        QDir(presetPath).mkpath(presetPath);
    }

    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        presetPath, Kwave::FileDialog::SaveFile,
        _("*.curve *.CURVE|") +
        i18nc("Filter description for Kwave curve presets, "
              "for use in a FileDialog",
              "Kwave curve preset (*.curve)"),
         this, QUrl(), _("*.curve"));
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Save Curve Preset"));
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }

    QString name = dlg->selectedUrl().toLocalFile();
    delete dlg;

    // append the extension if not given
    if (!name.endsWith(_(".curve")))
        name.append(_(".curve"));

    QFile out(name);
    if (!out.open(QIODevice::WriteOnly)) return;
    QString cmd = m_curve.getCommand();
    out.write(DBG(cmd), cmd.length());

    // reload the list of known presets
    loadPresetList();
}

//***************************************************************************
void Kwave::CurveWidget::loadPresetList()
{
    const QChar s = QDir::separator();
    QString presetSubDir = s + _("kwave") + s + _("presets") + s + _("curves");
    QStringList files;
    QStringList presetPaths = QStandardPaths::standardLocations(
        QStandardPaths::GenericDataLocation);
    foreach (const QString &path, presetPaths) {
        QDir d(path + presetSubDir);
        QStringList f = d.entryList(QDir::Files, QDir::Name);
        foreach (const QString &file, f) {
            QString preset = d.path() + s + file;
            if (!files.contains(preset)) files.append(preset);
        }
    }
    files.sort();

    m_preset_menu->clear();
    foreach (const QString &file, files) {
        QFileInfo fi(file);
        QString name = fi.baseName();
        QAction *action = new(std::nothrow) QAction(name, m_preset_menu);
        Q_ASSERT(action);
        if (!action) continue;
        action->setData(file);
        m_preset_menu->addAction(action);
    }
}

//***************************************************************************
void Kwave::CurveWidget::loadPreset(QAction *action)
{
    Q_ASSERT(m_preset_menu);
    Q_ASSERT(action);
    if (!m_preset_menu || !action) return;
    if (!action->data().isValid()) return;

    // invalidate the current selection
    m_current = Kwave::Curve::NoPoint;
    m_last    = Kwave::Curve::NoPoint;

    // get the path of the file and check whether it (still) exists
    QString filename = action->data().toString();
    QFileInfo fi(filename);
    if (!fi.exists(filename)) return;

    // load the file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("CurveWidget::loadPreset('%s') - FAILED", DBG(filename));
        return;
    }
    QTextStream stream(&file);
    m_curve.fromCommand(stream.readLine());
    file.close();

    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::secondHalf()
{
    m_curve.secondHalf ();
    m_last = Kwave::Curve::NoPoint;
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::firstHalf()
{
    m_curve.firstHalf ();
    m_last = Kwave::Curve::NoPoint;
    repaint();
}

//****************************************************************************
void Kwave::CurveWidget::deleteSecond()
{
    m_curve.deleteSecondPoint();
    m_last = Kwave::Curve::NoPoint;
    repaint ();
}

//****************************************************************************
void Kwave::CurveWidget::deleteLast()
{
    if (m_last != Kwave::Curve::NoPoint) {
        m_curve.deletePoint(m_last, true);
        m_last = Kwave::Curve::NoPoint;
        repaint();
    }
}

//***************************************************************************
void Kwave::CurveWidget::HFlip()
{
    m_curve.HFlip();
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::VFlip()
{
    m_curve.VFlip();
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::scaleFit()
{
    m_curve.scaleFit();
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::addPoint(double newx, double newy)
{
    m_curve.insert(newx, newy);
    m_last = Kwave::Curve::NoPoint;
    repaint();
}

//***************************************************************************
Kwave::Curve::Point Kwave::CurveWidget::findPoint(int sx, int sy)
// checks, if given coordinates fit to a control point in the list...
{
    Q_ASSERT(m_width > 1);
    Q_ASSERT(m_height > 1);
    if ((m_width <= 1) || (m_height <= 1)) return Kwave::Curve::NoPoint;

    return m_curve.findPoint((static_cast<double>(sx)) / (m_width - 1),
        (static_cast<double>(m_height) - sy) / (m_height - 1));
}

//***************************************************************************
void Kwave::CurveWidget::mousePressEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_width > 1);
    Q_ASSERT(m_height > 1);
    if (!e || (m_width <= 1) || (m_height <= 1)) return;

    if (e->buttons() == Qt::RightButton) {
        // right mouse button -> context menu
        QPoint popup = QCursor::pos();
        if (m_menu) m_menu->popup(popup);
    } else if (e->buttons() == Qt::LeftButton) {
        // left mouse button -> select existing or create new point
        m_down = true;
        m_current = findPoint(e->pos().x(), e->pos().y());
        if (m_current == Kwave::Curve::NoPoint) {
            // no matching point is found -> generate a new one !
            addPoint(static_cast<double>(e->pos().x()) / (m_width - 1),
                     static_cast<double>(m_height - e->pos().y()) /
                     (m_height - 1));
            m_current = findPoint(e->pos().x(), e->pos().y());
        }
        repaint();
    }
}

//***************************************************************************
void Kwave::CurveWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_last = m_current;
    m_current = Kwave::Curve::NoPoint;
    m_down = false;
    repaint();
}

//***************************************************************************
void Kwave::CurveWidget::mouseMoveEvent(QMouseEvent *e )
{
    Q_ASSERT(e);
    Q_ASSERT(m_width > 1);
    Q_ASSERT(m_height > 1);
    if (!e || (m_width <= 1) || (m_height <= 1)) return;

    int x = e->pos().x();
    int y = e->pos().y();

    // if a point is selected...
    if (m_current != Kwave::Curve::NoPoint) {
        if (m_current == m_curve.first()) x = 0;
        if (m_current == m_curve.last())  x = m_width - 1;

        m_curve.deletePoint(m_current, false);

        m_current.setX(static_cast<double>(x) / (m_width - 1));
        m_current.setY(static_cast<double>(m_height - y) / (m_height - 1));

        if (m_current.x() < 0.0) m_current.setX(0.0);
        if (m_current.y() < 0.0) m_current.setY(0.0);
        if (m_current.x() > 1.0) m_current.setX(1.0);
        if (m_current.y() > 1.0) m_current.setY(1.0);

        double dx = (1.0 / static_cast<double>(m_width - 1));
        do {
            Kwave::Curve::Point nearest = m_curve.findPoint(
                m_current.x(), m_current.y(), 1.0);
            if (qFuzzyCompare(nearest.x(), m_current.x())) {
                if (nearest == m_curve.last())
                    m_current.setX(m_current.x() - dx);
                else
                    m_current.setX(m_current.x() + dx);
            }
            else
                break;
        } while (true);

        m_curve.insert(m_current.x(), m_current.y());

        repaint ();
    } else {
        if (findPoint(x, y) != Kwave::Curve::NoPoint)
            setCursor(Qt::SizeAllCursor);
        else
            setCursor(Qt::ArrowCursor);
    }
}

//***************************************************************************
void Kwave::CurveWidget::paintEvent(QPaintEvent *)
{
//    qDebug("CurveWidget::paintEvent (QPaintEvent *)");
    QPainter p;
    int ly;

    m_height = rect().height();
    m_width  = rect().width();

    if (!m_curve.count()) return; // nothing to draw

    const int kw = m_knob.width();
    const int kh = m_knob.height();

    QVector<double> y = m_curve.interpolation(m_width);
    Q_ASSERT(Kwave::toInt(y.count()) == m_width);
    if (Kwave::toInt(y.count()) < m_width) {
        qWarning("CurveWidget: unable to get interpolation !");
        return;
    }

    p.begin(this);
    p.fillRect(rect(), QBrush(palette().dark()));
    p.setPen(palette().text().color());

    // draw the lines
    ly = (m_height-1) - Kwave::toInt(y[0] * (m_height - 1));
    for (int i = 1; i < m_width; i++) {
        int ay = (m_height-1) - Kwave::toInt(y[i] * (m_height - 1));
        p.drawLine (i - 1, ly, i, ay);
        ly = ay;
    }

    // draw the points (knobs)
    foreach (const Kwave::Curve::Point &pt, m_curve) {
        int lx = Kwave::toInt(pt.x() * (m_width - 1));
        ly = (m_height - 1) - Kwave::toInt(pt.y() * (m_height - 1));

        if ((pt == m_current) || (!m_down && (pt == m_last)) )
            p.drawPixmap(lx - (kw >> 1), ly - (kh >> 1), m_selected_knob);
        else
            p.drawPixmap(lx - (kw >> 1), ly - (kh >> 1), m_knob);
    }
    p.end();

}

//***************************************************************************
//***************************************************************************

#include "moc_CurveWidget.cpp"
