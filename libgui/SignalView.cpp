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

#include <QBitmap>
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

#include <kglobalsettings.h>

#include "libkwave/SignalManager.h"

#include "libgui/MouseMark.h"

#include "SignalView.h"

/**
 * tolerance in pixel for snapping to a label or selection border
 * @todo selection tolerance should depend on some KDE setting,
 *        but which ?
 */
#define SELECTION_TOLERANCE 10

/** number of milliseconds until the position widget disappears */
#define POSITION_WIDGET_TIME 5000

//***************************************************************************
Kwave::SignalView::SignalView(QWidget *parent, QWidget *controls,
                              SignalManager *signal_manager,
                              Location preferred_location,
                              int track)
    :QWidget(parent),
     m_controls(controls),
     m_signal_manager(signal_manager),
     m_preferred_location(preferred_location),
     m_track_index(track),
     m_offset(0),
     m_zoom(1.0),
     m_mouse_mode(Kwave::MouseMark::MouseNormal),
     m_mouse_selection(),
     m_mouse_down_x(0),
    m_position_widget(this),
    m_position_widget_timer(this)
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
}

//***************************************************************************
int Kwave::SignalView::samples2pixels(sample_index_t samples) const
{
    return (m_zoom > 0.0) ? (samples / m_zoom) : 0;
}

//***************************************************************************
sample_index_t Kwave::SignalView::pixels2samples(int pixels) const
{
    return pixels * m_zoom;
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
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return None;

    const sample_index_t p     = pixels2samples(x) + m_offset;
    const sample_index_t tol   = pixels2samples(SELECTION_TOLERANCE);
    const sample_index_t first = m_signal_manager->selection().first();
    const sample_index_t last  = m_signal_manager->selection().last();
    Q_ASSERT(first <= last);

    // get distance to left/right selection border
    sample_index_t d_left  = (p > first) ? (p - first) : (first - p);
    sample_index_t d_right = (p > last)  ? (p - last)  : (last  - p);

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
void Kwave::SignalView::showPosition(const QString &text, sample_index_t pos,
                                     double ms, const QPoint &mouse)
{
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

    unsigned int t, h, m, s, tms;
    t = static_cast<unsigned int>(rint(ms * 10.0));
    tms = t % 10000;
    t /= 10000;
    s = t % 60;
    t /= 60;
    m = t % 60;
    t /= 60;
    h = t;

    QString str;
    QString hms_format = i18nc(
	"time of the position widget, "\
	"%1=hours, %2=minutes, %3=seconds, %4=milliseconds",
	"%02u:%02u:%02u.%04u");
    QString hms;
    hms.sprintf(hms_format.toUtf8().data(), h, m, s, tms);
    QString txt = QString("%1\n%2\n%3").arg(text).arg(pos).arg(hms);

    switch (selectionPosition(mouse.x()) & ~Selection) {
	case LeftBorder:
	    m_position_widget.setText(txt, Qt::AlignRight);
	    x = samples2pixels(pos - m_offset) - m_position_widget.width();
	    if (x < 0) {
		// switch to left aligned mode
		m_position_widget.setText(txt, Qt::AlignLeft);
		x = samples2pixels(pos - m_offset);
	    }
	    break;
	case RightBorder:
	default:
	    m_position_widget.setText(txt, Qt::AlignLeft);
	    x = samples2pixels(pos - m_offset);
	    if (x + m_position_widget.width() > width()) {
		// switch to right aligned mode
		m_position_widget.setText(txt, Qt::AlignRight);
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
    QPoint pos(mouse_x, mouse_y);

    // bail out if the position did not change
    static int last_x = -1;
    static int last_y = -1;
    if ((mouse_x == last_x) && (mouse_y == last_y)) {
	e->ignore();
	return;
    }
    last_x = mouse_x;
    last_y = mouse_y;

    switch (m_mouse_mode) {
	case Kwave::MouseMark::MouseSelect: {
	    // in move mode, a new selection was created or an old one grabbed
	    // this does the changes with every mouse move...
	    sample_index_t x = m_offset + pixels2samples(mouse_x);
	    m_mouse_selection.update(x);
	    m_signal_manager->selectRange(
		m_mouse_selection.left(),
		m_mouse_selection.length()
	    );
	    showPosition(i18n("Selection"), x, samples2ms(x), pos);
	    break;
	}
	default: {
	    sample_index_t first = m_signal_manager->selection().first();
	    sample_index_t last  = m_signal_manager->selection().last();
// 	    Label label = findLabelNearMouse(mouse_x);

	    // find out what is nearer: label or selection border ?
// 	    if (!label.isNull() && (first != last) && isSelectionBorder(mouse_x)) {
// 		const sample_index_t pos = m_offset + pixels2samples(mouse_x);
// 		const sample_index_t d_label = (pos > label.pos()) ?
// 		    (pos - label.pos()) : (label.pos() - pos);
// 		const sample_index_t d_left = (pos > first) ?
// 		    (pos - first) : (first - pos);
// 		const sample_index_t d_right = (pos > last) ?
// 		    (pos - last) : (last - pos);
// 		if ( ((d_label ^ 2) > (d_left ^ 2)) &&
// 		     ((d_label ^ 2) > (d_right ^ 2)) )
// 		{
// 		    // selection borders are nearer
// 		    label = Label();
// 		}
// 	    }

// 	    // yes, this code gives the nifty cursor change....
// 	    if (!label.isNull()) {
// 		setMouseMode(MouseAtSelectionBorder);
// 		int index = signal_manager->labelIndex(label);
// 		QString text = (label.name().length()) ?
// 		    i18n("Label #%1 (%2)", index, label.name()) :
// 		    i18n("Label #%1", index);
// 		showPosition(text, label.pos(), samples2ms(label.pos()), pos);
// 		break;
	    /* } else*/ if ((first != last) && isSelectionBorder(mouse_x)) {
		setMouseMode(Kwave::MouseMark::MouseAtSelectionBorder);
		switch (selectionPosition(mouse_x) & ~Selection) {
		    case LeftBorder:
			showPosition(i18n("Selection, left border"),
 			    first, samples2ms(first), pos);
			break;
		    case RightBorder:
			showPosition(i18n("Selection, right border"),
			    last, samples2ms(last), pos);
			break;
		    default:
			hidePosition();
		}
	    } else if (isInSelection(mouse_x)) {
		setMouseMode(Kwave::MouseMark::MouseInSelection);
		hidePosition();
                int dmin = KGlobalSettings::dndEventDelay();
		if ((e->buttons() & Qt::LeftButton) &&
		    ((mouse_x < m_mouse_down_x - dmin) ||
		     (mouse_x > m_mouse_down_x + dmin)) )
		{
		    qDebug("startDragging();");
		}
	    } else {
		setMouseMode(Kwave::MouseMark::MouseNormal);
		hidePosition();
	    }
	}
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
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
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
bool Kwave::SignalView::PositionWidget::event(QEvent *e)
{
    if (!e) return false;

    // ignore any kind of event that might be of interest
    // for the parent widget (in our case the SignalWidget)
    switch (e->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::Shortcut:
	case QEvent::Wheel:
	case QEvent::Clipboard:
	case QEvent::Speech:
	case QEvent::DragEnter:
	case QEvent::DragMove:
	case QEvent::DragLeave:
	case QEvent::Drop:
	case QEvent::DragResponse:
	case QEvent::TabletMove:
	case QEvent::TabletPress:
	case QEvent::TabletRelease:
	    return false;
	default:
	    ;
    }

    // everything else: let it be handled by QLabel
    return QWidget::event(e);
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
		w-1, 0,
		w-1, h-1,
		m_arrow_length, h-1,
		m_arrow_length, 2*h/3,
		0, h/2,
		m_arrow_length, h/3,
		m_arrow_length, 0
	    );
	    break;
	case Qt::AlignRight:
	    m_polygon.setPoints(8,
		0, 0,
		w-1-m_arrow_length, 0,
		w-1-m_arrow_length, h/3,
		w-1, h/2,
		w-1-m_arrow_length, 2*h/3,
		w-1-m_arrow_length, h-1,
		0, h-1,
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
