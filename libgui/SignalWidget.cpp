/***************************************************************************
             SignalWidget.cpp  -  Widget for displaying the signal
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

#include <config.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtCore/QEvent>
#include <QtGui/QFrame>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtCore/QTime>
#include <QtGui/QToolTip>

#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

#include "libkwave/ClipBoard.h"
#include "libkwave/LabelList.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"

// #include "libgui/ShortcutWrapper.h"
#include "libgui/SignalView.h"
#include "libgui/SignalWidget.h"
#include "libgui/TrackPixmap.h"
#include "libgui/TrackView.h"

// /** table of keyboard shortcuts 0...9 */
// static const int tbl_keys[10] = {
//     Qt::Key_1,
//     Qt::Key_2,
//     Qt::Key_3,
//     Qt::Key_4,
//     Qt::Key_5,
//     Qt::Key_6,
//     Qt::Key_7,
//     Qt::Key_8,
//     Qt::Key_9,
//     Qt::Key_0
// };

/** vertical zoom factor: minimum value */
#define VERTICAL_ZOOM_MIN 1.0

/** vertical zoom factor: maximum value */
#define VERTICAL_ZOOM_MAX 100.0

/** vertical zoom factor: increment/decrement factor */
#define VERTICAL_ZOOM_STEP_FACTOR 1.5

/** interval for limiting the number of repaints per second [ms] */
#define REPAINT_INTERVAL 125

//***************************************************************************
Kwave::SignalWidget::SignalWidget(QWidget *parent,
                                  Kwave::SignalManager *signal_manager,
                                  QVBoxLayout *upper_dock,
                                  QVBoxLayout *lower_dock)
    :QWidget(parent),
     m_signal_manager(signal_manager),
     m_views(),
     m_layout(this),
     m_upper_dock(upper_dock),
     m_lower_dock(lower_dock),
     m_offset(0),
     m_zoom(0.0),
     m_vertical_zoom(1.0),
     m_repaint_timer()
{
//    qDebug("SignalWidget::SignalWidget()");

    // connect to the signal manager's signals
    Kwave::SignalManager *sig = m_signal_manager;

    connect(sig,  SIGNAL(sigTrackInserted(unsigned int, Kwave::Track *)),
            this, SLOT( slotTrackInserted(unsigned int, Kwave::Track *)));
    connect(sig,  SIGNAL(sigTrackDeleted(unsigned int, Kwave::Track *)),
            this, SLOT( slotTrackDeleted(unsigned int, Kwave::Track *)));

    // use a timer for limiting the repaint rate
    m_repaint_timer.setSingleShot(true);
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this,             SLOT(repaintTimerElapsed()));

//     // -- accelerator keys for 1...9 --
//     for (int i = 0; i < 10; i++) {
// 	Kwave::ShortcutWrapper *shortcut =
// 	    new Kwave::ShortcutWrapper(this, tbl_keys[i], i);
// 	connect(shortcut, SIGNAL(activated(int)),
// 	        this, SLOT(parseKey(int)));
//     }

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout.setColumnStretch(0,   0);
    m_layout.setColumnStretch(1, 100);
    m_layout.setMargin(0);
    m_layout.setVerticalSpacing(3);
    setLayout(&m_layout);
    m_layout.activate();
}

//***************************************************************************
Kwave::SignalWidget::~SignalWidget()
{
}

//***************************************************************************
void Kwave::SignalWidget::setZoomAndOffset(double zoom, sample_index_t offset)
{
    Q_ASSERT(zoom >= 0.0);
    foreach (QPointer<Kwave::SignalView> view, m_views)
	view->setZoomAndOffset(zoom, offset);
}

//***************************************************************************
void Kwave::SignalWidget::forwardCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
void Kwave::SignalWidget::requestRepaint(Kwave::SignalView *view)
{
    // add the view to the repaint queue (if not already there)
    if (!m_repaint_queue.contains(view))
	m_repaint_queue.enqueue(view);

    if (!m_repaint_timer.isActive()) {
	// start the repaint timer
	m_repaint_timer.start(REPAINT_INTERVAL);
    }
    // else: repainting is inhibited -> wait until the
    // repaint timer is elapsed
}

//***************************************************************************
void Kwave::SignalWidget::repaintTimerElapsed()
{
    while (!m_repaint_queue.isEmpty())
    {
	Kwave::SignalView *view = m_repaint_queue.dequeue();
	if (!view) continue;
	view->refresh();
    }
}

//***************************************************************************
void Kwave::SignalWidget::contextMenuEvent(QContextMenuEvent *e)
{
    Q_ASSERT(e);

    bool have_signal = m_signal_manager && !m_signal_manager->isEmpty();
    if (!have_signal)return;
    bool have_selection = (m_signal_manager->selection().length() > 1);
    bool have_labels =
	!(Kwave::LabelList(m_signal_manager->metaData()).isEmpty());

    QMenu *context_menu = new QMenu(this);
    Q_ASSERT(context_menu);
    if (!context_menu) return;

    KIconLoader icon_loader;

    /* menu items common to all cases */

    // undo
    QAction *action;
    action = context_menu->addAction(
	icon_loader.loadIcon(_("edit-undo"), KIconLoader::Toolbar),
	i18n("&Undo"), this, SLOT(contextMenuEditUndo()),
	Qt::CTRL + Qt::Key_Z);
    Q_ASSERT(action);
    if (!action) return;
    if (!m_signal_manager->canUndo())
	action->setEnabled(false);

    // redo
    action = context_menu->addAction(
	icon_loader.loadIcon(_("edit-redo"), KIconLoader::Toolbar),
	i18n("&Redo"), this, SLOT(contextMenuEditRedo()),
	Qt::CTRL + Qt::Key_Y);
    Q_ASSERT(action);
    if (!action) return;
    if (!m_signal_manager->canRedo())
	action->setEnabled(false);
    context_menu->addSeparator();

    // cut/copy/paste
    QAction *action_cut = context_menu->addAction(
	icon_loader.loadIcon(_("edit-cut"), KIconLoader::Toolbar),
	i18n("Cu&t"), this, SLOT(contextMenuEditCut()),
	Qt::CTRL + Qt::Key_X);
    QAction *action_copy = context_menu->addAction(
	icon_loader.loadIcon(_("edit-copy"), KIconLoader::Toolbar),
	i18n("&Copy"), this, SLOT(contextMenuEditCopy()),
	Qt::CTRL + Qt::Key_C);
    QAction *action_paste = context_menu->addAction(
	icon_loader.loadIcon(_("edit-paste"), KIconLoader::Toolbar),
	i18n("&Paste"), this, SLOT(contextMenuEditPaste()),
	Qt::CTRL + Qt::Key_V);
    context_menu->addSeparator();
    if (action_cut)   action_cut->setEnabled(have_selection);
    if (action_copy)  action_copy->setEnabled(have_selection);
    if (action_paste)
	action_paste->setEnabled(!Kwave::ClipBoard::instance().isEmpty());

    int mouse_x = mapFromGlobal(e->globalPos()).x();
    int mouse_y = mapFromGlobal(e->globalPos()).y();
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= width())   mouse_x = width()   - 1;
    if (mouse_y >= height())  mouse_y = height()  - 1;


    QMenu *submenu_select = context_menu->addMenu(i18n("Selection"));
    Q_ASSERT(submenu_select);
    if (!submenu_select) return;

    // Selection / &Save
    QAction *action_select_save = submenu_select->addAction(
	icon_loader.loadIcon(_("document-save"), KIconLoader::Toolbar),
	i18n("&Save..."), this, SLOT(contextMenuSaveSelection()));
    Q_ASSERT(action_select_save);
    if (!action_select_save) return;
    action_select_save->setEnabled(have_selection);

    // Selection / &Expand to labels
    QAction *action_select_expand_to_labels = submenu_select->addAction(
	i18n("&Expand to Labels"), this,
	SLOT(contextMenuSelectionExpandToLabels()), Qt::Key_E);
    Q_ASSERT(action_select_expand_to_labels);
    if (!action_select_expand_to_labels) return;
    action_select_expand_to_labels->setEnabled(have_labels);

    // Selection / to next labels
    QAction *action_select_next_labels = submenu_select->addAction(
	i18n("To Next Labels"), this,
	SLOT(contextMenuSelectionNextLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_N);
    Q_ASSERT(action_select_next_labels);
    if (!action_select_next_labels) return;
    action_select_next_labels->setEnabled(have_labels);

    // Selection / to previous labels
    QAction *action_select_prev_labels = submenu_select->addAction(
	i18n("To Previous Labels"), this,
	SLOT(contextMenuSelectionPrevLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_P);
    Q_ASSERT(action_select_prev_labels);
    if (!action_select_prev_labels) return;
    action_select_prev_labels->setEnabled(have_labels);

    // find out whether there was a click within a signal view
    QSharedPointer<Kwave::ViewItem> item(0);
    foreach (QPointer<Kwave::SignalView> view, m_views) {
	// map the rect of the view to our coordinate system
	const QRect view_rect = QRect(
	    view->mapToParent(view->rect().topLeft()),
	    view->mapToParent(view->rect().bottomRight()));

	// check: mouse click was into that view?
	if (view_rect.contains(mouse_x, mouse_y)) {
	    // map mouse click position to coordinate system of the view
	    QPoint pos = view->mapFromParent(QPoint(mouse_x, mouse_y));

	    // give the view the chance to extend the context menu
	    view->handleContextMenu(pos, context_menu);

	    // try to find a view item at these coordinates
	    item = view->findItem(pos);

	    // if found, give the item the chance to extend the context menu
	    if (!item.isNull()) {
		connect(item.data(), SIGNAL(sigCommand(const QString &)),
		        this, SLOT(forwardCommand(const QString &)));
		item->appendContextMenu(context_menu);
	    }

	    // we process only one view, views cannot overlap!
	    break;
	}
    }

    context_menu->exec(QCursor::pos());
    delete context_menu;
}

//***************************************************************************
void Kwave::SignalWidget::wheelEvent(QWheelEvent *event)
{
    if (!event) return;

    // we currently are only interested in <Alt> + <WheelUp/Down>
    if (event->modifiers() != Qt::AltModifier) {
	event->ignore();
	return;
    }

    if (event->delta() > 0) {
	// zoom in
	setVerticalZoom(m_vertical_zoom  * VERTICAL_ZOOM_STEP_FACTOR);
	event->accept();
    } else if (event->delta() < 0) {
	// zoom out
	setVerticalZoom(m_vertical_zoom  / VERTICAL_ZOOM_STEP_FACTOR);
	event->accept();
    } else {
	// no change
	event->ignore();
    }
}

//***************************************************************************
void Kwave::SignalWidget::setVerticalZoom(double zoom)
{
    if (zoom > VERTICAL_ZOOM_MAX) zoom = VERTICAL_ZOOM_MAX;
    if (zoom < VERTICAL_ZOOM_MIN) zoom = VERTICAL_ZOOM_MIN;
    if (qFuzzyCompare(zoom, m_vertical_zoom)) return; // no change

    // take over the zoom factor
    m_vertical_zoom = zoom;

    // propagate the zoom to all views
    foreach (QPointer<Kwave::SignalView> view, m_views)
	if (view) view->setVerticalZoom(m_vertical_zoom);

    // get back the maximum zoom set by the views
    foreach (QPointer<Kwave::SignalView> view, m_views)
	if (view && view->verticalZoom() > m_vertical_zoom)
	    m_vertical_zoom = view->verticalZoom();
}

//***************************************************************************
int Kwave::SignalWidget::mapToViewPort(const QPoint &pos) const
{
    if (m_views.isEmpty()) return mapFromGlobal(pos).x(); // if empty
    return mapFromGlobal(pos).x() - m_layout.cellRect(0, 1).left();
}

//***************************************************************************
int Kwave::SignalWidget::visibleWidth() const
{
    if (m_views.isEmpty()) return width(); // if empty
    return m_layout.cellRect(0, 1).width();
}

//***************************************************************************
void Kwave::SignalWidget::insertRow(int index, Kwave::SignalView *view,
                                    QWidget *controls)
{
    const int rows = m_layout.rowCount();
    const int cols = m_layout.columnCount();

    // update the layout: move all items from the index on to the next row
    for (int row = rows; row > index; row--) {
	for (int col = 0; col < cols; col++) {
	    QLayoutItem *item = m_layout.itemAtPosition(row - 1, col);
	    if (item) {
		m_layout.removeItem(item);
		m_layout.addItem(item, row, col);
	    }
	}
    }

    // add the widget to the layout
    m_layout.addWidget(view, index, 1);

    if (controls) {
	// add the controls to the layout
	m_layout.addWidget(controls, index, 0);

	// associate the controls to the view, so that when the view
	// gets removed/deleted, the controls get removed as well
	view->addSibling(controls);

	m_layout.activate();
	// NOTE: QLayout::activate() does not really activate the layout
	//       synchronously as expected. Instead it posts an event to
	//       invalidate the layout of it's parent widget.
	//       Unfortunately we need reliable geometry information
	//       already *now*, which forces us to do the event loop
	//       right here.
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents |
	                    QEventLoop::ExcludeSocketNotifiers);
    }
}

//***************************************************************************
void Kwave::SignalWidget::deleteRow(int index)
{
    const int rows = m_layout.rowCount();
    const int cols = m_layout.columnCount();

    if (index >= rows)
	return;

    // update the layout: move all items from this row to the previous
    for (int row = index; row < (rows - 1); row++) {
	for (int col = 0; col < cols; col++) {
	    QLayoutItem *item = m_layout.itemAtPosition(row + 1, col);
	    if (item) {
		m_layout.removeItem(item);
		m_layout.addItem(item, row, col);
	    }
	}
    }
}

//***************************************************************************
void Kwave::SignalWidget::insertView(Kwave::SignalView *view,
                                     QWidget *controls)
{
    Q_ASSERT(m_upper_dock);
    Q_ASSERT(m_lower_dock);
    Q_ASSERT(view);
    if (!m_upper_dock || !m_lower_dock) return;
    if (!view) return;

    // set initial vertical zoom
    view->setVerticalZoom(m_vertical_zoom);

    // find the proper row to insert the track view
    int index = 0;
    int track = (view) ? view->track() : -1;
    const Kwave::SignalView::Location where = view->preferredLocation();
    switch (where) {
	case Kwave::SignalView::UpperDockTop: {
	    // upper dock area, top
	    index = 0;
	    m_upper_dock->insertWidget(0, view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::UpperDockBottom: {
	    // upper dock area, bottom
	    index = m_upper_dock->count();
	    m_upper_dock->addWidget(view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::Top: {
	    // central layout, above all others
	    index = m_upper_dock->count();
	    int row = 0;
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::AboveTrackTop: {
	    // above the corresponding track, start of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() >= track) break; // reached top
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::Bottom) break;
	    }
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::AboveTrackBottom: {
	    // above the corresponding track, end of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() !=
		        Kwave::SignalView::AboveTrackTop) break;
	    }
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::BelowTrackTop: {
	    // below the corresponding track, start of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::BelowTrackTop) break;
	    }
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::BelowTrackBottom: {
	    // below the corresponding track, end of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::Bottom) break;
	    }
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::Bottom: {
	    // below all others
	    int row = m_layout.rowCount();
	    index = m_upper_dock->count() + row;
	    insertRow(row, view, controls);
	    break;
	}
	case Kwave::SignalView::LowerDockTop: {
	    // lower dock area, top
	    index = m_upper_dock->count() + m_layout.rowCount();
	    m_lower_dock->insertWidget(0, view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::LowerDockBottom:
	    // lower dock area, bottom
	    index = m_upper_dock->count() + m_layout.rowCount() +
	            m_lower_dock->count();
	    m_lower_dock->addWidget(view);
	    Q_ASSERT(!controls);
	    break;
    }

    // insert the view into the list of views
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < m_upper_dock->count() + m_layout.rowCount() +
             m_lower_dock->count());
    m_views.insert(index, view);

    // initially set the current view info
    view->setZoomAndOffset(m_zoom, m_offset);

    // connect all signals
    connect(view,       SIGNAL(sigMouseChanged(Kwave::MouseMark::Mode,
                                               sample_index_t, sample_index_t)),
            this,       SIGNAL(sigMouseChanged(Kwave::MouseMark::Mode,
                                               sample_index_t, sample_index_t)));

    connect(view,       SIGNAL(sigCommand(QString)),
	    this,       SIGNAL(sigCommand(QString)),
	    Qt::QueuedConnection);

    connect(view,       SIGNAL(sigNeedRepaint(Kwave::SignalView *)),
	    this,       SLOT(requestRepaint(Kwave::SignalView *)));

    connect(view,       SIGNAL(contentSizeChanged()),
	    this,       SLOT(updateMinimumHeight()));
    connect(view,       SIGNAL(destroyed(QObject*)),
	    this,       SLOT(updateMinimumHeight()));

    updateMinimumHeight();
}

//***************************************************************************
void Kwave::SignalWidget::slotTrackInserted(unsigned int index,
                                            Kwave::Track *track)
{
    Q_ASSERT(track);
    if (!track) return;

    // create a container widget for the track controls
    QWidget *controls = new QWidget(0);
    Q_ASSERT(controls);
    if (!controls) return;

    // create a new view for the track's signal
    Kwave::SignalView *new_view = new Kwave::TrackView(
	this, controls, m_signal_manager, track);
    Q_ASSERT(new_view);
    if (!new_view) {
	delete controls;
	return;
    }

    // loop over all views and adjust the track index of the following ones
    foreach (QPointer<Kwave::SignalView> view, m_views) {
	if (view->track() >= Kwave::toInt(index))
	    view->setTrack(view->track() + 1);
    }

    // assign the view to the new track
    new_view->setTrack(index);

    insertView(new_view, controls);
}

//***************************************************************************
void Kwave::SignalWidget::slotTrackDeleted(unsigned int index,
                                           Kwave::Track *track)
{
    Q_UNUSED(track);

    // loop over all views, delete those that are bound to this track
    // and adjust the index of the following ones
    bool empty = true;
    QMutableListIterator<QPointer<Kwave::SignalView> > it(m_views);
    while (it.hasNext()) {
	Kwave::SignalView *view = it.next();
	if (view->track() == Kwave::toInt(index)) {
	    it.remove();
	    delete view;
	} else if (view->track() > Kwave::toInt(index)) {
	    view->setTrack(view->track() - 1);
	    empty = false;
	} else if (view->track() != -1) {
	    empty = false;
	}
    }

    // find out if there are any empty rows in the grid now
    const int rows = m_layout.rowCount();
    const int cols = m_layout.columnCount();
    for (int row = 0; row < rows; row++) {
	bool row_is_empty = true;
	for (int col = 0; col < cols; col++) {
	    QLayoutItem *item = m_layout.itemAtPosition(row, col);
	    if (item) {
		row_is_empty = false;
		break;
	    }
	}
	if (row_is_empty) deleteRow(row);
    }

    // if there are only views with track() == -1, we are empty,
    // in that case delete the rest (all views)
    if (empty) {
	while (!m_views.isEmpty())
	    delete m_views.takeFirst();
    }
}

//***************************************************************************
void Kwave::SignalWidget::updateMinimumHeight()
{
    int height = 0;

    const int rows = m_layout.rowCount();
    const int cols = m_layout.columnCount();
    for (int row = 0; row < rows; row++) {
	int h = 0;
	for (int col = 0; col < cols; col++) {
	    QLayoutItem *item = m_layout.itemAtPosition(row, col);
	    QWidget *widget = (item) ? item->widget() : 0;
	    if (widget) {
		int min_height = widget->minimumSize().height();
		if (min_height > h) h = min_height;
	    }
	}
	height += h;
    }
    if (rows > 1) height += (rows - 1) * m_layout.verticalSpacing();

    setMinimumHeight(height);
}

//***************************************************************************
// void Kwave::SignalWidget::parseKey(int key)
// {
//     if ((key < 0) || (key >= m_lamps.count()))
// 	return;
//     if (m_lamps.at(key)) m_lamps.at(key)->nextState();
// }

//***************************************************************************
#include "SignalWidget.moc"
//***************************************************************************
//***************************************************************************
