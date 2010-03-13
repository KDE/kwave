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

#include <QMouseEvent>

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
     m_mouse_mode(MouseNormal),
     m_mouse_selection(),
     m_mouse_down_x(0)
{
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
	case MouseSelect: {
	    // in move mode, a new selection was created or an old one grabbed
	    // this does the changes with every mouse move...
	    sample_index_t x = m_offset + pixels2samples(mouse_x);
	    m_mouse_selection.update(x);
	    m_signal_manager->selectRange(
		m_mouse_selection.left(),
		m_mouse_selection.length()
	    );
// 	    showPosition(i18n("Selection"), x, samples2ms(x), pos);
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
// 		qDebug("setMouseMode(MouseAtSelectionBorder);");
		switch (selectionPosition(mouse_x) & ~Selection) {
		    case LeftBorder:
// 			qDebug("Selection, left border");
// 			showPosition(i18n("Selection, left border"),
//  			    first, samples2ms(first), pos);
			break;
		    case RightBorder:
// 			qDebug("Selection, right border");
// 			showPosition(i18n("Selection, right border"),
// 			    last, samples2ms(last), pos);
			break;
		    default:
// 			qDebug("hidePosition");
// 			hidePosition();
			;
		}
	    } else if (isInSelection(mouse_x)) {
// 		qDebug("setMouseMode(MouseInSelection);");
// 		hidePosition();
                int dmin = KGlobalSettings::dndEventDelay();
		if ((e->buttons() & Qt::LeftButton) &&
		    ((mouse_x < m_mouse_down_x - dmin) ||
		     (mouse_x > m_mouse_down_x + dmin)) )
		{
		    qDebug("startDragging();");
		}
	    } else {
// 		qDebug("setMouseMode(MouseNormal);");
// 		hidePosition();
	    }
	}
    }
}

//***************************************************************************
#include "SignalView.moc"
//***************************************************************************
//***************************************************************************
