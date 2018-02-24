/***************************************************************************
    TrackView.cpp  -  signal views that shows the track in time space
			     -------------------
    begin                : Sat Jan 30 2010
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

#include <QtGlobal>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QTime>
#include <QVBoxLayout>

#include <KIconLoader>

#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"

#include "libgui/LabelItem.h"
#include "libgui/MultiStateWidget.h"
#include "libgui/SelectionBorderItem.h"
#include "libgui/SelectionItem.h"
#include "libgui/TrackView.h"
#include "libgui/ViewItem.h"

/** minimum height of the view in pixel */
#define MINIMUM_HEIGHT 100

//***************************************************************************
Kwave::TrackView::TrackView(QWidget *parent, QWidget *controls,
                            Kwave::SignalManager *signal_manager,
                            Kwave::Track *track)
    :Kwave::SignalView(parent, controls, signal_manager,
                       Kwave::SignalView::AboveTrackTop),
     m_pixmap(*track),
     m_last_width(-1),
     m_last_height(-1),
     m_image(),
     m_img_signal(),
     m_img_selection(),
     m_img_markers(),
     m_img_signal_needs_refresh(true),
     m_img_selection_needs_refresh(true),
     m_img_markers_needs_refresh(true),
     m_mouse_click_position(0),
     m_cursor_pos(SAMPLE_INDEX_MAX)
{
    setMinimumSize(400, MINIMUM_HEIGHT);

    // trigger a repaint request when the signal has been modified
    connect(&m_pixmap, SIGNAL(sigModified()),
            this,      SLOT(refreshSignalLayer()));

    if (controls) {
	// add the channel controls, for "enabled" / "disabled"

	QVBoxLayout *layout = new QVBoxLayout(controls);
	Q_ASSERT(layout);
	if (!layout) return;

        Kwave::MultiStateWidget *msw =
            new(std::nothrow) Kwave::MultiStateWidget(Q_NULLPTR, 0);
	Q_ASSERT(msw);
	if (!msw) {
	    delete layout;
	    return;
	}

	// add a bitmap for off (0 and on (1)
	msw->addPixmap(_("light_off.xpm"));
	msw->addPixmap(_("light_on.xpm"));

	// connect widget <-> track
	connect(
	    msw,   SIGNAL(clicked(int)),
	    track, SLOT(toggleSelection())
	);
	connect(
	    track, SIGNAL(sigSelectionChanged(bool)),
	    msw,   SLOT(switchState(bool))
	);

	msw->setMinimumSize(20, 20);
	msw->switchState(track->selected());
	layout->addWidget(msw);
   }

    // get informed about meta data changes
    connect(signal_manager, SIGNAL(
            sigMetaDataChanged(Kwave::MetaDataList)),
            this,           SLOT(refreshMarkersLayer()),
            Qt::QueuedConnection);

    // get informed about selection changes
    connect(&(signal_manager->selection()),
            SIGNAL(changed(sample_index_t,sample_index_t)),
            this,
            SLOT(refreshSelectionLayer()));

    // update the playback position
    connect(&(signal_manager->playbackController()),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this,
            SLOT(showCursor(sample_index_t)));
    connect(&(signal_manager->playbackController()),
            SIGNAL(sigPlaybackStopped()),
            this,
	    SLOT(showCursor()));

    // update when the track selection changed
    connect(track, SIGNAL(sigSelectionChanged(bool)),
            this,  SLOT(refreshSignalLayer()));
}

//***************************************************************************
Kwave::TrackView::~TrackView()
{
}

//***************************************************************************
void Kwave::TrackView::refresh()
{
//     qDebug("Kwave::TrackView[%d]::refresh()", track());
    m_pixmap.repaint();
    repaint();
}

//***************************************************************************
void Kwave::TrackView::setZoomAndOffset(double zoom, sample_index_t offset)
{
    Q_ASSERT(zoom >= 0.0);
    Kwave::SignalView::setZoomAndOffset(zoom, offset);
    m_pixmap.setZoom(zoom);
    m_pixmap.setOffset(offset);
    if (m_pixmap.isModified()) {
	refreshAllLayers();
    }
}

//***************************************************************************
void Kwave::TrackView::setVerticalZoom(double zoom)
{
    const int    old_height = this->height();
    const double old_zoom   = verticalZoom();

    if (old_height > MINIMUM_HEIGHT * old_zoom) {
	// stretched mode
	zoom *= double(old_height) / (old_zoom * double(MINIMUM_HEIGHT));
    }

    Kwave::SignalView::setVerticalZoom(zoom);
    setMinimumHeight(Kwave::toInt(MINIMUM_HEIGHT * zoom));
    emit contentSizeChanged();
}

//***************************************************************************
QSharedPointer<Kwave::ViewItem> Kwave::TrackView::findItem(const QPoint &pos)
{
    QSharedPointer<Kwave::ViewItem> item =
        QSharedPointer<Kwave::ViewItem>(Q_NULLPTR);
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return item;

    const double offset    = m_offset + pixels2samples(pos.x()); // [samples]
    const double tolerance = m_zoom * selectionTolerance();      // [samples]
    const double fine_pos  = static_cast<double>(m_offset) +
	(static_cast<double>(pos.x()) * m_zoom);

    // we support the following items (with this priority):
    // 1. a label, which can be moved
    // 2. the border of a selection (left or right), which can be moved
    // 3. the body of a selection, which can be dragged

    // find the nearest label
    Kwave::Label nearest_label;
    unsigned int nearest_label_index = 0;
    double       d_label             = tolerance;
    {
	unsigned int index = 0;
	foreach (const Kwave::Label &label,
	    Kwave::LabelList(m_signal_manager->metaData()))
	{
	    double d = qAbs(static_cast<double>(label.pos()) - fine_pos);
	    if (d < qMin(d_label, tolerance)) {
		d_label             = d;
		nearest_label       = label;
		nearest_label_index = index;
	    }
	    index++;
	}
    }

    // get information about the current selection
    double selection_first  = static_cast<double>(
	m_signal_manager->selection().first());
    double selection_last   = static_cast<double>(
	m_signal_manager->selection().last());
    bool selection_is_empty = (m_signal_manager->selection().length() == 0);
    const double d_selection_left   = qAbs(selection_first - fine_pos);
    const double d_selection_right  = qAbs(selection_last  - fine_pos);

    // special case: label is near selection left and cursor is left
    //               of selection -> take the label
    //               (or vice versa at the right border)
    bool prefer_the_label =
	((d_selection_left  < tolerance) && (fine_pos < selection_first)) ||
	((d_selection_right < tolerance) && (fine_pos > selection_last));
    bool selection_is_nearer =
	(d_selection_left <= d_label) || (d_selection_right <= d_label);
    if (selection_is_nearer && !prefer_the_label) {
	// one of the selection borders is nearer
	d_label = d_selection_left + d_selection_right;
    }

    if ( (d_label <= qMin(d_selection_left, d_selection_right)) &&
	  !nearest_label.isNull() ) {
	// found a label
	return QSharedPointer<Kwave::ViewItem>(new(std::nothrow)
	    Kwave::LabelItem(*this, *m_signal_manager,
	                     nearest_label_index, nearest_label));
    }

    if ( (d_selection_left < qMin(tolerance, d_selection_right)) ||
         ((d_selection_left < tolerance) && selection_is_empty) )
    {
	// found selection border (left) or empty selection
	return QSharedPointer<Kwave::ViewItem>(new(std::nothrow)
	    Kwave::SelectionBorderItem(
	        *this, *m_signal_manager,
		m_signal_manager->selection().first()));
    }

    if (d_selection_right < qMin(tolerance, d_selection_left)) {
	// found selection border (right)
	return QSharedPointer<Kwave::ViewItem>(new(std::nothrow)
	    Kwave::SelectionBorderItem(
		*this, *m_signal_manager,
		m_signal_manager->selection().last()));
    }

    if ((offset >= selection_first) && (offset <= selection_last)) {
	// found selection body
	return QSharedPointer<Kwave::ViewItem>(new(std::nothrow)
	    Kwave::SelectionItem(*this, *m_signal_manager));
    }

    // nothing found
    return QSharedPointer<Kwave::ViewItem>(Q_NULLPTR);
}

//***************************************************************************
void Kwave::TrackView::handleContextMenu(const QPoint &pos, QMenu *menu)
{
    KIconLoader icon_loader;

    QMenu *submenu_label = menu->addMenu(i18n("Label"));
    Q_ASSERT(submenu_label);
    if (!submenu_label) return;

    // add label
    QAction *action_label_new = submenu_label->addAction(
	icon_loader.loadIcon(_("list-add"), KIconLoader::Toolbar),
	i18n("New"), this, SLOT(contextMenuLabelNew()));
    Q_ASSERT(action_label_new);
    if (!action_label_new) return;

    // store the menu position
    m_mouse_click_position = m_offset + pixels2samples(pos.x());
}

//***************************************************************************
void Kwave::TrackView::contextMenuLabelNew()
{
    emit sigCommand(_("label:add(%1)").arg(m_mouse_click_position));
}

//***************************************************************************
void Kwave::TrackView::resizeEvent(QResizeEvent *event)
{
    Kwave::SignalView::resizeEvent(event);
    if (!event) return;

    // request a repaint on all size changes, but not on horizontal shrink
    if ((event->size().width()   > m_last_width) ||
	(event->size().height() != m_last_height))
    {
	refreshAllLayers();
    }
}

//***************************************************************************
void Kwave::TrackView::refreshSignalLayer()
{
    m_img_signal_needs_refresh = true;
    emit sigNeedRepaint(this);
}

//***************************************************************************
void Kwave::TrackView::refreshSelectionLayer()
{
    m_img_selection_needs_refresh = true;
    emit sigNeedRepaint(this);
}

//***************************************************************************
void Kwave::TrackView::refreshMarkersLayer()
{
    m_img_markers_needs_refresh = true;
    emit sigNeedRepaint(this);
}

//***************************************************************************
void Kwave::TrackView::refreshAllLayers()
{
    m_img_signal_needs_refresh    = true;
    m_img_selection_needs_refresh = true;
    m_img_markers_needs_refresh   = true;
    emit sigNeedRepaint(this);
}

//***************************************************************************
void Kwave::TrackView::paintEvent(QPaintEvent *)
{
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return;

//     qDebug("TrackView::paintEvent()");
// #define DEBUG_REPAINT_TIMES
#ifdef DEBUG_REPAINT_TIMES
    QTime time;
    time.start();
#endif /* DEBUG_REPAINT_TIMES */

    QPainter p;
    const int width  = QWidget::width();
    const int height = QWidget::height();

//     qDebug("TrackView::paintEvent(): width=%d, height=%d", width, height);

    // --- detect size changes and refresh the whole image ---
    if ((width > m_last_width) || (height != m_last_height)) {
// 	qDebug("TrackView::paintEvent(): window size changed from "
// 	      "%dx%d to %dx%d", m_last_width, m_last_height, width, height);

	// create new images for the layers
	const QImage::Format format = QImage::Format_ARGB32_Premultiplied;
	m_img_signal    = QImage(width, height, format);
	m_img_selection = QImage(width, height, format);
	m_img_markers   = QImage(width, height, format);

	// create a new target image
	m_image         = QImage(width, height, format);

	// mark all images as "need refresh"
	m_img_signal_needs_refresh    = true;
	m_img_selection_needs_refresh = true;
	m_img_markers_needs_refresh   = true;

	// remember the last width
	m_last_width  = width;
	m_last_height = height;
    }

    // --- repaint of the signal layer ---
    if (m_img_signal_needs_refresh) {
// 	qDebug("TrackView::paintEvent(): - redraw of signal layer -");

	p.begin(&m_img_signal);

	// fix the width and height of the track pixmap
	if ((m_pixmap.width() < width) || (m_pixmap.height() != height))
	    m_pixmap.resize(width, height);

	// refresh the pixmap
	if (m_pixmap.isModified())
	    m_pixmap.repaint();

	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.drawPixmap(0, 0, m_pixmap.pixmap());
	p.end();

	m_img_signal_needs_refresh = false;
    }

    // --- repaint of the markers layer ---
    if (m_img_markers_needs_refresh) {
// 	qDebug("TrackView::paintEvent(): - redraw of markers layer -");

	p.begin(&m_img_markers);
	p.fillRect(0, 0, width, height, Qt::black);

	int last_marker = -1;
	const sample_index_t last_visible = lastVisible();
	foreach (const Kwave::Label &label,
	         Kwave::LabelList(m_signal_manager->metaData()))
	{
	    sample_index_t pos = label.pos();
	    if (pos < m_offset)     continue; // outside left
	    if (pos > last_visible) break;    // far outside right, done
	    int x = samples2pixels(pos - m_offset);
	    if (x >= width) break; // outside right, done

	    // position must differ from the last one, otherwise we
	    // would wipe out the last one with XOR mode
	    if (x == last_marker) continue;

	    p.setPen(Qt::cyan);
	    p.setCompositionMode(QPainter::CompositionMode_Exclusion);
	    p.drawLine(x, 0, x, height);

	    last_marker = x;
	}

	p.end();

	m_img_markers_needs_refresh = false;
    }

    // --- repaint of the selection layer ---
    if (m_img_selection_needs_refresh) {
// 	qDebug("TrackView::paintEvent(): - redraw of selection layer -");

	p.begin(&m_img_selection);
	p.fillRect(0, 0, width, height, Qt::black);

	sample_index_t left  = m_signal_manager->selection().first();
	sample_index_t right = m_signal_manager->selection().last();
	const sample_index_t visible = pixels2samples(width);

	if ((right > 0) && (right >= m_offset)) {

	    // shift and clip the selection, relative to m_offset
	    left  = (left > m_offset) ? (left - m_offset) : 0;
	    if (left <= visible) {
		right -= m_offset;
		if (right > visible) right = visible + 1;

		// transform to pixel coordinates
		int l = samples2pixels(left);
		int r = samples2pixels(right);

		// clip to the widget's size
		if (r >= width) r = width - 1;
		if (l > r)      l = r;

		p.setPen(Qt::yellow);
		if (l == r) {
		    p.drawLine(l, 0, l, height);
		} else {
		    p.setBrush(Qt::yellow);
		    p.drawRect(l, 0, r - l + 1, height);
		}
	    }
	}
	p.end();

	m_img_selection_needs_refresh = false;
    }

    // bitBlt all layers together
    p.begin(&m_image);
    p.fillRect(0, 0, width, height, Qt::black);

    // paint the signal layer (copy mode)
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(0, 0, m_img_signal);

    // paint the selection layer (XOR mode)
    p.setCompositionMode(QPainter::CompositionMode_Exclusion);
    p.drawImage(0, 0, m_img_selection);

    // paint the markers/labels layer (XOR mode)
    p.setCompositionMode(QPainter::CompositionMode_Exclusion);
    p.drawImage(0, 0, m_img_markers);

    // --- show the cursor position ---
    do
    {
	if (m_cursor_pos == SAMPLE_INDEX_MAX) break;
	if (m_cursor_pos < m_offset) break;
	const sample_index_t visible = pixels2samples(width);
	if (m_cursor_pos >= m_offset + visible) break;

	int x = samples2pixels(m_cursor_pos - m_offset);
	if (x >= width) break;

	p.setPen(Qt::yellow);
	p.setCompositionMode(QPainter::CompositionMode_Exclusion);
	p.drawLine(x, 0, x, height);
    } while (0);

    p.end();

    // draw the result
    p.begin(this);
    p.drawImage(0, 0, m_image);
    p.end();

#ifdef DEBUG_REPAINT_TIMES
   qDebug("TrackView::paintEvent() -- done, t=%d ms --", time.elapsed());
#endif /* DEBUG_REPAINT_TIMES */
}

//***************************************************************************
void Kwave::TrackView::showCursor(sample_index_t pos)
{
    m_cursor_pos = pos;
    emit sigNeedRepaint(this);
}

//***************************************************************************
//***************************************************************************
