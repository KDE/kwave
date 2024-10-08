/***************************************************************************
        ZoomToolBar.cpp  -  Toolbar for zoom control
                             -------------------
    begin                : 2014-08-12
    copyright            : (C) 2014 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

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

#include <cmath>
#include <array>

#include <QAction>
#include <QIcon>

#include <KComboBox>
#include <KLocalizedString>
#include <KMainWindow>
#include <KStandardAction>

#include "libkwave/Selection.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"

#include "libgui/MenuManager.h"
#include "libgui/Zoomable.h"

#include "FileContext.h"
#include "TopWidget.h"
#include "ZoomToolBar.h"

/** role value for entries in the zoom combo box, "predefined" flag (bool) */
constexpr auto ZOOM_DATA_PREDEFINED = Qt::UserRole + 0;

/** role value for entries in the zoom combo box, "time" in ms (double) */
constexpr auto ZOOM_DATA_TIME = Qt::UserRole + 1;

//***************************************************************************
Kwave::ZoomToolBar::ZoomToolBar(TopWidget *parent, const QString &name)
    :KToolBar(name, parent, true),
     m_context(nullptr),
     m_action_zoomselection(nullptr),
     m_action_zoomin(nullptr),
     m_action_zoomout(nullptr),
     m_action_zoomnormal(nullptr),
     m_action_zoomall(nullptr),
     m_action_zoomselect(nullptr),
     m_zoomselect(nullptr)
{

    m_action_zoomselection = KStandardAction::zoom(this, &Kwave::ZoomToolBar::zoomSelection, this);
    m_action_zoomselection->setText(i18n("Zoom to selection")); 
    m_action_zoomselection->setToolTip(i18n("Zoom to selection")); 
    addAction(m_action_zoomselection);

    m_action_zoomin = KStandardAction::zoomIn(this, &Kwave::ZoomToolBar::zoomIn, this);
    addAction(m_action_zoomin);

    m_action_zoomout = KStandardAction::zoomOut(this, &Kwave::ZoomToolBar::zoomOut, this);
    addAction(m_action_zoomout);

    m_action_zoomnormal = KStandardAction::actualSize(this, &Kwave::ZoomToolBar::zoomNormal, this);
    m_action_zoomnormal->setText(i18n("Zoom to 100%"));
    m_action_zoomnormal->setToolTip(i18n("Zoom to 100%"));
    addAction(m_action_zoomnormal);

    m_action_zoomall = KStandardAction::fitToPage(this, &Kwave::ZoomToolBar::zoomAll, this);
    m_action_zoomall->setText(i18n("Zoom to all"));
    m_action_zoomall->setToolTip(i18n("Zoom to all"));
    addAction(m_action_zoomall);

    // zoom selection combo box
    m_zoomselect = new KComboBox(this);
    m_zoomselect->setToolTip(i18n("Select zoom factor"));
    m_zoomselect->setInsertPolicy(QComboBox::InsertAtTop);
    m_zoomselect->setEditable(false);

    /** Initialized list of zoom factors */
    const auto zoomFactors = std::to_array<std::pair<QString, quint32>>({
        { i18n("%1 ms",   1),            1L},
        { i18n("%1 ms",  10),           10L},
        { i18n("%1 ms", 100),          100L},
        { i18n("%1 sec",  1),         1000L},
        { i18n("%1 sec", 10),     10L*1000L},
        { i18n("%1 sec", 30),     30L*1000L},
        { i18n("%1 min",  1),  1L*60L*1000L},
        { i18n("%1 min",  3),  3L*60L*1000L},
        { i18n("%1 min",  5),  5L*60L*1000L},
        { i18n("%1 min", 10), 10L*60L*1000L},
        { i18n("%1 min", 30), 30L*60L*1000L},
        { i18n("%1 min", 60), 60L*60L*1000L},
    });

    for (const auto &[text, ms] : zoomFactors) {
        m_zoomselect->addItem(text);
        int index = m_zoomselect->count() - 1;
        unsigned int time = ms;
        m_zoomselect->setItemData(index, QVariant(true), ZOOM_DATA_PREDEFINED);
        m_zoomselect->setItemData(index, QVariant(time), ZOOM_DATA_TIME);
    }

    m_action_zoomselect = addWidget(m_zoomselect);
    connect(m_zoomselect, &KComboBox::activated,
            this, &Kwave::ZoomToolBar::selectZoom);
    connect(this, &Kwave::ZoomToolBar::sigCommand,
            parent, &TopWidget::forwardCommand);

    int h = m_zoomselect->sizeHint().height();
    m_zoomselect->setMinimumWidth(h * 5);
    m_zoomselect->setFocusPolicy(Qt::FocusPolicy(Qt::ClickFocus |
                                                 Qt::TabFocus));

    m_zoomselect->clearFocus();
}

//***************************************************************************
Kwave::ZoomToolBar::~ZoomToolBar() = default;

//***************************************************************************
void Kwave::ZoomToolBar::contextSwitched(Kwave::FileContext *context)
{
    if (context == m_context) return; // nothing to do
    m_context = context;
    updateToolbar();
}

//***************************************************************************
void Kwave::ZoomToolBar::contextDestroyed(Kwave::FileContext *context)
{
    if (context != m_context) return; // not of interest
    contextSwitched(nullptr);
}

//***************************************************************************
void Kwave::ZoomToolBar::updateToolbar()
{
    bool have_signal    = false;
    bool have_selection = false;
    bool is_closed      = true;

    if (m_context) {
        Kwave::SignalManager *signal_manager = m_context->signalManager();
        Q_ASSERT(signal_manager);
        if (!signal_manager) return;
        have_signal    = (signal_manager->tracks() != 0);
        have_selection = (signal_manager->selection().length() != 0);
        is_closed      = signal_manager->isClosed();
    }

    if (m_action_zoomselection)
        m_action_zoomselection->setEnabled(have_signal  && have_selection);
    if (m_action_zoomin)
        m_action_zoomin->setEnabled(have_signal);
    if (m_action_zoomout)
        m_action_zoomout->setEnabled(have_signal);
    if (m_action_zoomnormal)
        m_action_zoomnormal->setEnabled(have_signal);
    if (m_action_zoomall)
        m_action_zoomall->setEnabled(have_signal);
    if (m_action_zoomselect)
        m_action_zoomselect->setEnabled(have_signal);

    if (m_zoomselect && is_closed) {
        for (int i = 0; i < m_zoomselect->count(); i++) {
            QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_PREDEFINED);
            if (!v.isValid() || !v.toBool()) {
                m_zoomselect->removeItem(i);
                break;
            }
        }
        m_zoomselect->insertItem(-1, _(" "));
        m_zoomselect->setCurrentIndex(0);
    }
}

//***************************************************************************
void Kwave::ZoomToolBar::selectZoom(int index)
{
    if (!m_context) return;

    Kwave::SignalManager *signal_manager = m_context->signalManager();
    Q_ASSERT(signal_manager);
    Q_ASSERT(m_zoomselect);
    if (!signal_manager) return;
    if (!m_zoomselect) return;
    if (index < 0) return;
    if (index >= m_zoomselect->count()) return;

    QVariant v = m_zoomselect->itemData(index, ZOOM_DATA_TIME);
    unsigned int ms = 1;
    bool ok = false;
    if (v.isValid()) ms = v.toUInt(&ok);
    if (!ok) ms = 1;

    Kwave::Zoomable *zoomable = m_context->zoomable();
    Q_ASSERT(zoomable);
    if (!zoomable) return;

    const double rate = signal_manager->rate();
    unsigned int width = zoomable->visibleWidth();
    Q_ASSERT(width > 1);
    if (width <= 1) width = 2;
    const double new_zoom = rint(((rate * ms) / 1.0E3) -1 ) /
        static_cast<double>(width - 1);
    zoomable->setZoom(new_zoom);

    // force the zoom factor to be set, maybe the current selection
    // has been changed/corrected to the previous value so that we
    // don't get a signal.
    setZoomInfo(m_context, zoomable->zoom());
}

//***************************************************************************
void Kwave::ZoomToolBar::setZoomInfo(Kwave::FileContext *context, double zoom)
{
    if (!m_context || (context != m_context)) return;

    Kwave::SignalManager *signal_manager = m_context->signalManager();
    Kwave::Zoomable *zoomable = m_context->zoomable();
    Q_ASSERT(zoom >= 0);
    Q_ASSERT(m_zoomselect);
    if (zoom <= 0.0) return; // makes no sense or signal is empty
    if (!m_zoomselect) return;

    double rate = (signal_manager) ? signal_manager->rate() : 0.0;
    double ms   = ((rate > 0) && (zoomable)) ?
        ((static_cast<double>(zoomable->visibleSamples()) * 1E3) / rate) : 0.0;

    QString strZoom;
    if ((signal_manager) && (signal_manager->tracks())) {
        if (rate > 0) {
            // time display mode
            strZoom = Kwave::ms2string(ms, 3);
        } else {
            // percent mode
            double percent = 100.0 / zoom;
            strZoom = Kwave::zoom2string(percent);
        }
    }

    m_zoomselect->blockSignals(true);

    // if the text is equal to an entry in the current list -> keep it
    if (m_zoomselect->contains(strZoom)) {
        // select existing entry, string match
        m_zoomselect->setCurrentIndex(m_zoomselect->findText(strZoom));
    } else {
        // remove user defined entries and scan for more or less exact match
        int i = 0;
        int match = -1;
        while (i < m_zoomselect->count()) {
            QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_PREDEFINED);
            if (!v.isValid() || !v.toBool()) {
                m_zoomselect->removeItem(i);
            } else {
                QVariant vz = m_zoomselect->itemData(i, ZOOM_DATA_TIME);
                bool ok = false;
                double t = vz.toDouble(&ok);
                if (ok && (t > 0) && fabs(1 - (t / ms)) < (1.0 / 60.0)) {
                    match = i;
                }
                i++;
            }
        }

        if (match >= 0) {
            // use an exact match from the list
            i = match;
        } else if (rate > 0) {
            // time mode:
            // find the best index where to insert the new user defined value
            for (i = 0; i < m_zoomselect->count(); ++i) {
                QVariant v = m_zoomselect->itemData(i, ZOOM_DATA_TIME);
                bool ok = false;
                double t = v.toDouble(&ok);
                if (!ok) continue;
                if (t > ms) break;
            }
            m_zoomselect->insertItem(i, strZoom);
            m_zoomselect->setItemData(i, QVariant(ms), ZOOM_DATA_TIME);
        } else {
            // percent mode -> just insert at top
            m_zoomselect->insertItem(-1, strZoom);
            i = 0;
        }
        m_zoomselect->setCurrentIndex(i);
    }

    m_zoomselect->blockSignals(false);
}

//***************************************************************************
//***************************************************************************

#include "moc_ZoomToolBar.cpp"
