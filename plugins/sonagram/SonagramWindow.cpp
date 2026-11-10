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

#include "SonagramPlugin.h"
#include "config.h"

#include <cmath>
#include <new>

#include <QBitmap>
#include <QImage>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QPointer>
#include <QLayout>
#include <QStatusBar>
#include <QTimer>
#include <qnumeric.h>

#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/WindowFunction.h"

#include "libgui/FileDialog.h"
#include "libgui/ImageView.h"
#include "libgui/ScaleWidget.h"

#include "SonagramWindow.h"
#include "SonagramImage.h"

/**
 * delay between two screen updates [ms]
 */
#define REFRESH_DELAY 100

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

/** global list with all sonagram windows */
static QList<Kwave::SonagramWindow *> g_windows;

//****************************************************************************
Kwave::SonagramWindow::SonagramWindow(QWidget *parent)
    :KMainWindow(parent),
     m_status_time(nullptr),
     m_status_freq(nullptr),
     m_status_ampl(nullptr),
     m_image(),
     m_view(nullptr),
     m_overview(nullptr),
     m_points(0),
     m_rate(0),
     m_xscale(nullptr),
     m_yscale(nullptr),
     m_refresh_timer()
{
    QWidget *mainwidget = new(std::nothrow) QWidget(this);
    Q_ASSERT(mainwidget);
    if (!mainwidget) return;
    setCentralWidget(mainwidget);

    QGridLayout *top_layout = new(std::nothrow) QGridLayout(mainwidget);
    Q_ASSERT(top_layout);
    if (!top_layout) return;

    QMenuBar *bar = menuBar();
    Q_ASSERT(bar);
    if (!bar) return ;

//    QMenu *spectral = new QMenu();
//    Q_ASSERT(spectral);
//    if (!spectral) return ;

    QMenu *file = bar->addMenu(i18n("&Sonagram"));
    Q_ASSERT(file);
    if (!file) return ;

    file->addAction(
        QIcon::fromTheme(_("document-import")),
        i18n("&Import from Bitmap..."),
        this, SLOT(load())
    );
    file->addAction(
        QIcon::fromTheme(_("document-export")),
        i18n("&Export to Bitmap..."),
        this, SLOT(save())
    );
    file->addAction(
        QIcon::fromTheme(_("dialog-close")),
        i18n("&Close"),
        QKeySequence::Close,
        this, SLOT(close())
    );

    QStatusBar *status = statusBar();
    Q_ASSERT(status);
    if (!status) return ;

    m_status_time = new(std::nothrow)
        QLabel(i18n("Time: ------ ms"), status);
    m_status_freq = new(std::nothrow)
        QLabel(i18n("Frequency: ------ Hz"), status);
    m_status_ampl = new(std::nothrow)
        QLabel(i18n("Amplitude: --- %"), status);
    status->addPermanentWidget(m_status_time);
    status->addPermanentWidget(m_status_freq);
    status->addPermanentWidget(m_status_ampl);

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
    m_overview->setFixedHeight(SONAGRAM_OVERVIEW_HEIGHT);
    top_layout->addWidget(m_overview, 2, 1);

    connect(m_view, SIGNAL(sigCursorPos(QPoint)),
            this, SLOT(cursorPosChanged(QPoint)));
    connect(&m_refresh_timer, SIGNAL(timeout()),
            this, SLOT(refresh_view()));

    top_layout->setRowStretch(0, 100);
    top_layout->setRowStretch(1, 0);
    top_layout->setRowStretch(2, 0);
    top_layout->setColumnStretch(0, 0);
    top_layout->setColumnStretch(1, 100);
    top_layout->activate();

    if (m_status_time) m_status_time->setText(i18n("Time: 0 ms"));
    if (m_status_freq) m_status_freq->setText(i18n("Frequency: 0 Hz"));
    if (m_status_ampl) m_status_ampl->setText(i18n("Amplitude: 0 %"));

    // try to make 5:3 format (looks best)
    int w = sizeHint().width();
    int h = sizeHint().height();
    if ((w * 3 / 5) < h) w = (h * 5) / 3;
    if ((h * 5 / 3) < w) h = (w * 3) / 5;
    resize(w, h);

    g_windows.push_back(this);
    show();
}

//****************************************************************************
Kwave::SonagramWindow::~SonagramWindow()
{
    g_windows.removeAll(this);
}

//****************************************************************************
quint64 Kwave::SonagramWindow::index() const
{
    return g_windows.indexOf(this);
}

//****************************************************************************
Kwave::SonagramWindow *Kwave::SonagramWindow::fromIndex(quint64 index)
{
    return (qsizetype(index) < g_windows.size()) ? g_windows[index] : nullptr;
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

    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        _("kfiledialog:///kwave_sonagram"),
        Kwave::FileDialog::SaveFile, QString(),
        this, QUrl(), _("*.bmp")
    );
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Save Sonagram"));
    if (dlg->exec() == QDialog::Accepted) {
        QString filename = dlg->selectedUrl().toLocalFile();
        if (!filename.isEmpty()) {
            emit sigCommand(_("plugin:execute(sonagram,save,") +
                            QString::number(index()) + _(",") +
                            filename + _(")"));
        }
    }
    delete dlg;
}

//****************************************************************************
void Kwave::SonagramWindow::load()
{
    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        _("kfiledialog:///kwave_sonagram_import"),
        Kwave::FileDialog::OpenFile, QString(),
        this, QUrl(), _("*.bmp")
    );
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Load Sonagram"));
    if (dlg->exec() == QDialog::Accepted) {
        QString filename = dlg->selectedUrl().toLocalFile();
        if (!filename.isEmpty()) {
            emit sigCommand(_("plugin:execute(sonagram,load,") +
                            filename + _(")"));
        }
    }
    delete dlg;
}

//****************************************************************************
void Kwave::SonagramWindow::setImage(const QImage &image)
{
    Q_ASSERT(m_view);
    if (!m_view) return;

    m_image.setImage(image);

    refresh_view();
}

//****************************************************************************
void Kwave::SonagramWindow::resizeImage(unsigned int width)
{
    m_image.resizeWidth(width);
    refresh_view();
}

//****************************************************************************
void Kwave::SonagramWindow::setOverView(const QImage &overview)
{
    if (m_overview) m_overview->setImage(overview);
}

//****************************************************************************
void Kwave::SonagramWindow::insertSlice(
    const unsigned int slice_nr,
    const std::array<QColor, MAX_FFT_POINTS> &slice)
{
    Q_ASSERT(m_view);
    if (!m_view) return;
    if (m_image.isNull()) return;

    m_image.insertSlice(slice_nr, m_points / 2, slice);

    if (!m_refresh_timer.isActive()) {
        m_refresh_timer.setSingleShot(true);
        m_refresh_timer.start(REFRESH_DELAY);
    }
}

//****************************************************************************
void Kwave::SonagramWindow::refresh_view()
{
    Q_ASSERT(m_view);
    if (!m_view) return;
    m_image.createPalette();
    m_view->setImage(m_image);
}

//***************************************************************************
void Kwave::SonagramWindow::translatePixels2TF(const QPoint p,
                                               double *ms, double *f)
{
    if (qFuzzyIsNull(m_rate)) {
        if (ms) *ms = 0.0;
        if (f)  *f  = 0.0;
    }

    if (ms) {
        // get the time coordinate [0...(N_samples-1) * (1/f_sample) ]
        double t = static_cast<double>(p.x()) *
                   static_cast<double>(m_points) * 1000.0 / m_rate;
        *ms = (!qIsNaN(t) && !qIsInf(t)) ? t : 0.0;
    }

    if (f) {
        // get the frequency coordinate
        double py = (m_points >= 2) ? (m_points / 2) - 1 : 0;
        double y = py - p.y();
        if (y < 0) y = 0;
        double freq = !qFuzzyIsNull(py) ? (y / py * (m_rate / 2.0)) : 0.0;
        *f = (!qIsNaN(freq) && !qIsInf(freq)) ? freq : 0.0;
    }
}

//***************************************************************************
void Kwave::SonagramWindow::updateScaleWidgets()
{
    double ms = 0.0;
    double f  = 0.0;

    translatePixels2TF(QPoint(m_image.width() - 1, 0), &ms, &f);

    m_xscale->setMinMax(0, Kwave::toInt(rint(ms)));
    m_yscale->setMinMax(0, Kwave::toInt(rint(f)));
}

//***************************************************************************
void Kwave::SonagramWindow::setColorMode(Kwave::SonagramPlugin::ColorMode mode)
{
    Q_ASSERT(mode != Kwave::SonagramPlugin::COLOR_MODE_NONE);
    m_image.setColorMode(mode);
    refresh_view();
}

//***************************************************************************
void Kwave::SonagramWindow::setName(const QString &name)
{
    setWindowTitle((name.length()) ?
        i18n("Sonagram of %1", name) :
        i18n("Sonagram")
    );
}

//****************************************************************************
void Kwave::SonagramWindow::cursorPosChanged(const QPoint pos)
{
    QStatusBar *status = statusBar();
    Q_ASSERT(status);
    Q_ASSERT(m_points);
    if (!status) return;
    if (m_image.isNull()) return;
    if (!m_points) return;
    if (qFuzzyIsNull(m_rate)) return;

    double ms;
    double f;
    double a;
    translatePixels2TF(pos, &ms, &f);

    // item 1: time in milliseconds
    if (m_status_time)
        m_status_time->setText(i18n("Time: %1", Kwave::ms2string(ms)));

    // item 2: frequency in Hz
    if (m_status_freq)
        m_status_freq->setText(i18n("Frequency: %1 Hz", Kwave::toInt(f)));

    // item 3: amplitude in %
    if (m_image.valid(pos.x(), pos.y())) {
        if (m_image.format() == QImage::Format_Indexed8)
            a = static_cast<double>(
                m_image.pixelIndex(pos.x(), pos.y())) / 254.0;
        else {
            float h, s, v = 0.0;
            QColor c = m_image.pixelColor(pos.x(), pos.y());
            c.getHsvF(&h, &s, &v);
            a = static_cast<double>(v);
        }
    } else {
        a = 0.0;
    }

    if (m_status_ampl) {
        if (qFuzzyIsNull(a) || qIsNaN(a) || qIsInf(a))
            m_status_ampl->setText(i18n("Amplitude: 0"));
        else
            m_status_ampl->setText(i18n("Amplitude: %1 dB",
                QString::number(20.0 * std::log10(a), 'f', 1)));
    }
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
//***************************************************************************
