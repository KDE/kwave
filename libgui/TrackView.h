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

#include <QImage>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QWidget>

#include "kdemacros.h"

#include "libkwave/Sample.h"
#include "libkwave/Track.h"

#include "libgui/SignalView.h"
#include "libgui/TrackPixmap.h"

class QPaintEvent;
class QResizeEvent;
class SignalManager; // forward declaration

namespace Kwave {

    class KDE_EXPORT TrackView: public SignalView
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param parent pointer to the parent widget
	 * @param controls container widget for associated controls
	 * @param signal_manager the signal manager
	 */
	TrackView(QWidget *parent, QWidget *controls,
	          SignalManager *signal_manager,
	          QPointer<Track> track);

	/** Destructor */
	virtual ~TrackView();

	/**
	 * sets new zoom factor and offset
	 * @param zoom the new zoom factor in pixels/sample
	 * @param offset the index of the first visible sample
	 */
	virtual void setZoomAndOffset(double zoom, sample_index_t offset);

	/** called when the widget has been resized */
	virtual void resizeEvent(QResizeEvent *event);

	/** slot for repainting the widget or portions of it */
	void paintEvent(QPaintEvent *);

    private slots:

	/** requests a repaint, as soon as the repaint timer elapsed */
	void needRepaint();

	/** refreshes the bitmap that displays the signal */
	void refreshBitmap();

	/** requests a refresh of the signal layer */
	void refreshSignalLayer();

	/** requests a refresh of the selection layer */
	void refreshSelectionLayer();

	/** requests a refresh of the markers layer */
	void refreshMarkersLayer();

	/** requests a refresh of all layers */
	void refreshAllLayers();

    private:

	/** reference to the signal manager */
	QPointer<SignalManager> m_signal_manager;

	/** the track pixmap */
	Kwave::TrackPixmap m_pixmap;

	/** timer for limiting the number of repaints per second */
	QTimer m_repaint_timer;

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

    };
}

#endif /* _TRACK_VIEW_H_ */
