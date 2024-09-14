/***************************************************************************
    SignalView.cpp  -  base class for widgets for views to a signal
                             -------------------
    begin                : Mon Jan 18 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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


#include <math.h>
#include <new>

#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <QToolTip>
#include <QUrl>

#include "libkwave/CodecManager.h"
#include "libkwave/Drag.h"
#include "libkwave/FileDrag.h"
#include "libkwave/Parser.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SignalView.h"

/** number of milliseconds until the position widget disappears */
#define POSITION_WIDGET_TIME 5000

//***************************************************************************
//***************************************************************************
Kwave::SignalView::SignalView(QWidget *parent, QWidget *controls,
                              Kwave::SignalManager *signal_manager,
                              Location preferred_location,
                              int track)
    :QWidget(parent),
     m_controls(controls),
     m_signal_manager(signal_manager),
     m_preferred_location(preferred_location),
     m_track_index(track),
     m_offset(0),
     m_zoom(1.0),
     m_vertical_zoom(1.0),
     m_mouse_mode(MouseNormal),
     m_mouse_selection(),
     m_mouse_down_x(0),
     m_position_widget(this),
     m_position_widget_timer(this),
     m_siblings(),
     m_selected_item(nullptr)
{
    // connect the timer of the position widget
    connect(&m_position_widget_timer, SIGNAL(timeout()),
            &m_position_widget, SLOT(hide()));

    setMouseTracking(true);
    setAcceptDrops(true);            // enable drag&drop
    setFocusPolicy(Qt::StrongFocus); // enabled keyboard events
}

//***************************************************************************
Kwave::SignalView::~SignalView()
{
    m_mouse_mode = MouseNormal;
    setCursor(Qt::ArrowCursor);
    hidePosition();
    m_selected_item.clear();

    if (!m_siblings.isEmpty()) {
        QMutableListIterator<QPointer<QWidget> > it(m_siblings);
        it.toBack();
        while (it.hasPrevious()) {
            QWidget *widget = it.previous();
            it.remove();
            delete widget;
        }
    }
}

//***************************************************************************
void Kwave::SignalView::refresh()
{
    repaint();
}

//***************************************************************************
void Kwave::SignalView::addSibling(QWidget *widget)
{
    m_siblings.append(QPointer<QWidget>(widget));
}

//***************************************************************************
void Kwave::SignalView::setTrack(int track)
{
    m_track_index = (track >= 0) ? track : -1;
}

//***************************************************************************
void Kwave::SignalView::setZoomAndOffset(double zoom, sample_index_t offset)
{
    if (qFuzzyCompare(zoom, m_zoom) && (offset == m_offset)) return;
    m_zoom   = zoom;
    m_offset = offset;

//     sample_index_t visible = ((width() - 1) * zoom) + 1;
//     sample_index_t last = offset + visible - 1;
//     qDebug("SignalView::setZoomAndOffset(%g, %lu), last visible=%lu",
//         zoom,
//         static_cast<unsigned long int>(offset),
//         static_cast<unsigned long int>(last));

    // the relation to the position widget has become invalid
    hidePosition();
}

//***************************************************************************
void Kwave::SignalView::setVerticalZoom(double zoom)
{
    m_vertical_zoom = zoom;
}

//***************************************************************************
void Kwave::SignalView::showCursor(sample_index_t pos)
{
    Q_UNUSED(pos)
}

//***************************************************************************
int Kwave::SignalView::samples2pixels(sample_index_t samples) const
{
    return Kwave::toInt((m_zoom > 0.0) ?
        (static_cast<double>(samples) / m_zoom) : 0);
}

//***************************************************************************
sample_index_t Kwave::SignalView::pixels2samples(int pixels) const
{
    if ((pixels <= 0) || (m_zoom <= 0.0)) return 0;
    return static_cast<sample_index_t>(static_cast<double>(pixels) * m_zoom);
}

//***************************************************************************
double Kwave::SignalView::samples2ms(sample_index_t samples)
{
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return 0.0;

    double rate = m_signal_manager->rate();
    if (rate == 0.0) return 0.0;
    return static_cast<double>(samples) * 1E3 / rate;
}

//***************************************************************************
Kwave::SignalView::SelectionPos Kwave::SignalView::selectionPosition(int x)
{
    // shortcut: if this view can't handle selection...
    if (!canHandleSelection()) return None;

    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return None;

    const Kwave::Selection &sel = m_signal_manager->selection();
    const double p     = (m_zoom * x) + static_cast<double>(m_offset);
    const double tol   = m_zoom * selectionTolerance();
    const double first = static_cast<double>(sel.first());
    const double last  = static_cast<double>(sel.last());
    Q_ASSERT(first <= last);

    // get distance to left/right selection border
    double d_left  = (p > first) ? (p - first) : (first - p);
    double d_right = (p > last)  ? (p - last)  : (last  - p);

    // the simple cases...
    Kwave::SignalView::SelectionPos pos = None;
    if ((d_left  <= tol) && (p < last))  pos |= LeftBorder;
    if ((d_right <= tol) && (p > first)) pos |= RightBorder;
    if ((p >= first) && (p <= last))     pos |= Selection;

    if ((pos & LeftBorder) && (pos & RightBorder)) {
        // special case: determine which border is nearer
        if (d_left < d_right)
            pos &= ~RightBorder; // more on the left
        else
            pos &= ~LeftBorder;  // more on the right
    }

    return pos;
}

//***************************************************************************
bool Kwave::SignalView::isInSelection(int x)
{
    return (selectionPosition(x) & Selection) != 0;
}

//***************************************************************************
QSharedPointer<Kwave::ViewItem> Kwave::SignalView::findItem(const QPoint &pos)
{
    Q_UNUSED(pos)
    return QSharedPointer<Kwave::ViewItem>(nullptr);
}

//***************************************************************************
void Kwave::SignalView::showPosition(const QString &text, sample_index_t pos,
                                     const QPoint &mouse)
{
    // check: showPosition() must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    int x = mouse.x();
    int y = mouse.y();

    // x/y == -1/-1 or empty text -> reset/hide the position
    if (((x < 0) && (y < 0)) || !text.length()) {
        m_position_widget_timer.stop();
        m_position_widget.hide();
        return;
    }

    setUpdatesEnabled(false);
    m_position_widget.hide();

    switch (selectionPosition(mouse.x()) & ~Selection) {
        case LeftBorder:
            m_position_widget.setText(text, Qt::AlignRight);
            x = samples2pixels(pos - m_offset) - m_position_widget.width();
            if (x < 0) {
                // switch to left aligned mode
                m_position_widget.setText(text, Qt::AlignLeft);
                x = samples2pixels(pos - m_offset);
            }
            break;
        case RightBorder:
        default:
            m_position_widget.setText(text, Qt::AlignLeft);
            x = samples2pixels(pos - m_offset);
            if (x + m_position_widget.width() > width()) {
                // switch to right aligned mode
                m_position_widget.setText(text, Qt::AlignRight);
                x = samples2pixels(pos - m_offset) - m_position_widget.width();
            }
            break;
    }

    // adjust the position to avoid vertical clipping
    int lh = m_position_widget.height();
    if (y - lh/2 < 0) {
        y = 0;
    } else if (y + lh/2 > height()) {
        y = height() - lh;
    } else {
        y -= lh/2;
    }

    m_position_widget.move(x, y);

    if (!m_position_widget.isVisible())
        m_position_widget.show();

    m_position_widget_timer.stop();
    m_position_widget_timer.setSingleShot(true);
    m_position_widget_timer.start(POSITION_WIDGET_TIME);

    setUpdatesEnabled(true);
}

//***************************************************************************
void Kwave::SignalView::findNewItem(const QPoint &mouse_pos, bool active)
{
    m_selected_item = QSharedPointer<Kwave::ViewItem>(nullptr);
    if (!active) m_mouse_mode = MouseNormal;

    m_selected_item = findItem(mouse_pos);
    if (m_selected_item) {
        // we have an item to show, activate the position window
        sample_index_t item_pos  = m_offset + pixels2samples(mouse_pos.x());
        QString        item_text = m_selected_item->toolTip(item_pos);
        showPosition(item_text, item_pos, mouse_pos);

        // update the mouse cursor
        Kwave::ViewItem::Flags flags = m_selected_item->flags();
        if (flags & Kwave::ViewItem::CanDragAndDrop)
            setCursor(active ? Qt::ClosedHandCursor : Qt::ArrowCursor);
        else if (flags & Kwave::ViewItem::CanGrabAndMove)
            setCursor(m_selected_item->mouseCursor());
        else
            setCursor(Qt::ArrowCursor);

        if (active) m_mouse_mode = MouseMoveItem;
    } else {
        // out of scope
        hidePosition();
        m_mouse_mode = MouseNormal;
        setCursor(Qt::ArrowCursor);
    }
}

//***************************************************************************
void Kwave::SignalView::mouseMoveEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;

    // abort if no signal is loaded
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager || !m_signal_manager->length()) {
        e->ignore();
        return;
    }

    // there seems to be a BUG in Qt, sometimes e->pos() produces flicker/wrong
    // coordinates on the start of a fast move!?
    // globalPos() seems not to have this effect
    const QPoint mouse_pos(
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).x(), width()  - 1),
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).y(), height() - 1)
    );

    // bail out if the position did not change
    static QPoint last_pos = QPoint(-1, -1);
    if (mouse_pos == last_pos) {
        e->ignore();
        return;
    }

    switch (m_mouse_mode) {
        case MouseDragItem: /* FALLTHROUGH */
        case MouseMoveItem: {
            // move mode

            if (m_selected_item.isNull()) {
                hidePosition();
                break;
            }

            bool hide_position = true;
            Kwave::ViewItem::Flags flags = m_selected_item->flags();
            if (flags & Kwave::ViewItem::CanDragAndDrop) {
                const int dmin = QApplication::startDragDistance();
                if ((e->buttons() & Qt::LeftButton) &&
                    ((mouse_pos.x() < (m_mouse_down_x - dmin)) ||
                     (mouse_pos.x() > (m_mouse_down_x + dmin))) )
                {
                    m_selected_item->startDragging();
                }
            }

            if (flags & Kwave::ViewItem::CanGrabAndMove) {
                // update the position of the item
                m_selected_item->moveTo(mouse_pos);

                // show the position window, so that we see the coordinates
                // where we move the item to
                sample_index_t pos  = m_offset + pixels2samples(mouse_pos.x());
                QString item_text = m_selected_item->toolTip(pos);
                showPosition(item_text, pos, mouse_pos);
                hide_position = false;
            }

            if (hide_position) hidePosition();
            break;
        }
        case MouseNormal: /* FALLTHROUGH */
        default:
            findNewItem(mouse_pos, false);
            break;
    }
    e->accept();
}

//***************************************************************************
void Kwave::SignalView::mousePressEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_signal_manager);
    if (!e) return;

    // abort if no signal is loaded
    if (!m_signal_manager || !m_signal_manager->length()) {
        e->ignore();
        return;
    }

    // ignore all mouse press events in playback mode
    if (m_signal_manager->playbackController().running()) {
        e->ignore();
        return;
    }

    const QPoint mouse_pos(
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).x(), width()  - 1),
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).y(), height() - 1)
    );

    findNewItem(mouse_pos, false);

    if (e->button() == Qt::LeftButton) {
        sample_index_t ofs = m_offset + pixels2samples(mouse_pos.x());
        sample_index_t selection_first = m_signal_manager->selection().first();
        sample_index_t selection_last  = m_signal_manager->selection().last();

        switch (e->modifiers()) {
            case Qt::ShiftModifier: {
                // expand the selection to "here"
                m_mouse_selection.set(selection_first, selection_last);
                m_mouse_selection.grep(ofs);
                m_signal_manager->selectRange(
                    m_mouse_selection.left(),
                    m_mouse_selection.length()
                );

                // this probably changes to "adjust selection border"
                findNewItem(mouse_pos, true);
                break;
            }
            case Qt::NoModifier: {
                // check whether there is some object near this position
                if (m_selected_item) {
                    // we have an item here:
                    m_mouse_mode = MouseMoveItem;

                    Kwave::ViewItem::Flags flags = m_selected_item->flags();
                    if (flags & Kwave::ViewItem::CanDragAndDrop) {
                        // store the x position for later drag&drop
                        m_mouse_down_x = mouse_pos.x();
                        setCursor(Qt::DragMoveCursor);
                    }
                } else if (canHandleSelection()) {
                    // start a new selection
                    m_mouse_selection.set(ofs, ofs);
                    m_signal_manager->selectRange(ofs, 0);

                    // this probably changes to "adjust selection border"
                    findNewItem(mouse_pos, true);
                }
                break;
            }
            default:
                break;
        }
    }
    e->accept();
}

//***************************************************************************
void Kwave::SignalView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_signal_manager);
    if (!e) return;

    // abort if no signal is loaded
    if (!m_signal_manager || !m_signal_manager->length()) {
        e->ignore();
        return;
    }

    // ignore all mouse release events in playback mode
    if (m_signal_manager->playbackController().running()) {
        e->ignore();
        return;
    }

    const QPoint mouse_pos(
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).x(), width()  - 1),
        qBound(0, mapFromGlobal(e->globalPosition().toPoint()).y(), height() - 1)
    );

    switch (m_mouse_mode) {
        case MouseDragItem:
            // released after dragging
            if (m_selected_item) m_selected_item->done();
            findNewItem(mouse_pos, false);
            break;
        case MouseMoveItem:
            if (m_selected_item) {
                // released after move
                if (m_selected_item->flags() & Kwave::ViewItem::CanGrabAndMove)
                    m_selected_item->done();
                else if (canHandleSelection()) {
                    // maybe started dragging, but released before reaching
                    // the minimum drag distance -> start a new selection
                    sample_index_t ofs = m_offset +
                                         pixels2samples(mouse_pos.x());
                    m_mouse_selection.set(ofs, ofs);
                    m_signal_manager->selectRange(ofs, 0);
                }
            }
            findNewItem(mouse_pos, false);
            break;
        default:
            break;
    }

    e->accept();
}

//***************************************************************************
void Kwave::SignalView::leaveEvent(QEvent *e)
{
    m_mouse_mode = MouseNormal;
    setCursor(Qt::ArrowCursor);
    hidePosition();
    QWidget::leaveEvent(e);
}

//***************************************************************************
void Kwave::SignalView::keyPressEvent(QKeyEvent *e)
{
    if (!e) return;
    if (e->matches(QKeySequence::Cancel)) {
        // Cancel key (Escape) -> reset all view item operations
        m_mouse_mode = MouseNormal;
        setCursor(Qt::ArrowCursor);
        m_selected_item = QSharedPointer<Kwave::ViewItem>(nullptr);
    } else {
        QWidget::keyPressEvent(e);
    }
    hidePosition();
}

//***************************************************************************
int Kwave::SignalView::selectionTolerance() const
{
    return (2 * QApplication::startDragDistance());
}

//***************************************************************************
void Kwave::SignalView::handleContextMenu(const QPoint &pos, QMenu *menu)
{
    Q_UNUSED(pos)
    Q_UNUSED(menu)
}

//***************************************************************************
void Kwave::SignalView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) return;
    if ((event->proposedAction() != Qt::MoveAction) &&
        (event->proposedAction() != Qt::CopyAction))
        return; /* unsupported action */

    if (Kwave::FileDrag::canDecode(event->mimeData()))
        event->acceptProposedAction();
    emit sigCursorChanged(SAMPLE_INDEX_MAX);
}

//***************************************************************************
void Kwave::SignalView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    m_mouse_mode = MouseNormal;
    setCursor(Qt::ArrowCursor);
    emit sigCursorChanged(SAMPLE_INDEX_MAX);
}

//***************************************************************************
void Kwave::SignalView::dropEvent(QDropEvent *event)
{
    if (!event) return;
    const QMimeData *mime_data = event->mimeData();
    if (!mime_data) return;
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return;

    if (Kwave::Drag::canDecode(mime_data)) {
        Kwave::UndoTransactionGuard undo(*m_signal_manager,
                                         i18n("Drag and Drop"));
        sample_index_t pos = m_offset + pixels2samples(event->position().toPoint().x());
        sample_index_t len = 0;

        if ((len = Kwave::Drag::decode(this, mime_data,
            *m_signal_manager, pos)))
        {
            // set selection to the new area where the drop was done
            m_signal_manager->selectRange(pos, len);
            event->acceptProposedAction();
        } else {
            QStringList formats = mime_data->formats();
            qWarning("SignalView::dropEvent(%s): failed !",
                DBG(formats.join(_("; "))));
        }
    } else if (mime_data->hasUrls()) {
        bool first = true;
        foreach (const QUrl &url, mime_data->urls()) {
            QString filename = url.toLocalFile();
            QString mimetype = Kwave::CodecManager::mimeTypeOf(url);
            if (Kwave::CodecManager::canDecode(mimetype)) {
                if (first) {
                    // first dropped URL -> open in this window
                    emit sigCommand(_("open(") +
                                    Kwave::Parser::escape(filename) +
                                    _(")"));
                    first = false;
                    event->acceptProposedAction();
                } else {
                    // all others -> open a new window
                    emit sigCommand(_("newwindow(") +
                                    Kwave::Parser::escape(filename) +
                                    _(")"));
                }
            }
        }
    }

    qDebug("SignalView::dropEvent(): done");
    m_mouse_mode = MouseNormal;
    setCursor(Qt::ArrowCursor);
    emit sigCursorChanged(SAMPLE_INDEX_MAX);

    if (!event->isAccepted()) event->ignore();
}

//***************************************************************************
void Kwave::SignalView::dragMoveEvent(QDragMoveEvent *event)
{
    if (!event) return;
    const int x = event->position().toPoint().x();

    if ((event->source() == this) && isInSelection(x)) {
        // disable drag&drop into the selection itself
        // this would be nonsense

        Q_ASSERT(m_signal_manager);
        if (!m_signal_manager) {
            event->ignore();
            emit sigCursorChanged(SAMPLE_INDEX_MAX);
            return;
        }

        sample_index_t left  = m_signal_manager->selection().first();
        sample_index_t right = m_signal_manager->selection().last();
        const sample_index_t w = pixels2samples(width());
        QRect rect(this->rect());

        // crop selection to widget borders
        if (left < m_offset) left = m_offset;
        if (right > m_offset + w) right = m_offset + w - 1;

        // transform to pixel coordinates
        int l = qMin(samples2pixels(left  - m_offset), width() - 1);
        int r = qMin(samples2pixels(right - m_offset), l);
        rect.setLeft(l);
        rect.setRight(r);
        event->ignore(rect);
        emit sigCursorChanged(SAMPLE_INDEX_MAX);
    } else if (Kwave::Drag::canDecode(event->mimeData())) {
        // accept if it is decodeable within the
        // current range (if it's outside our own selection)
        event->acceptProposedAction();

        // show a cursor at the possible drop location
        sample_index_t item_pos  = m_offset + pixels2samples(x);
        emit sigCursorChanged(item_pos);
    } else if (Kwave::FileDrag::canDecode(event->mimeData())) {
        // file drag
        event->accept();
        emit sigCursorChanged(SAMPLE_INDEX_MAX);
    } else {
        event->ignore();
        emit sigCursorChanged(SAMPLE_INDEX_MAX);
    }
}

//***************************************************************************
//***************************************************************************
Kwave::SignalView::PositionWidget::PositionWidget(QWidget *parent)
    :QWidget(parent), m_label(nullptr), m_alignment(),
     m_radius(10), m_arrow_length(30), m_last_alignment(Qt::AlignHCenter),
     m_last_size(QSize(0,0)), m_polygon()
{
    hide();

    m_label = new(std::nothrow) QLabel(this);
    Q_ASSERT(m_label);
    if (!m_label) return;

    m_label->setFrameStyle(static_cast<int>(QFrame::Panel) |
                           static_cast<int>(QFrame::Plain));
    m_label->setPalette(QToolTip::palette()); // use same colors as a QToolTip
    m_label->setFocusPolicy(Qt::NoFocus);
    m_label->setMouseTracking(true);
    m_label->setLineWidth(0);

    setPalette(QToolTip::palette()); // use same colors as a QToolTip
    setMouseTracking(false);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

//***************************************************************************
Kwave::SignalView::PositionWidget::~PositionWidget()
{
    delete m_label;
    m_label = nullptr;
}

//***************************************************************************
void Kwave::SignalView::PositionWidget::setText(const QString &text,
                                                Qt::Alignment alignment)
{
    if (!m_label) return;

    m_alignment = alignment;

    m_label->setText(text);
    m_label->setAlignment(m_alignment);
    m_label->resize(m_label->sizeHint());

    switch (m_alignment) {
        case Qt::AlignLeft:
            resize(m_arrow_length + m_radius + m_label->width() + m_radius,
                   m_radius + m_label->height() + m_radius);
            m_label->move(m_arrow_length + m_radius, m_radius);
            break;
        case Qt::AlignRight:
            resize(m_radius + m_label->width() + m_radius + m_arrow_length,
                   m_radius + m_label->height() + m_radius);
            m_label->move(m_radius, m_radius);
            break;
        case Qt::AlignHCenter:
            resize(m_radius + m_label->width() + m_radius,
                   m_arrow_length + m_radius + m_label->height() + m_radius);
            m_label->move(m_radius, m_arrow_length + m_radius);
            break;
        default:
            ;
    }

    updateMask();
}

//***************************************************************************
void Kwave::SignalView::PositionWidget::updateMask()
{
    // bail out if nothing has changed
    if ((size() == m_last_size) && (m_alignment == m_last_alignment))
        return;

    QPainter p;
    QBitmap bmp(size());
    bmp.fill(Qt::color0);
    p.begin(&bmp);

    QBrush brush(Qt::color1);
    p.setBrush(brush);
    p.setPen(Qt::color1);

    const int h = height();
    const int w = width();

    // re-create the polygon, depending on alignment
    switch (m_alignment) {
        case Qt::AlignLeft:
            m_polygon.setPoints(8,
                m_arrow_length, 0,
                w - 1, 0,
                w - 1, h - 1,
                m_arrow_length, h - 1,
                m_arrow_length, (2 * h) / 3,
                0, h / 2,
                m_arrow_length, h / 3,
                m_arrow_length, 0
            );
            break;
        case Qt::AlignRight:
            m_polygon.setPoints(8,
                0, 0,
                w - 1 - m_arrow_length, 0,
                w - 1 - m_arrow_length, h / 3,
                w - 1, h/2,
                w - 1 - m_arrow_length, (2 * h) / 3,
                w - 1 - m_arrow_length, h - 1,
                0, h - 1,
                0, 0
            );
            break;
        case Qt::AlignHCenter:
            break;
        default:
            ;
    }

    p.drawPolygon(m_polygon);
    p.end();

    // activate the new widget mask
    clearMask();
    setMask(bmp);

    // remember size/alignment for detecting changes
    m_last_alignment = m_alignment;
    m_last_size      = size();
}

//***************************************************************************
void Kwave::SignalView::PositionWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(palette().window().color());
    p.drawPolygon(m_polygon);
}

//***************************************************************************
//***************************************************************************

#include "moc_SignalView.cpp"
