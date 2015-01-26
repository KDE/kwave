/***************************************************************************
    TrackView.h  -  signal views that shows the track in time space
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

#ifndef _TRACK_VIEW_H_
#define _TRACK_VIEW_H_

#include "config.h"

#include <QtGui/QImage>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtGui/QWidget>

#include "kdemacros.h"

#include "libkwave/Sample.h"

#include "libgui/SignalView.h"
#include "libgui/TrackPixmap.h"

class QPaintEvent;
class QResizeEvent;

namespace Kwave
{

    class SignalManager; // forward declaration
    class Track;

    class KDE_EXPORT TrackView: public SignalView
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param parent pointer to the parent widget
	 * @param controls container widget for associated controls
	 * @param signal_manager the signal manager
	 * @param track the track object this view is bound to
	 */
	TrackView(QWidget *parent, QWidget *controls,
	          Kwave::SignalManager *signal_manager,
	          Kwave::Track *track);

	/** Destructor */
	virtual ~TrackView();

	/** refreshes the bitmap that displays the signal */
	virtual void refresh();

	/**
	 * sets new zoom factor and offset
	 * @param zoom the new zoom factor in pixels/sample
	 * @param offset the index of the first visible sample
	 */
	virtual void setZoomAndOffset(double zoom, sample_index_t offset);

	/**
	 * sets new vertical zoom factor
	 * @param zoom vertical zoom factor
	 */
	virtual void setVerticalZoom(double zoom);

	/** called when the widget has been resized */
	virtual void resizeEvent(QResizeEvent *event);

	/** slot for repainting the widget or portions of it */
	virtual void paintEvent(QPaintEvent *);

	/**
	 * Should be overwritten by subclasses that can display the currently
	 * selected range and allow the user to change the selection by mouse.
	 * @return true if mouse selection is handled
	 */
	virtual bool canHandleSelection() const { return true; }

	/**
	 * Tries to find the nearest item that is visible in this view
	 * at a given position
	 *
	 * @param pos position to look at, relative to view [pixels]
	 * @return the nearest ViewObject in range
	 *         or a null pointer if nothing found
	 */
	virtual QSharedPointer<Kwave::ViewItem> findItem(const QPoint &pos);

	/**
	 * Called when the context menu has been activated over this view
	 * @param pos a position in pixel within this widget
	 * @param menu pointer to the context menu
	 */
	virtual void handleContextMenu(const QPoint &pos, QMenu *menu);

    private slots:

	/**
	 * requests a repaint, as soon as the repaint timer elapsed
	 * @param pos current position of the playback pointer
	 */
	void refreshPlaybackPointer(sample_index_t pos = SAMPLE_INDEX_MAX);

	/** requests a refresh of the signal layer */
	void refreshSignalLayer();

	/** requests a refresh of the selection layer */
	void refreshSelectionLayer();

	/** requests a refresh of the markers layer */
	void refreshMarkersLayer();

	/** requests a refresh of all layers */
	void refreshAllLayers();

	/** context menu: "label / new" */
	void contextMenuLabelNew();

    private:

	/** reference to the signal manager */
	QPointer<Kwave::SignalManager> m_signal_manager;

	/** the track pixmap */
	Kwave::TrackPixmap m_pixmap;

	/** last/previous width of the widget, for detecting size changes */
	int m_last_width;

	/** last/previous height of the widget, for detecting size changes */
	int m_last_height;

	/** QImage used for composition */
	QImage m_image;

	/** QImage used for the signal layer */
	QImage m_img_signal;

	/** QImage used for the selection layer */
	QImage m_img_selection;

	/** QImage used for the markers/labels layer */
	QImage m_img_markers;

	/** if true, the signal layer needs to be refreshed */
	bool m_img_signal_needs_refresh;

	/** if true, the selection layer needs to be refreshed */
	bool m_img_selection_needs_refresh;

	/** if true, the markers layer needs to be refreshed */
	bool m_img_markers_needs_refresh;

	/** position of the last mouse click in samples */
	sample_index_t m_mouse_click_position;

	/** last known position of the playback pointer */
	sample_index_t m_playback_pos;
    };
}

#endif /* _TRACK_VIEW_H_ */

//***************************************************************************
//***************************************************************************
