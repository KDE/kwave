/***************************************************************************
         MainWidget.cpp  -  main widget of the Kwave TopWidget
                             -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
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

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include <limits>
#include <new>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPoint>
#include <QPointer>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextStream>
#include <QWheelEvent>
#include <QtGlobal>

#include <KLocalizedString>
#include <KMessageBox>

#include "libkwave/Drag.h"
#include "libkwave/FileDrag.h"
#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/Parser.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/FileDialog.h"
#include "libgui/LabelPropertiesWidget.h"
#include "libgui/OverViewWidget.h"
#include "libgui/SignalWidget.h"

#include "App.h"
#include "FileContext.h"
#include "MainWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == _(x)) {

/**
 * Limits the zoom to a minimum number of samples visible in one
 * screen.
 */
#define MINIMUM_SAMPLES_PER_SCREEN 5

/**
 * Default widht of the display in seconds when in streaming mode,
 * where no initial length information is available (guess: 5min)
 */
#define DEFAULT_DISPLAY_TIME (5 * 60.0)

/**
 * File extension used for label lists
 */
#define LABEL_LIST_EXT _("*.label")

/**
 * Filter for file dialogs for loading/saving labels
 */
#define LABEL_LIST_FILTER \
    LABEL_LIST_EXT + _("|") + \
    i18n("Kwave label list") + \
    _(" (") + LABEL_LIST_EXT + _(")")

//***************************************************************************
Kwave::MainWidget::MainWidget(QWidget *parent, Kwave::FileContext &context,
                              const QSize &preferred_size)
    :QWidget(parent),
     Kwave::CommandHandler(),
     Kwave::Zoomable(),
     m_context(context), m_upper_dock(), m_lower_dock(),
     m_scroll_area(this), m_horizontal_scrollbar(nullptr),
     m_signal_widget(
         &m_scroll_area,
         context.signalManager(),
         &m_upper_dock,
         &m_lower_dock
     ),
     m_overview(nullptr), m_offset(0), m_zoom(1.0),
     m_preferred_size(preferred_size),
     m_delayed_update_timer(this)
{
//    qDebug("MainWidget::MainWidget()");

    setAcceptDrops(true); // enable drag&drop

    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    Kwave::PluginManager *plugin_manager = context.pluginManager();
    Q_ASSERT(plugin_manager);
    if (!plugin_manager) return;
    plugin_manager->registerViewManager(&m_signal_widget);

    // topLayout:
    // - upper dock
    // - view port
    // - lower dock
    // - overview
    // - horizontal scroll bar
    QVBoxLayout *topLayout = new(std::nothrow) QVBoxLayout(this);
    Q_ASSERT(topLayout);
    if (!topLayout) return;

    topLayout->setSpacing(0);

    // -- upper dock --
    topLayout->addLayout(&m_upper_dock);

    // -- view port with signal widget --
    m_signal_widget.setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
    m_scroll_area.setFrameStyle(QFrame::NoFrame);
    m_scroll_area.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll_area.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll_area.setWidgetResizable(true);
    m_scroll_area.setWidget(&m_signal_widget);

    topLayout->addWidget(&m_scroll_area);

    // -- lower dock --
    topLayout->addLayout(&m_lower_dock);

    // -- overview widget --

    m_overview = new(std::nothrow) Kwave::OverViewWidget(*signal_manager, this);
    Q_ASSERT(m_overview);
    if (!m_overview) return;
    m_overview->setMinimumHeight(m_overview->sizeHint().height());
    m_overview->setSizePolicy(
        QSizePolicy::MinimumExpanding, QSizePolicy::Fixed
    );
    topLayout->addWidget(m_overview);
    connect(m_overview, SIGNAL(valueChanged(sample_index_t)),
            this,       SLOT(setOffset(sample_index_t)));
    connect(m_overview, SIGNAL(sigCommand(QString)),
            this,       SIGNAL(sigCommand(QString)));
    m_overview->metaDataChanged(signal_manager->metaData());
    m_overview->hide();

    // -- horizontal scrollbar --

    m_horizontal_scrollbar = new(std::nothrow) QScrollBar(this);
    Q_ASSERT(m_horizontal_scrollbar);
    if (!m_horizontal_scrollbar) return;
    m_horizontal_scrollbar->setOrientation(Qt::Horizontal);
    topLayout->addWidget(m_horizontal_scrollbar);
    connect(m_horizontal_scrollbar, SIGNAL(valueChanged(int)),
            this,                   SLOT(horizontalScrollBarMoved(int)));
    m_horizontal_scrollbar->hide();

    // -- playback position update --

    const Kwave::PlaybackController &playback =
        signal_manager->playbackController();
    connect(&playback, SIGNAL(sigPlaybackPos(sample_index_t)),
        m_overview, SLOT(showCursor(sample_index_t)));
    connect(&playback, SIGNAL(sigPlaybackStopped()),
        m_overview, SLOT(showCursor()));

    // -- connect all signals from/to the signal widget --

    connect(&m_signal_widget, SIGNAL(sigCommand(QString)),
            this,             SIGNAL(sigCommand(QString)));
    connect(&m_signal_widget, SIGNAL(sigCursorChanged(sample_index_t)),
            m_overview,       SLOT(showCursor(sample_index_t)));

    // -- connect all signals from/to the signal manager --

    connect(signal_manager,
            SIGNAL(sigTrackInserted(uint,Kwave::Track*)),
            this,
            SLOT(slotTrackInserted(uint,Kwave::Track*)));
    connect(signal_manager,
            SIGNAL(sigTrackDeleted(uint,Kwave::Track*)),
            this,
            SLOT(slotTrackDeleted(uint,Kwave::Track*)));
    connect(signal_manager,
            SIGNAL(sigMetaDataChanged(Kwave::MetaDataList)),
            this,
            SLOT(updateViewRange()));

    // set up the timer for delayed view range update
    m_delayed_update_timer.setSingleShot(true);
    connect(&m_delayed_update_timer, SIGNAL(timeout()),
            this, SLOT(updateViewRange()));

    setLayout(topLayout);
}

//***************************************************************************
bool Kwave::MainWidget::isOK()
{
    return (m_horizontal_scrollbar && m_overview);
}

//***************************************************************************
Kwave::MainWidget::~MainWidget()
{
    Kwave::PluginManager *plugin_manager = m_context.pluginManager();
    Q_ASSERT(plugin_manager);
    if (plugin_manager) plugin_manager->registerViewManager(nullptr);
}

//***************************************************************************
void Kwave::MainWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_context.isActive() &&
        (m_context.app().guiType() == Kwave::App::GUI_TAB))
    {
        // HACK: this is a workaround for stupid bug in Qt, which sends us a
        //       resize event with bogus size, when we are in tab mode, just
        //       before getting deactivated !?
        if (!m_delayed_update_timer.isActive())
            m_delayed_update_timer.start(0);
    } else {
        // update immediately
        updateViewRange();
    }
}

//***************************************************************************
void Kwave::MainWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) return;
    if ((event->proposedAction() != Qt::MoveAction) &&
        (event->proposedAction() != Qt::CopyAction))
        return; /* unsupported action */

    if (Kwave::FileDrag::canDecode(event->mimeData()))
        event->acceptProposedAction();
}

//***************************************************************************
void Kwave::MainWidget::dropEvent(QDropEvent *event)
{
    if (!event) return;
    if (!event->mimeData()) return;

    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    if (signal_manager->isEmpty() &&
        Kwave::Drag::canDecode(event->mimeData()))
    {
        sample_index_t pos = m_offset + pixels2samples(
            event->position().toPoint().x());
        sample_index_t len = 0;

        if ((len = Kwave::Drag::decode(this, event->mimeData(),
            *signal_manager, pos)))
        {
            // set selection to the new area where the drop was done
            signal_manager->selectRange(pos, len);
            event->acceptProposedAction();
        } else {
            QStringList formats = event->mimeData()->formats();
            QString fmt = (!formats.isEmpty()) ? formats.first() : QString();
            qWarning("MainWidget::dropEvent(%s): failed !", DBG(fmt));
            event->ignore();
        }
    }
}

//***************************************************************************
void Kwave::MainWidget::wheelEvent(QWheelEvent *event)
{
    if (!event || !m_overview) return;

    // process only wheel events on the signal and overview frame,
    // not on the channel controls or scrollbars
    if (!m_scroll_area.geometry().contains(event->position().toPoint()) &&
        !m_overview->geometry().contains(event->position().toPoint()) )
    {
        event->ignore();
        return;
    }

    switch (event->modifiers()) {
        case Qt::NoModifier: {
            // no modifier + <WheelUp/Down> => scroll left/right
            if (event->angleDelta().ry() > 0)
                executeCommand(_("view:scroll_left()"));
            else if (event->angleDelta().ry() < 0)
                executeCommand(_("view:scroll_right()"));
            event->accept();
            break;
        }
        case Qt::ShiftModifier:
            // <Shift> + <WheelUp/Down> => page up/down
            if (event->angleDelta().ry() > 0)
                executeCommand(_("view:scroll_prev()"));
            else if (event->angleDelta().ry() < 0)
                executeCommand(_("view:scroll_next()"));
            event->accept();
            break;
        case Qt::ControlModifier: {
            // <Ctrl> + <WheelUp/Down> => zoom in/out
            int x = qMax(m_signal_widget.mapToViewPort(
                event->globalPosition().toPoint()), 0);
            if (event->angleDelta().ry() > 0)
                executeCommand(_("view:zoom_in(%1)").arg(x));
            else if (event->angleDelta().ry() < 0)
                executeCommand(_("view:zoom_out(%1)").arg(x));
            event->accept();
            break;
        }
        default:
            event->ignore();
    }
}

//***************************************************************************
void Kwave::MainWidget::closeEvent(QCloseEvent *e)
{
    m_context.closeFile() ? e->accept() : e->ignore();
}

//***************************************************************************
void Kwave::MainWidget::slotTrackInserted(unsigned int index,
                                          Kwave::Track *track)
{
//     qDebug("MainWidget::slotTrackInserted(%u, %p)",
//         index, static_cast<void *>(track));
    Q_UNUSED(index)
    Q_UNUSED(track)

    // when the first track has been inserted, set some reasonable zoom
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    bool first_track = (signal_manager && (signal_manager->tracks() == 1));
    if (first_track)
        zoomAll();
    else
        updateViewRange();

}

//***************************************************************************
void Kwave::MainWidget::slotTrackDeleted(unsigned int index,
                                         Kwave::Track *track)
{
//     qDebug("MainWidget::slotTrackDeleted(%u)", index);
    Q_UNUSED(index)
    Q_UNUSED(track)

    updateViewRange();
}

//***************************************************************************
double Kwave::MainWidget::zoom() const
{
    return m_zoom;
}

//***************************************************************************
int Kwave::MainWidget::executeCommand(const QString &command)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return -EINVAL;
    if (!command.length()) return -EINVAL;

    Kwave::Parser parser(command);
    const sample_index_t visible_samples = visibleSamples();
    const sample_index_t signal_length   = signal_manager->length();

    if (false) {

    // -- zoom --
    CASE_COMMAND("view:zoom_selection")
        zoomSelection();
    CASE_COMMAND("view:zoom_in")
        int x = parser.hasParams() ? parser.toInt() : -1;
        zoomIn(x);
    CASE_COMMAND("view:zoom_out")
        int x = parser.hasParams() ? parser.toInt() : -1;
        zoomOut(x);
    CASE_COMMAND("view:zoom_normal")
        zoomNormal();
    CASE_COMMAND("view:zoom_all")
        zoomAll();

    // -- navigation --
    CASE_COMMAND("goto")
        sample_index_t offset = parser.toSampleIndex();
        setOffset((offset > (visible_samples / 2)) ?
                  (offset - (visible_samples / 2)) : 0);
        signal_manager->selectRange(offset, 0);
    CASE_COMMAND("view:scroll_right")
        const sample_index_t step = visible_samples / 10;
        setOffset(m_offset + step);
    CASE_COMMAND("view:scroll_left")
        const sample_index_t step = visible_samples / 10;
        setOffset((step < m_offset) ? (m_offset - step) : 0);
    CASE_COMMAND("view:scroll_start")
        setOffset(0);
        signal_manager->selectRange(0, 0);
    CASE_COMMAND("view:scroll_end")
        if (signal_length >= visible_samples)
            setOffset(signal_length - visible_samples);
    CASE_COMMAND("view:scroll_next")
        setOffset(m_offset + visible_samples);
    CASE_COMMAND("view:scroll_prev")
        setOffset((visible_samples < m_offset) ?
                  (m_offset - visible_samples) : 0);
    CASE_COMMAND("view:scroll_next_label")
        sample_index_t ofs =
            Kwave::LabelList(signal_manager->metaData()).nextLabelRight(
                m_offset + (visible_samples / 2));
        if (ofs > signal_length)
            ofs = signal_length - 1;
        setOffset((ofs > (visible_samples / 2)) ?
                  (ofs - (visible_samples / 2)) : 0);
    CASE_COMMAND("view:scroll_prev_label")
        sample_index_t ofs =
            Kwave::LabelList(signal_manager->metaData()).nextLabelLeft(
                m_offset + (visible_samples / 2));
        setOffset((ofs > (visible_samples / 2)) ?
                  (ofs - (visible_samples / 2)) : 0);
    // -- selection --
    CASE_COMMAND("selectall")
        signal_manager->selectRange(0, signal_manager->length());
    CASE_COMMAND("selectnext")
        if (signal_manager->selection().length())
            signal_manager->selectRange(signal_manager->selection().last() + 1,
                        signal_manager->selection().length());
        else
            signal_manager->selectRange(signal_manager->length() - 1, 0);
    CASE_COMMAND("selectprev")
        sample_index_t ofs = signal_manager->selection().first();
        sample_index_t len = signal_manager->selection().length();
        if (!len) len = 1;
        if (len > ofs) len = ofs;
        signal_manager->selectRange(ofs - len, len);
    CASE_COMMAND("selecttoleft")
        signal_manager->selectRange(0, signal_manager->selection().last() + 1);
    CASE_COMMAND("selecttoright")
        signal_manager->selectRange(signal_manager->selection().first(),
            signal_manager->length() - signal_manager->selection().first()
        );
    CASE_COMMAND("selectvisible")
        signal_manager->selectRange(m_offset, visibleSamples());
    CASE_COMMAND("selectnone")
        signal_manager->selectRange(m_offset, 0);

    // label handling
    CASE_COMMAND("label:add")
        sample_index_t pos = parser.toSampleIndex();
        if (!parser.isDone()) {
            // 2 parameters: position + description
            QString description = parser.nextParam();
            signal_manager->addLabel(pos, description);
        } else {
            // 1 parameter only: open dialog for editing the description
            addLabel(pos, QString());
        }
    CASE_COMMAND("label:edit")
        int index = parser.toInt();
        Kwave::LabelList labels(signal_manager->metaData());
        if ((index >= labels.count()) || (index < 0))
            return -EINVAL;
        Kwave::Label label = labels.at(index);
        labelProperties(label);
    CASE_COMMAND("label:load")
        QString filename = parser.nextParam();
        return loadLabels(filename);
   CASE_COMMAND("label:save")
        QString filename = parser.nextParam();
        return saveLabels(filename);

//    CASE_COMMAND("label:by_intensity")
//      markSignal(command);
//    CASE_COMMAND("label:to_pitch")
//      convertMarkstoPitch(command);
//    CASE_COMMAND("label:by_period")
//      markPeriods(command);

    } else
        return signal_manager->executeCommand(command);

    return 0;
}

//***************************************************************************
void Kwave::MainWidget::refreshHorizontalScrollBar()
{
    if (!m_horizontal_scrollbar || !m_context.signalManager()) return;

    m_horizontal_scrollbar->blockSignals(true);

    // show/hide the overview widget
    if (!m_context.signalManager()->isEmpty() && !m_overview->isVisible())
        m_overview->show();
    if (m_context.signalManager()->isEmpty() && m_overview->isVisible())
        m_overview->hide();

    // adjust the limits of the horizontal scrollbar
    if (m_context.signalManager()->length() > 1) {
        // get the view information in samples
        sample_index_t length  = m_context.signalManager()->length();
        sample_index_t visible = visibleSamples();
        if (visible > length) visible = length;

        // calculate the scrollbar ranges in scrollbar's units
        //
        // NOTE: we must take care of possible numeric overflows
        //       as the scrollbar internally works with "int" and
        //       the offsets we use for the samples might be bigger!
        //
        // [-------------------------------------------##############]
        // ^                                          ^     ^
        // min                                      max    page
        //
        // max + page = x | x < INT_MAX (!)
        //                                  x := length / f
        // page = x * (visible / length)  = visible  / f
        // max                            = length   / f - page
        // pos  = (m_offset / length) * x = m_offset / f

        const sample_index_t int_max = std::numeric_limits<int>::max();
        const sample_index_t f = qMax(
            sample_index_t(1),
            sample_index_t((length + int_max - 1) / int_max)
        );
        int page    = Kwave::toInt(visible  / f);
        int min     = 0;
        int max     = Kwave::toInt(length   / f) - page;
        int pos     = Kwave::toInt(m_offset / f);
        int single  = qMax(1, (page / (10 * qApp->wheelScrollLines())));
        if (page < single) page = single;
//      qDebug("width=%d, max=%d, page=%d, single=%d, pos=%d, visible=%d",
//             m_width, max, page, single, pos, visible);
//      qDebug("       last=%d", pos + visible - 1);
        m_horizontal_scrollbar->setRange(min, max);
        m_horizontal_scrollbar->setValue(pos);
        m_horizontal_scrollbar->setSingleStep(single);
        m_horizontal_scrollbar->setPageStep(page);
    } else {
        m_horizontal_scrollbar->setRange(0, 0);
    }

    m_horizontal_scrollbar->blockSignals(false);
}

//***************************************************************************
void Kwave::MainWidget::horizontalScrollBarMoved(int newval)
{
    // new offset = pos * f
    const sample_index_t length  = m_context.signalManager()->length();
    const sample_index_t int_max = std::numeric_limits<int>::max();
    const sample_index_t f = qMax(
        sample_index_t(1),
        sample_index_t((length + int_max - 1) / int_max)
    );
    const sample_index_t pos = newval * f;
    setOffset(pos);
}

//***************************************************************************
void Kwave::MainWidget::updateViewRange()
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    sample_index_t total = (signal_manager) ? signal_manager->length() : 0;

    if (m_overview) {
        m_overview->setRange(m_offset, visibleSamples(), total);

        // show/hide the overview widget and the horizontal scroll bar
        if (!m_context.signalManager()->isEmpty()) {
            if (!m_overview->isVisible())
                m_overview->show();
            if (!m_horizontal_scrollbar->isVisible())
                m_horizontal_scrollbar->show();
        }
        if (m_context.signalManager()->isEmpty()) {
            if (m_overview->isVisible())
                m_overview->hide();
            if (m_horizontal_scrollbar->isVisible())
                m_horizontal_scrollbar->hide();
        }
    }

    // forward the zoom and offset to the signal widget and overview
    fixZoomAndOffset(m_zoom, m_offset);
    m_signal_widget.setZoomAndOffset(m_zoom, m_offset);
    refreshHorizontalScrollBar();
}

//***************************************************************************
sample_index_t Kwave::MainWidget::ms2samples(double ms)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return 0;

    return static_cast<sample_index_t>(
        rint(ms * signal_manager->rate() / 1E3));
}

//***************************************************************************
sample_index_t Kwave::MainWidget::pixels2samples(unsigned int pixels) const
{
    if ((pixels == 0) || (m_zoom <= 0.0)) return 0;

    return static_cast<sample_index_t>(static_cast<double>(pixels) * m_zoom);
}

//***************************************************************************
int Kwave::MainWidget::samples2pixels(sample_index_t samples) const
{
    if (m_zoom <= 0.0) return 0;
    return Kwave::toInt(rint(static_cast<double>(samples) / m_zoom));
}

//***************************************************************************
int Kwave::MainWidget::visibleWidth() const
{
    return m_signal_widget.visibleWidth();
}

//***************************************************************************
sample_index_t Kwave::MainWidget::visibleSamples() const
{
    return pixels2samples(visibleWidth() - 1) + 1;
}

//***************************************************************************
double Kwave::MainWidget::fullZoom() const
{
    const int width = visibleWidth();
    if (width <= 0) return 0.0; // no zoom info, window is not yet ready

    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return 0.0;
    if (signal_manager->isEmpty()) return 0.0;    // no zoom if no signal

    sample_index_t length = signal_manager->length();
    if (!length) {
        // no length: streaming mode -> try to use "estimated length"
        // and add some extra, 10% should be ok
        Kwave::FileInfo info(signal_manager->metaData());
        if (info.contains(Kwave::INF_ESTIMATED_LENGTH)) {
            // estimated length in samples
            length = info.get(Kwave::INF_ESTIMATED_LENGTH).toULongLong();
            length += length / 10;
        }
        if (length <= 0) {
            // fallback: start with some default zoom,
            // use five minutes (just guessed)
            length = static_cast<sample_index_t>(ceil(DEFAULT_DISPLAY_TIME *
                signal_manager->rate()));
        }
    }
    if (!length) return 0; // still no length !? -> bail out

    // example: width = 100 [pixels] and length = 3 [samples]
    //          -> samples should be at positions 0, 49.5 and 99
    //          -> 49.5 [pixels / sample]
    //          -> zoom = 1 / 49.5 [samples / pixel]
    // => full zoom [samples/pixel] = (length - 1) / (width - 1)
    if (length < static_cast<sample_index_t>(width) * 2) {
        // zoom is small enough, no error compensation needed
        return (static_cast<double>(length) - 1.0) /
                static_cast<double>(width - 1);
    } else {
        // zoom is big, pre-compensate rounding errors
        return (static_cast<double>(length) - 0.5) /
                static_cast<double>(width - 1);
    }
}

//***************************************************************************
void Kwave::MainWidget::fixZoomAndOffset(double zoom, sample_index_t offset)
{
    double max_zoom;
    double min_zoom;
    sample_index_t length;

    const double         old_zoom   = m_zoom;
    const sample_index_t old_offset = m_offset;
    const int            width      = visibleWidth();

    m_zoom   = zoom;
    m_offset = offset;

    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;
    if (!width) return;

    length = signal_manager->length();
    if (!length) {
        // in streaming mode we have to use a guessed length
        length = static_cast<sample_index_t>(ceil(width * fullZoom()));
    }
    if (!length) {
        m_offset = 0;
        return;
    }

    // ensure that m_offset is [0...length-1]
    if (m_offset >= length) m_offset = (length - 1);

    // ensure that the zoom is in a reasonable range
    max_zoom = fullZoom();
    min_zoom = static_cast<double>(MINIMUM_SAMPLES_PER_SCREEN) /
               static_cast<double>(width);

    // ensure that pixel coordinates are within signed int range
    min_zoom = qMax(min_zoom, static_cast<double>(length) /
                              double(std::numeric_limits<int>::max()));

    if (m_zoom < min_zoom) m_zoom = min_zoom;
    if (m_zoom > max_zoom) m_zoom = max_zoom;

    // try to correct the offset if there is not enough data to fill
    // the current window
    // example: width=100 [pixels], length=3 [samples],
    //          offset=1 [sample], zoom=1/49.5 [samples/pixel] (full)
    //          -> current last displayed sample = length-offset
    //             = 3 - 1 = 2
    //          -> available space = pixels2samples(width-1) + 1
    //             = (99/49.5) + 1 = 3
    //          -> decrease offset by 3 - 2 = 1
    Q_ASSERT(length >= m_offset);
    if ((pixels2samples(width - 1) + 1) > (length - m_offset)) {
        // there is space after the signal -> move offset right
        sample_index_t shift = pixels2samples(width - 1) + 1 -
                               (length - m_offset);
        if (shift >= m_offset) {
            m_offset = 0;
        } else {
            m_offset -= shift;
        }
    }

    // emit change in the zoom factor
    if (!qFuzzyCompare(m_zoom, old_zoom)) emit sigZoomChanged(m_zoom);

    if ((m_offset != old_offset) || !qFuzzyCompare(m_zoom, old_zoom)) {
        emit sigVisibleRangeChanged(m_offset, visibleSamples(), length);
        updateViewRange();
    }
}

//***************************************************************************
void Kwave::MainWidget::setZoom(double new_zoom)
{
    fixZoomAndOffset(new_zoom, m_offset);
}

//***************************************************************************
void Kwave::MainWidget::setOffset(sample_index_t new_offset)
{
    fixZoomAndOffset(m_zoom, new_offset);
}

//***************************************************************************
void Kwave::MainWidget::scrollTo(sample_index_t pos)
{
    sample_index_t visible = visibleSamples();

    if ( (pos < (m_offset + (visible / 10))) ||
         (pos > (m_offset + (visible /  2))) )
    {
        // new position is out of range or too close to the border
        sample_index_t offset = (visible / 2);
        pos = (pos > offset) ? (pos - offset) : 0;
        if (pos != m_offset) {
            // check: is the difference >= 1 pixel ?
            sample_index_t diff = (pos > m_offset) ?
                (pos - m_offset) : (m_offset - pos);
            if (samples2pixels(diff) >= 1)
                setOffset(pos);
        }
    }
}

//***************************************************************************
void Kwave::MainWidget::zoomSelection()
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    sample_index_t ofs = signal_manager->selection().offset();
    sample_index_t len = signal_manager->selection().length();

    if (len) {
        int width = visibleWidth();
        m_offset = ofs;
        setZoom((static_cast<double>(len)) / static_cast<double>(width - 1));
    }
}

//***************************************************************************
void Kwave::MainWidget::zoomAll()
{
    setZoom(fullZoom());
}

//***************************************************************************
void Kwave::MainWidget::zoomNormal()
{
    const sample_index_t shift1 = visibleWidth() / 2;
    const sample_index_t shift2 = visibleSamples() / 2;
    m_offset = (m_offset + shift2 >= m_offset) ? (m_offset + shift2) : -shift2;
    m_offset = (m_offset > shift1) ? (m_offset - shift1) : 0;
    setZoom(1.0);
}

//***************************************************************************
void Kwave::MainWidget::zoomIn(int pos)
{
    if (pos >= 0) {
        // position given, calculate shift:
        // ofs_old + (pos * m_zoom) := ofs_new + (pos * (m_zoom / 3))
        // ofs_new = ofs_old + (pos * m_zoom) -  (pos * (m_zoom / 3))
        fixZoomAndOffset(
            m_zoom / 3,
            m_offset + Kwave::toInt(pos * (m_zoom * (2.0 / 3.0)))
        );
    } else {
        // no position given, show centered
        fixZoomAndOffset(
            m_zoom / 3,
            m_offset + (visibleSamples() / 3)
        );
    }
}

//***************************************************************************
void Kwave::MainWidget::zoomOut(int pos)
{
    sample_index_t shift;

    if (pos >= 0) {
        // position given, calculate shift
        // ofs_old + (pos * m_zoom) := ofs_new + (pos * (m_zoom * 3))
        // ofs_new = ofs_old + (pos * m_zoom) -  (pos * (m_zoom * 3))
        shift = Kwave::toInt(m_zoom * 2.0) * pos;
    } else {
        // no position given, show centered
        shift = visibleSamples();
    }
    fixZoomAndOffset(
        m_zoom * 3,
        (m_offset > shift) ? (m_offset - shift) : 0
    );
}

//***************************************************************************
void Kwave::MainWidget::addLabel(sample_index_t pos, const QString &description)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    // remember the last "modified" state, in case that we abort
    bool was_modified = signal_manager->isModified();

    // create an own undo transaction
    Kwave::UndoTransactionGuard undo(*signal_manager, i18n("Add Label"));

    // add a new label, with undo
    Kwave::Label label = signal_manager->addLabel(pos, description);
    if (label.isNull()) {
        signal_manager->abortUndoTransaction();
        if (!was_modified) signal_manager->setModified(false);
        return;
    }

    // edit the properties of the new label
    if (!labelProperties(label)) {
        // aborted or failed -> delete (without undo)
        int index = signal_manager->labelIndex(label);
        signal_manager->deleteLabel(index, false);
        signal_manager->abortUndoTransaction();
        if (!was_modified) signal_manager->setModified(false);
    }
}

//****************************************************************************
int Kwave::MainWidget::loadLabels(const QString &filename)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return -1;

    QUrl url;
    if (!filename.length()) {
        QPointer<Kwave::FileDialog> dlg = new (std::nothrow)Kwave::FileDialog(
        _("kfiledialog:///kwave_label_dir"),
        Kwave::FileDialog::OpenFile, LABEL_LIST_FILTER, this,
        QUrl(), LABEL_LIST_EXT);
        if (!dlg) return -1;
        dlg->setWindowTitle(i18n("Load Labels"));
        if (dlg->exec() != QDialog::Accepted) {
            delete dlg;
            return 0;
        } else {
            url = dlg->selectedUrl();
            delete dlg;
        }
    } else {
        url = Kwave::URLfromUserInput(filename);
    }

    // create an own undo transaction
    Kwave::UndoTransactionGuard undo(*signal_manager, i18n("Load Labels"));

    return m_context.loadBatch(url);
}

//****************************************************************************
int Kwave::MainWidget::saveLabels(const QString &filename)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return false;

    QUrl url;
    url = QUrl(m_context.signalName());

    if (!filename.length()) {
        QString name(filename);

        QPointer<Kwave::FileDialog> dlg = new(std::nothrow)Kwave::FileDialog(
            _("kfiledialog:///kwave_label_dir"),
            Kwave::FileDialog::SaveFile, LABEL_LIST_FILTER,
            this, url, LABEL_LIST_EXT);
        if (!dlg) return 0;
        dlg->setWindowTitle(i18n("Save Labels"));
        if (dlg->exec() != QDialog::Accepted) {
            delete dlg;
            return -1;
        }
        url = dlg->selectedUrl();
        if (url.isEmpty()) {
            delete dlg;
            return 0;
        }
        delete dlg;

        // add an extension if necessary
        name = url.path();
        if (!QFileInfo(name).suffix().length()) {
            url.setPath(name + _(".label"));
            name = url.path();
        }

        // check if the file exists and ask before overwriting it
        // if it is not the old filename
        if (QFileInfo::exists(name))
        {
            if (Kwave::MessageBox::warningYesNo(this,
                i18n("The file '%1' already exists.\n"
                "Do you really want to overwrite it?", name)) !=
                KMessageBox::PrimaryAction)
            {
                return -1;
            }
        }
    } else {
        url = Kwave::URLfromUserInput(filename);
    }

    // now we have a file name -> save all labels...
    Kwave::Logger::log(this, Kwave::Logger::Info,
        _("saving labels to '") + url.toDisplayString() + _("'"));

    QFile file(url.path());
    if (!file.open(QIODevice::WriteOnly)) return -1;
    QTextStream out(&file);

    Kwave::LabelList labels(signal_manager->metaData());
    foreach (const Kwave::Label &label, labels) {
        sample_index_t pos = label.pos();
        const QString name = Kwave::Parser::escape(label.name());
        out << _("label:add(") << pos;
        if (name.length()) out << _(", ") << name;
        out << _(")") << Qt::endl;
    }

    file.close();
    return 0;
}

//***************************************************************************
bool Kwave::MainWidget::labelProperties(Kwave::Label &label)
{
    Kwave::SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return false;

    if (label.isNull()) return false;
    int index = signal_manager->labelIndex(label);
    Q_ASSERT(index >= 0);
    if (index < 0) return false;

    // try to modify the label. just in case that the user moves it
    // to a position where we already have one, catch this situation
    // and ask if he wants to abort, re-enter the label properties
    // dialog or just replace (remove) the label at the target position
    bool accepted;
    sample_index_t new_pos  = label.pos();
    QString        new_name = label.name();
    int old_index = -1;
    while (true) {
        // create and prepare the dialog
        Kwave::LabelPropertiesWidget *dlg =
            new(std::nothrow) Kwave::LabelPropertiesWidget(this);
        Q_ASSERT(dlg);
        if (!dlg) return false;
        dlg->setLabelIndex(index);
        dlg->setLabelPosition(new_pos, signal_manager->length(),
            signal_manager->rate());
        dlg->setLabelName(new_name);

        // execute the dialog
        accepted = (dlg->exec() == QDialog::Accepted);
        if (!accepted) {
            // user pressed "cancel"
            delete dlg;
            break;
        }

        // if we get here the user pressed "OK"
        new_pos  = dlg->labelPosition();
        new_name = dlg->labelName();
        dlg->saveSettings();
        delete dlg;

        // check: if there already is a label at the new position
        // -> ask the user if he wants to overwrite that one
        if ((new_pos != label.pos()) &&
            !signal_manager->findLabel(new_pos).isNull())
        {
            int res = Kwave::MessageBox::warningYesNoCancel(this, i18n(
                "There already is a label at the position you have chosen.\n"\
                "Do you want to replace it?"));
            if (res == KMessageBox::PrimaryAction) {
                // delete the label at the target position (with undo)
                Kwave::Label old = signal_manager->findLabel(new_pos);
                old_index = signal_manager->labelIndex(old);
                break;
            }
            if (res == KMessageBox::SecondaryAction) {
                // make another try -> re-enter the dialog
                continue;
            }

            // cancel -> abort the whole action
            accepted = false;
            break;
        } else {
            // ok, we can put it there
            break;
        }
    }

    if (accepted) {
        // shortcut: abort if nothing has changed
        if ((new_name == label.name()) && (new_pos == label.pos()))
            return true;

        Kwave::UndoTransactionGuard undo(*signal_manager, i18n("Modify Label"));

        // if there is a label at the target position, remove it first
        if (old_index >= 0) {
            signal_manager->deleteLabel(old_index, true);
            // this might have changed the current index!
            index = signal_manager->labelIndex(label);
        }

        // modify the label through the signal manager
        if (!signal_manager->modifyLabel(index, new_pos, new_name, true)) {
            // position is already occupied
            signal_manager->abortUndoTransaction();
            return false;
        }

        // reflect the change in the passed label
        label.moveTo(new_pos);
        label.rename(new_name);

        // NOTE: moving might also change the index, so the complete
        //       markers layer has to be refreshed
    }
    else
        signal_manager->abortUndoTransaction();

    return accepted;
}

////****************************************************************************
//void MainWidget::markSignal (const char *str)
//{
//    if (signalmanage) {
//   Kwave::Label *newmark;
//
//   Kwave::Parser parser (str);
//
//   int level = (int) (parser.toDouble() / 100 * (1 << 23));
//
//   int len = signalmanage->getLength();
//   int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
//   LabelType *start = findMarkerType(parser.getNextParam());
//   LabelType *stop = findMarkerType (parser.getNextParam());
//   int time = (int) (parser.toDouble () * signalmanage->getRate() / 1000);
//
//   printf ("%d %d\n", level, time);
//   printf ("%s %s\n", start->name, stop->name);
//
//   ProgressDialog *dialog =
//       new ProgressDialog (len, "Searching for Signal portions...");
//
//   if (dialog && start && stop) {
//       dialog->show();
//
//       newmark = new Kwave::Label(0, start); // generate initial label
//
//       labels->inSort (newmark);
//
//       for (int i = 0; i < len; i++) {
//           if (qAbs(sam[i]) < level) {
//               int j = i;
//               while ((i < len) && (qAbs(sam[i]) < level)) i++;
//
//               if (i - j > time) {
//                   //insert labels...
//                   newmark = new Kwave::Label(i, start);
//                   labels->inSort (newmark);
//
//                   if (start != stop) {
//                       newmark = new Kwave::Label(j, stop);
//                       labels->inSort (newmark);
//                   }
//               }
//           }
//           dialog->setProgress (i);
//       }
//
//       newmark = new Kwave::Label(len - 1, stop);
//       labels->inSort (newmark);
//
//       refresh ();
//       delete dialog;
//   }
//    }
//}

////****************************************************************************
//void MainWidget::markPeriods (const char *str)
//{
//    if (signalmanage) {
//   Kwave::Parser parser (str);
//
//   int high = signalmanage->getRate() / parser.toInt();
//   int low = signalmanage->getRate() / parser.toInt();
//   int octave = parser.toBool ("true");
//   double adjust = parser.toDouble ();
//
//   generate static weighting function
//   for (int i = 0; i < AUTOKORRWIN; i++)
//       autotable[i] = 1 - (((double)i * i * i) / (
//           AUTOKORRWIN * AUTOKORRWIN * AUTOKORRWIN));
//
//   if (octave)
//         for (int i = 0; i < AUTOKORRWIN; i++)
//             weighttable[i] = 1;    //initialise moving weight table
//
//   Kwave::Label *newmark;
//   int next;
//   int len = signalmanage->getLength();
//   int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
//   LabelType *start = markertype;
//   int cnt = findFirstMark (sam, len);
//
//   ProgressDialog *dialog = new ProgressDialog (len - AUTOKORRWIN,
//       "Correlating Signal to find Periods:");
//   if (dialog) {
//       dialog->show();
//
//       newmark = new Kwave::Label(cnt, start);
//       labels->inSort (newmark);
//
//       while (cnt < len - 2*AUTOKORRWIN) {
//           if (octave)
//               next = findNextRepeatOctave (&sam[cnt], high, adjust);
//           else
//               next = findNextRepeat (&sam[cnt], high);
//
//           if ((next < low) && (next > high)) {
//               newmark = new Kwave::Label(cnt, start);
//
//               labels->inSort (newmark);
//           }
//           if (next < AUTOKORRWIN) cnt += next;
//           else
//               if (cnt < len - AUTOKORRWIN) {
//                   int a = findFirstMark (&sam[cnt], len - cnt);
//                   if (a > 0) cnt += a;
//                   else cnt += high;
//               } else cnt = len;
//
//           dialog->setProgress (cnt);
//       }
//
//       delete dialog;
//
//       refresh ();
//   }
//    }
//}

//*****************************************************************************
//int findNextRepeat (int *sample, int high)
////autocorellation of a windowed part of the sample
////returns length of period, if found
//{
//    int i, j;
//    double gmax = 0, max, c;
//    int maxpos = AUTOKORRWIN;
//    int down, up;         //flags
//
//    max = 0;
//    for (j = 0; j < AUTOKORRWIN; j++)
//   gmax += ((double)sample[j]) * sample [j];
//
//    //correlate signal with itself for finding maximum integral
//
//    down = 0;
//    up = 0;
//    i = high;
//    max = 0;
//    while (i < AUTOKORRWIN) {
//   c = 0;
//   for (j = 0; j < AUTOKORRWIN; j++)
//       c += ((double)sample[j]) * sample [i + j];
//   // multiply window with weight for preference of high frequencies
//   c = c * autotable[i];
//   if (c > max) max = c, maxpos = i;
//   i++;
//    }
//    return maxpos;
//}

//*****************************************************************************
//int findNextRepeatOctave (int *sample, int high, double adjust = 1.005)
// // autocorellation of a windowed part of the sample
// // same as above only with an adaptive weighting to decrease fast
// // period changes
//{
//    int i, j;
//    double gmax = 0, max, c;
//    int maxpos = AUTOKORRWIN;
//    int down, up;         //flags
//
//    max = 0;
//    for (j = 0; j < AUTOKORRWIN; j++)
//   gmax += ((double)sample[j]) * sample [j];
//
//    //correlate signal with itself for finding maximum integral
//
//    down = 0;
//    up = 0;
//    i = high;
//    max = 0;
//    while (i < AUTOKORRWIN) {
//   c = 0;
//   for (j = 0; j < AUTOKORRWIN; j++)
//       c += ((double)sample[j]) * sample[i + j];
//   c = c * autotable[i] * weighttable[i];
//   //multiply window with weight for preference of high frequencies
//   if (c > max) max = c, maxpos = i;
//   i++;
//    }
//
//    for (int i = 0; i < AUTOKORRWIN; i++) weighttable[i] /= adjust;
//
//    weighttable[maxpos] = 1;
//    weighttable[maxpos + 1] = .9;
//    weighttable[maxpos - 1] = .9;
//    weighttable[maxpos + 2] = .8;
//    weighttable[maxpos - 2] = .8;
//
//    float buf[7];
//
//    for (int i = 0; i < 7; buf[i++] = .1)
//
//   //low pass filter
//   for (int i = high; i < AUTOKORRWIN - 3; i++) {
//       buf[i % 7] = weighttable[i + 3];
//       weighttable[i] = (buf[0] + buf[1] + buf[2] + buf[3] +
//                         buf[4] + buf[5] + buf[6]) / 7;
//   }
//
//    return maxpos;
//}

//*****************************************************************************
//int findFirstMark (int *sample, int len)
// finds first sample that is non-zero, or one that preceeds a zero crossing
//{
//    int i = 1;
//    int last = sample[0];
//    int act = last;
//    if ((last < 100) && (last > -100)) i = 0;
//    else
//   while (i < len) {
//       act = sample[i];
//       if ((act < 0) && (last >= 0)) break;
//       if ((act > 0) && (last <= 0)) break;
//       last = act;
//       i++;
//   }
//    return i;
//}

//***************************************************************************
//***************************************************************************

#include "moc_MainWidget.cpp"
