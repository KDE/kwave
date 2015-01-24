/***************************************************************************
       OverViewWidget.h  -  horizontal slider with overview over a signal
                             -------------------
    begin                : Tue Oct 21 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _OVER_VIEW_WIDGET_H_
#define _OVER_VIEW_WIDGET_H_

#include <config.h>

#include <QtGui/QBitmap>
#include <QtGui/QColor>
#include <QtCore/QSize>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QWidget>

#include <kdemacros.h>

#include "libkwave/LabelList.h"
#include "libkwave/MetaDataList.h"

#include "libgui/ImageView.h"
#include "libgui/OverViewCache.h"

class QMouseEvent;
class QPainter;
class QResizeEvent;

namespace Kwave
{

    class SignalManager;
    class Track;

    class KDE_EXPORT OverViewWidget : public Kwave::ImageView
    {
	Q_OBJECT
    public:
	/** Constructor */
	OverViewWidget(Kwave::SignalManager &signal, QWidget *parent = 0);

	/** Destructor */
	virtual ~OverViewWidget();

	/** minimum size of the widget, @see QWidget::minimumSize() */
	virtual QSize minimumSize() const;

	/** optimal size for the widget, @see QWidget::sizeHint() */
	virtual QSize sizeHint() const;

    public slots:

	/**
	 * Sets new range parameters of the slider, using a scale that is calculated
	 * out of the slider's maximum position. All parameters are given in the
	 * user's coordinates/units (e.g. samples).
	 * @param offset index of the first visible sample
	 * @param viewport width of the visible area
	 * @param total width of the whole signal
	 */
	void setRange(sample_index_t offset, sample_index_t viewport,
	              sample_index_t total);

	/**
	 * called when the selected time has changed
	 * @param offset index of the first selected sample
	 * @param length number of selected samples
	 */
	void setSelection(sample_index_t offset, sample_index_t length);

	/**
	* should be called when meta data has changed
	* @param meta the list of new meta data
	*/
	void metaDataChanged(Kwave::MetaDataList meta);

	/** should be called to update the current playback posiotion */
	void playbackPositionChanged(sample_index_t pos);

	/** should be called when playback has been stopped */
	void playbackStopped();

    protected:

	/** refreshes the bitmap when resized */
	void resizeEvent(QResizeEvent *);

	/**
	 * On mouse move:
	 * move the current viewport center to the clicked position.
	 */
	void mouseMoveEvent(QMouseEvent *);

	/**
	 * On single-click with the left mouse button:
	 * move the current viewport center to the clicked position.
	 */
	void mousePressEvent(QMouseEvent *);

	/**
	 * On double click with the left mouse button, without shift:
	 * move the current viewport center to the clicked position, like
	 * on a single-click, but also zoom in (by sending "view:zoom_in()").
	 *
	 * When double clicked with the left mouse button with shift:
	 * The same as above, but zoom out instead of in
	 * (by sending "view:zoom_out()").
	 */
	void mouseDoubleClickEvent(QMouseEvent *e);

    protected slots:

	/** refreshes all modified parts of the bitmap */
	void refreshBitmap();

	/**
	 * connected to the m_repaint_timer, called when it has
	 * elapsed and the signal has to be repainted
	 */
	void overviewChanged();

    signals:

	/**
	 * Will be emitted if the slider position has changed. The value
	 * is in user's units (e.g. samples).
	 */
	void valueChanged(sample_index_t new_value);

	/** emitted for zooming in and out via command */
	void sigCommand(const QString &command);

	/** emitted when the background calculation of the image is done */
	void newImage(QImage image);

    protected:

	/**
	 * Converts a pixel offset within the overview's drawing area
	 * into the user's coordinate system.
	 * @param pixels the pixel coordinate [0...width-1]
	 * @return an offset [0..length-1]
	 */
	sample_index_t pixels2offset(int pixels);

	/**
	 * draws a little mark at the top and bottom of a line
	 * in the overview image
	 * @param p a QPainter used for painting, will be modified
	 * @param x offset from the left [pixel]
	 * @param height the height of the image [pixel]
	 * @param color base color for the marker
	 */
	void drawMark(QPainter &p, int x, int height, QColor color);

    private:

	/** does the calculation of the new bitmap in background */
	void calculateBitmap();

    private:

	/** internal worker thread for updating the bitmap in background */
	class WorkerThread: public QThread
	{
	public:
	    /** constructor */
	    WorkerThread(Kwave::OverViewWidget *widget);

	    /** destructor */
	    virtual ~WorkerThread();

	    /** thread function that calls calculateBitmap() */
	    virtual void run();

	private:

	    /** pointer to the calling overview widget */
	    Kwave::OverViewWidget *m_overview;
	};

    private:

	/** index of the first visible sample */
	sample_index_t m_view_offset;

	/** width of the visible area [samples] */
	sample_index_t m_view_width;

	/** length of the whole area [samples] */
	sample_index_t m_signal_length;

	/** start of the selection [samples] */
	sample_index_t m_selection_start;

	/** length of the selection [samples] */
	sample_index_t m_selection_length;

	/** last playback position */
	sample_index_t m_playback_position;

	/** last emitted offset (for avoiding duplicate events) */
	sample_index_t m_last_offset;

	/** cache with overview data */
	Kwave::OverViewCache m_cache;

	/** timer for limiting the number of repaints per second */
	QTimer m_repaint_timer;

	/** list of labels */
	Kwave::LabelList m_labels;

	/** worker thread for updates in background */
	WorkerThread m_worker_thread;

    };
}

#endif // _OVER_VIEW_WIDGET_H_

//***************************************************************************
//***************************************************************************
