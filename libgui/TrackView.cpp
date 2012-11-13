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

#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QTime>
#include <QVBoxLayout>

#include <kiconloader.h>

#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"

#include "LabelItem.h"
#include "MultiStateWidget.h"
#include "TrackView.h"
#include "ViewItem.h"

/** minimum height of the view in pixel */
#define MINIMUM_HEIGHT 100

//***************************************************************************
Kwave::TrackView::TrackView(QWidget *parent, QWidget *controls,
                            Kwave::SignalManager *signal_manager,
                            Kwave::Track *track)
    :Kwave::SignalView(parent, controls, signal_manager,
                       Kwave::SignalView::AboveTrackTop),
     m_signal_manager(signal_manager),
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
     m_mouse_click_position(0)
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

	MultiStateWidget *msw = new MultiStateWidget(0, 0);
	Q_ASSERT(msw);
	if (!msw) {
	    delete layout;
	    return;
	}

	// add a bitmap for off (0 and on (1)
	msw->addPixmap("light_off.xpm");
	msw->addPixmap("light_on.xpm");

	// connect widget <-> track
	connect(
	    msw,   SIGNAL(clicked(int)),
	    track, SLOT(toggleSelection())
	);
	connect(
	    track, SIGNAL(sigSelectionChanged(bool)),
	    msw,   SLOT(switchState(bool))
	);

	msw->setGeometry(5, 5, 20, 20);
	msw->setMinimumSize(20, 20);
	msw->switchState(track->selected());

	layout->addWidget(msw);
	msw->show();
    }

    // get informed about meta data changes
    connect(signal_manager, SIGNAL(
            sigMetaDataChanged(Kwave::MetaDataList)),
            this,           SLOT(refreshMarkersLayer()),
            Qt::QueuedConnection);

    // get informed about selection changes
    connect(&(signal_manager->selection()),
            SIGNAL(changed(sample_index_t, sample_index_t)),
            this,
            SLOT(refreshSelectionLayer()));

    // update the playback position
    connect(&(signal_manager->playbackController()),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this,
            SLOT(refreshPlaybackPointer()));
    connect(&(signal_manager->playbackController()),
            SIGNAL(sigPlaybackStopped()),
            this,
            SLOT(refreshPlaybackPointer()));

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
    const int h = height();
    double current_zoom = verticalZoom();
    if (h > (MINIMUM_HEIGHT * current_zoom)) {
	zoom *= h / (MINIMUM_HEIGHT * current_zoom);
    }
    Kwave::SignalView::setVerticalZoom(zoom);
    setMinimumHeight(MINIMUM_HEIGHT * zoom);
}

//***************************************************************************
QSharedPointer<Kwave::ViewItem> Kwave::TrackView::findItem(const QPoint &pos)
{
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return QSharedPointer<Kwave::ViewItem>(0);

    double offset    = m_offset + pixels2samples(pos.x()); // [samples]
    double tolerance = m_zoom * selectionTolerance();      // [samples]

    // our display can contain labels -> find the nearest label
    double d_min = tolerance;
    Kwave::Label nearest_label;
    int  nearest_index = 0;
    int index = 0;
    foreach (const Kwave::Label &label,
	Kwave::LabelList(m_signal_manager->metaData()))
    {
	double pos = static_cast<double>(label.pos());
	double d = (pos > offset) ? (pos - offset) : (offset - pos);
	if (d < d_min) {
	    d_min = d;
	    nearest_label = label;
	    nearest_index = index;
	}
	index++;
    }

    // found something, get the return value
    if (d_min < tolerance) {
	double ms = samples2ms(nearest_label.pos());
	return QSharedPointer<Kwave::ViewItem>(
	    new Kwave::LabelItem(nearest_index, ms, nearest_label)
	);
    }

    return QSharedPointer<Kwave::ViewItem>(0);
}

//***************************************************************************
void Kwave::TrackView::handleContextMenu(const QPoint &pos, QMenu *menu)
{
    KIconLoader icon_loader;

    QMenu *submenu_label = menu->addMenu(i18n("&Label"));
    Q_ASSERT(submenu_label);
    if (!submenu_label) return;

    // add label
    QAction *action_label_new = submenu_label->addAction(
	icon_loader.loadIcon("list-add", KIconLoader::Toolbar),
	i18n("&New"), this, SLOT(contextMenuLabelNew()));
    Q_ASSERT(action_label_new);
    if (!action_label_new) return;

    // store the menu position
    m_mouse_click_position = m_offset + pixels2samples(pos.x());
}

//***************************************************************************
void Kwave::TrackView::contextMenuLabelNew()
{
    emit sigCommand(QString("add_label(%1)").arg(m_mouse_click_position));
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
	const sample_index_t visible = pixels2samples(width);
	foreach (const Kwave::Label &label,
	         Kwave::LabelList(m_signal_manager->metaData()))
	{
	    sample_index_t pos = label.pos();
	    if (pos < m_offset)           continue; // outside left
	    if (pos > m_offset + visible) break;    // far outside right, done
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

    // --- show the playback position ---
    for (;;)
    {
	if (!m_signal_manager->playbackController().running() &&
	    !m_signal_manager->playbackController().paused())
	    break;
	const sample_index_t pos =
	    m_signal_manager->playbackController().currentPos();
	if (pos < m_offset) break;
	const sample_index_t visible = pixels2samples(width);
	if (pos >= m_offset + visible) break;

	int x = samples2pixels(pos - m_offset);
	if (x >= width) break;

	p.setPen(Qt::yellow);
	p.setCompositionMode(QPainter::CompositionMode_Exclusion);
	p.drawLine(x, 0, x, height);
	break;
    }

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
void Kwave::TrackView::refreshPlaybackPointer()
{
    emit sigNeedRepaint(this);
}

//***************************************************************************
#include "TrackView.moc"
//***************************************************************************
//***************************************************************************
