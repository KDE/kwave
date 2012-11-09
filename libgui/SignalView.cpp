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

#include "config.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <QToolTip>
#include <QUrl>

#include <kglobalsettings.h>

#include "libkwave/CodecManager.h"
#include "libkwave/Drag.h"
#include "libkwave/FileDrag.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/MouseMark.h"

#include "SignalView.h"

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
     m_mouse_mode(Kwave::MouseMark::MouseNormal),
     m_mouse_selection(),
     m_mouse_down_x(0),
     m_position_widget(this),
     m_position_widget_timer(this),
     m_siblings()
{
    // connect the timer of the position widget
    connect(&m_position_widget_timer, SIGNAL(timeout()),
            &m_position_widget, SLOT(hide()));

    setMouseTracking(true);
    setAcceptDrops(true); // enable drag&drop
}

//***************************************************************************
Kwave::SignalView::~SignalView()
{
    if (!m_siblings.isEmpty()) {
	QMutableListIterator<QPointer<QWidget> > it(m_siblings);
	it.toBack();
	while (it.hasPrevious()) {
	    QWidget *widget = it.previous();
	    it.remove();
	    if (widget) delete widget;
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
    if ((zoom == m_zoom) && (offset == m_offset)) return;
    m_zoom   = zoom;
    m_offset = offset;

//     sample_index_t visible = ((width() - 1) * zoom) + 1;
//     sample_index_t last = offset + visible - 1;
//     qDebug("SignalView::setZoomAndOffset(%g, %lu), last visible=%lu",
// 	   zoom,
// 	   static_cast<unsigned long int>(offset),
// 	   static_cast<unsigned long int>(last));

    // the relation to the position widget has become invalid
    hidePosition();
}

//***************************************************************************
void Kwave::SignalView::setVerticalZoom(double zoom)
{
    m_vertical_zoom = zoom;
}

//***************************************************************************
int Kwave::SignalView::samples2pixels(sample_index_t samples) const
{
    return (m_zoom > 0.0) ? (samples / m_zoom) : 0;
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
int Kwave::SignalView::selectionPosition(const int x)
{
    // shortcut: if this view can't handle selection...
    if (!canHandleSelection()) return 0;

    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return None;

    const double p     = (m_zoom * x) + m_offset;
    const double tol   = m_zoom * selectionTolerance();
    const double first = m_signal_manager->selection().first();
    const double last  = m_signal_manager->selection().last();
    Q_ASSERT(first <= last);

    // get distance to left/right selection border
    double d_left  = (p > first) ? (p - first) : (first - p);
    double d_right = (p > last)  ? (p - last)  : (last  - p);

    // the simple cases...
    int pos = None;
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
bool Kwave::SignalView::isSelectionBorder(int x)
{
    SelectionPos pos = static_cast<SignalView::SelectionPos>(
	selectionPosition(x) & ~Selection);

    return ((pos & LeftBorder) || (pos & RightBorder));
}

//***************************************************************************
bool Kwave::SignalView::isInSelection(int x)
{
    return (selectionPosition(x) & Selection) != 0;
}

//***************************************************************************
QSharedPointer<Kwave::ViewItem> Kwave::SignalView::findItem(const QPoint &pos)
{
    Q_UNUSED(pos);
    return QSharedPointer<Kwave::ViewItem>(0);
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

    // x/y == -1/-1 -> reset/hide the position
    if ((x < 0) && (y < 0)) {
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
void Kwave::SignalView::mouseMoveEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    if (!e) return;

    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) {
	e->ignore();
	return;
    }

    // abort if no signal is loaded
    if (!m_signal_manager->length()) {
	e->ignore();
	return;
    }

    // there seems to be a BUG in Qt, sometimes e->pos() produces flicker/wrong
    // coordinates on the start of a fast move!?
    // globalPos() seems not to have this effect
    int mouse_x = mapFromGlobal(e->globalPos()).x();
    int mouse_y = mapFromGlobal(e->globalPos()).y();
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= width())  mouse_x = width()  - 1;
    if (mouse_y >= height()) mouse_y = height() - 1;
    QPoint mouse_pos(mouse_x, mouse_y);

    // bail out if the position did not change
    static int last_x = -1;
    static int last_y = -1;
    if ((mouse_x == last_x) && (mouse_y == last_y)) {
	e->ignore();
	return;
    }
    last_x = mouse_x;
    last_y = mouse_y;

    const sample_index_t pos = m_offset + pixels2samples(mouse_x);
    const double fine_pos = static_cast<double>(m_offset) +
	(static_cast<double>(mouse_x) * m_zoom);

    if (m_mouse_mode == Kwave::MouseMark::MouseSelect) {
	if (canHandleSelection()) {
	    // in move mode, a new selection was created or an old one grabbed
	    // this does the changes with every mouse move...
	    m_mouse_selection.update(pos);
	    m_signal_manager->selectRange(
		m_mouse_selection.left(),
		m_mouse_selection.length()
	    );

	    QString txt = i18nc(
		"tooltip within the selection, "
		"%1=position in samples, "
		"%2=position as a time",
		"Selection\n%1\n%2",
		pos, Kwave::ms2hms(samples2ms(pos)));
	    showPosition(txt, pos, mouse_pos);
	}
    } else {
	sample_index_t selection_first = m_signal_manager->selection().first();
	sample_index_t selection_last  = m_signal_manager->selection().last();
	const bool     selection_is_empty = (selection_first == selection_last);

	// check whether there is some object near this position
	const QPoint mouse_pos = QPoint(mouse_x, mouse_y);
	QSharedPointer<Kwave::ViewItem> item       = findItem(mouse_pos);
	bool                            item_found = !item.isNull();

	// find out what is nearer: object or selection border ?
	if (item_found && (!selection_is_empty) && isSelectionBorder(mouse_x))
	{
	    double d_left  = (fine_pos > selection_first) ?
		(fine_pos - selection_first) : (selection_first - fine_pos);
	    double d_right = (fine_pos > selection_last) ?
		(fine_pos - selection_last) : (selection_last - fine_pos);
	    sample_index_t item_first = item->first();
	    sample_index_t item_last  = item->last();
	    double d_item  = qMin(
		(pos < item_first) ? (item_first - pos) : (pos - item_first),
		(pos < item_last)  ? (item_last  - pos) : (pos - item_last)
	    );

	    // special case: object is at selection left and cursor is left
	    //               of selection -> take the object
	    //               (or vice versa at the right border)
	    bool prefer_the_object =
		((pos == selection_first) && (fine_pos < selection_first)) ||
	        ((pos == selection_last)  && (fine_pos > selection_last));
	    bool selection_is_nearer =
		(d_left <= d_item) || (d_right <= d_item);
	    if (selection_is_nearer && !prefer_the_object) {
		// one of the selection borders is nearer
		item_found = false;
	    }
	}

	if (item_found) {
	    // we have an item to show, activate the position window
	    QString        item_text = item->toolTip(pos);
	    sample_index_t item_pos  = item->first();
	    setMouseMode(Kwave::MouseMark::MouseAtSelectionBorder);
	    showPosition(item_text, item_pos, mouse_pos);
	} else if ((!selection_is_empty) && isSelectionBorder(mouse_x)) {
	    // show position window for start or end of the selection

	    sample_index_t position = 0;
	    QString description;
	    switch (selectionPosition(mouse_x) & ~Selection) {
		case LeftBorder:
		    description = i18n("Selection, left border");
		    position    = selection_first;
		    break;
		case RightBorder:
		    description = i18n("Selection, right border");
		    position    = selection_last;
		    break;
		default:
		    hidePosition();
	    }

	    setMouseMode(Kwave::MouseMark::MouseAtSelectionBorder);

	    if (description.length()) {
		QString hms = Kwave::ms2hms(samples2ms(position));
		QString txt =
		    QString("%1\n%2\n%3").arg(description).arg(position).arg(hms);

		showPosition(txt, position, mouse_pos);
	    }
	} else if (isInSelection(mouse_x)) {
	    setMouseMode(Kwave::MouseMark::MouseInSelection);
	    hidePosition();
	    int dmin = KGlobalSettings::dndEventDelay();
	    if ((e->buttons() & Qt::LeftButton) &&
		((mouse_x < m_mouse_down_x - dmin) ||
		 (mouse_x > m_mouse_down_x + dmin)) )
	    {
		startDragging();
	    }
	} else {
	    setMouseMode(Kwave::MouseMark::MouseNormal);
	    hidePosition();
	}
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

    if (e->button() == Qt::LeftButton) {
	int mx = e->pos().x();
	if (mx < 0) mx = 0;
	if (mx >= width()) mx = width() - 1;
	sample_index_t x   = m_offset + pixels2samples(mx);
	sample_index_t len = m_signal_manager->selection().length();
	switch (e->modifiers()) {
	    case Qt::ShiftModifier: {
		// expand the selection to "here"
		setMouseMode(Kwave::MouseMark::MouseSelect);
		m_mouse_selection.set(
		    m_signal_manager->selection().first(),
		    m_signal_manager->selection().last()
		);
		m_mouse_selection.grep(x);
		m_signal_manager->selectRange(
		    m_mouse_selection.left(),
		    m_mouse_selection.length()
		);
		break;
	    }
	    case Qt::ControlModifier: {
		if (isInSelection(e->pos().x()) && (len > 1)) {
		    // start a drag&drop operation in "copy" mode
// 		    startDragging();
		}
		break;
	    }
	    case 0: {
		if (isSelectionBorder(e->pos().x())) {
		    // modify selection border
		    setMouseMode(Kwave::MouseMark::MouseSelect);
		    m_mouse_selection.set(
			m_signal_manager->selection().first(),
			m_signal_manager->selection().last()
		    );
		    m_mouse_selection.grep(x);
		    m_signal_manager->selectRange(
			m_mouse_selection.left(),
			m_mouse_selection.length()
		    );
		} else if (isInSelection(e->pos().x()) && (len > 1)) {
		    // store the x position for later drag&drop
		    m_mouse_down_x = e->pos().x();
		} else if (canHandleSelection()) {
		    // start a new selection
		    setMouseMode(Kwave::MouseMark::MouseSelect);
		    m_mouse_selection.set(x, x);
		    m_signal_manager->selectRange(x, 0);
		}
		break;
	    }
	}
	e->accept();
    } else {
	e->ignore();
    }
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

    switch (m_mouse_mode) {
	case Kwave::MouseMark::MouseSelect: {
	    if (canHandleSelection()) {
		sample_index_t x = m_offset + pixels2samples(e->pos().x());
		m_mouse_selection.update(x);
		m_signal_manager->selectRange(
		    m_mouse_selection.left(),
		    m_mouse_selection.length()
		);
		setMouseMode(Kwave::MouseMark::MouseNormal);
		hidePosition();
	    }
	    e->accept();
	    break;
	}
	case Kwave::MouseMark::MouseInSelection: {
	    int dmin = KGlobalSettings::dndEventDelay();
	    if ((e->button() & Qt::LeftButton) &&
		    ((e->pos().x() >= m_mouse_down_x - dmin) ||
		     (e->pos().x() <= m_mouse_down_x + dmin)) )
	    {
		// deselect if only clicked without moving
		sample_index_t pos = m_offset + pixels2samples(e->pos().x());
		m_signal_manager->selectRange(pos, 0);
		setMouseMode(Kwave::MouseMark::MouseNormal);
		hidePosition();
	    }
	    e->accept();
	    break;
	}
	default:
	    e->ignore();
    }
}

//***************************************************************************
void Kwave::SignalView::leaveEvent(QEvent* e)
{
    setMouseMode(Kwave::MouseMark::MouseNormal);
    hidePosition();
    QWidget::leaveEvent(e);
}

//***************************************************************************
void Kwave::SignalView::setMouseMode(Kwave::MouseMark::Mode mode)
{
    if (mode == m_mouse_mode) return;

    m_mouse_mode = mode;
    switch (mode) {
	case Kwave::MouseMark::MouseNormal:
	    setCursor(Qt::ArrowCursor);
	    break;
	case Kwave::MouseMark::MouseAtSelectionBorder:
	    setCursor(Qt::SizeHorCursor);
	    break;
	case Kwave::MouseMark::MouseInSelection:
	    setCursor(Qt::ArrowCursor);
	    break;
	case Kwave::MouseMark::MouseSelect:
	    setCursor(Qt::SizeHorCursor);
	    break;
    }

    emit sigMouseChanged(mode);
}

//***************************************************************************
int Kwave::SignalView::selectionTolerance() const
{
    return (2 * QApplication::startDragDistance());
}

//***************************************************************************
void Kwave::SignalView::handleContextMenu(const QPoint &pos, QMenu *menu)
{
    Q_UNUSED(pos);
    Q_UNUSED(menu);
}

//***************************************************************************
void Kwave::SignalView::startDragging()
{
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return;

    const sample_index_t length = m_signal_manager->selection().length();
    if (!length) return;

    KwaveDrag *d = new KwaveDrag(this);
    Q_ASSERT(d);
    if (!d) return;

    const sample_index_t first = m_signal_manager->selection().first();
    const sample_index_t last  = m_signal_manager->selection().last();
    const double         rate  = m_signal_manager->rate();
    const unsigned int   bits  = m_signal_manager->bits();

    MultiTrackReader src(Kwave::SinglePassForward, *m_signal_manager,
	m_signal_manager->selectedTracks(), first, last);

    // create the file info
    Kwave::MetaDataList meta = m_signal_manager->metaData().selectByTracks(
	m_signal_manager->selectedTracks());
    FileInfo info(meta);
    info.setLength(last - first + 1);
    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(src.tracks());
    meta.replace(info);

    if (!d->encode(this, src, meta)) {
	delete d;
	return;
    }

    // start drag&drop, mode is determined automatically
    Kwave::UndoTransactionGuard undo(*m_signal_manager, i18n("Drag and Drop"));
    Qt::DropAction drop = d->exec(Qt::CopyAction | Qt::MoveAction);

    if (drop == Qt::MoveAction) {
	// deleting also affects the selection !
	const sample_index_t f = m_signal_manager->selection().first();
	const sample_index_t l = m_signal_manager->selection().last();
	const sample_index_t len = l - f + 1;

	// special case: when dropping into the same widget, before
	// the previous selection, the previous range has already
	// been moved to the right !
	sample_index_t src = first;
	if ((d->target() == this) && (f < src)) src += len;

	m_signal_manager->deleteRange(src, len,
	    m_signal_manager->selectedTracks());

	// restore the new selection
	m_signal_manager->selectRange((first < f) ? (f - len) : f, len);
    }
}

//***************************************************************************
void Kwave::SignalView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) return;
    if ((event->proposedAction() != Qt::MoveAction) &&
        (event->proposedAction() != Qt::CopyAction))
        return; /* unsupported action */

    if (KwaveFileDrag::canDecode(event->mimeData()))
	event->acceptProposedAction();
}

//***************************************************************************
void Kwave::SignalView::dragLeaveEvent(QDragLeaveEvent *)
{
    setMouseMode(Kwave::MouseMark::MouseNormal);
}

//***************************************************************************
void Kwave::SignalView::dropEvent(QDropEvent *event)
{
    if (!event) return;
    if (!event->mimeData()) return;

    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return;

    if (KwaveDrag::canDecode(event->mimeData())) {
	Kwave::UndoTransactionGuard undo(*m_signal_manager,
	                                 i18n("Drag and Drop"));
	sample_index_t pos = m_offset + pixels2samples(event->pos().x());
	sample_index_t len = 0;

	if ((len = KwaveDrag::decode(this, event->mimeData(),
	    *m_signal_manager, pos)))
	{
	    // set selection to the new area where the drop was done
	    m_signal_manager->selectRange(pos, len);
	    event->acceptProposedAction();
	} else {
	    qWarning("SignalView::dropEvent(%s): failed !", event->format(0));
	    event->ignore();
	}
    } else if (event->mimeData()->hasUrls()) {
	bool first = true;
	foreach (QUrl url, event->mimeData()->urls()) {
	    QString filename = url.toLocalFile();
	    QString mimetype = CodecManager::whatContains(filename);
	    if (CodecManager::canDecode(mimetype)) {
		if (first) {
		    // first dropped URL -> open in this window
		    emit sigCommand("open(" + filename + ")");
		    first = false;
		} else {
		    // all others -> open a new window
		    emit sigCommand("newwindow(" + filename + ")");
		}
	    }
	}
    }

    qDebug("SignalView::dropEvent(): done");
    setMouseMode(Kwave::MouseMark::MouseNormal);
}

//***************************************************************************
void Kwave::SignalView::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event) return;

    const int x = event->pos().x();

    if ((event->source() == this) && isInSelection(x)) {
	// disable drag&drop into the selection itself
	// this would be nonsense

	Q_ASSERT(m_signal_manager);
	if (!m_signal_manager) {
	    event->ignore();
	    return;
	}

	sample_index_t left  = m_signal_manager->selection().first();
	sample_index_t right = m_signal_manager->selection().last();
	const sample_index_t w = pixels2samples(width());
	QRect r(this->rect());

	// crop selection to widget borders
	if (left < m_offset) left = m_offset;
	if (right > m_offset + w) right = m_offset + w - 1;

	// transform to pixel coordinates
	left  = samples2pixels(left  - m_offset);
	right = samples2pixels(right - m_offset);
	if (right >= static_cast<unsigned int>(width()))
	    right = width() - 1;
	if (left > right)
	    left = right;

	r.setLeft(left);
	r.setRight(right);
	event->ignore(r);
    } else if (KwaveDrag::canDecode(event->mimeData())) {
	// accept if it is decodeable within the
	// current range (if it's outside our own selection)
	event->acceptProposedAction();
    } else if (KwaveFileDrag::canDecode(event->mimeData())) {
	// file drag
	event->accept();
    } else event->ignore();
}

//***************************************************************************
//***************************************************************************
Kwave::SignalView::PositionWidget::PositionWidget(QWidget *parent)
    :QWidget(parent), m_label(0), m_alignment(0),
     m_radius(10), m_arrow_length(30), m_last_alignment(Qt::AlignHCenter),
     m_last_size(QSize(0,0)), m_polygon()
{
    hide();

    m_label = new QLabel(this);
    Q_ASSERT(m_label);
    if (!m_label) return;

    m_label->setFrameStyle(QFrame::Panel | QFrame::Plain);
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
    if (m_label) delete m_label;
    m_label = 0;
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

    // remember size/alignment for detecing changes
    m_last_alignment = m_alignment;
    m_last_size      = size();
}

//***************************************************************************
void Kwave::SignalView::PositionWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(palette().background().color());
    p.drawPolygon(m_polygon);
}

//***************************************************************************
#include "SignalView.moc"
//***************************************************************************
//***************************************************************************
